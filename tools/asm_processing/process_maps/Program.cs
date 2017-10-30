using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Net.Configuration;
using System.Text;
using System.Threading.Tasks;
using ParseRgbdsObj;

namespace process_maps
{
    class Program
    {
        static Dictionary<string, MapHeader> LoadHeaderData(string basePath)
        {
            var headerLabels = HeaderPointerParser.Parse(basePath + "/data/map_header_pointers.asm").Select(x => x.EndsWith("_h") ? x.Substring(0, x.Length - 2) : x).ToList();
            var mapConstants = MapConstantsParser.Parse(basePath + "/constants/map_constants.asm");
            var ret = Directory.EnumerateFiles(basePath + "data/mapHeaders/").Select(x => new MapHeader(x, mapConstants)).ToDictionary(x => x.Label);

            foreach (var header in ret.Values)
            {
                foreach (var connection in header.Connections)
                {
                    var destination = ret[headerLabels[mapConstants[connection.Destination]]];
                    connection.Destination = destination.Label;
                }
            }

            var blockData = LoadBlockData(ret);
            var blockDataByHash = new Dictionary<string, string>();
            foreach (var kv in blockData)
            {
                var hash = kv.Value.GetMd5();
                if (!blockDataByHash.ContainsKey(hash))
                    blockDataByHash[hash] = Convert.ToBase64String(kv.Value);
            }
            var usedByHash = new Dictionary<string, string>();
            foreach (var header in ret.Values)
            {
                var hash = blockData[header.Blocks].GetMd5();
                string value;
                if (!usedByHash.TryGetValue(hash, out value))
                {
                    header.Blocks = blockDataByHash[hash];
                    usedByHash[hash] = "@" + header.Label;
                }
                else
                    header.Blocks = value;
            }

            return ret;
        }

        static Dictionary<string, byte[]> LoadBlockData(Dictionary<string, MapHeader> headers)
        {
            var ret = new Dictionary<string, byte[]>();
            var parser = new ObjParser("main_red.o");
            foreach (var header in headers.Values)
                ret[header.Blocks] = parser.CopySymbolData(header.Blocks, header.Width * header.Height);
            return ret;
        }

        static void Main(string[] args)
        {
            try
            {
                const string basePath = "../../pokered/";
                var headers = LoadHeaderData(basePath);

                using (var file1 = new StreamWriter("maps.csv"))
                using (var file2 = new StreamWriter("map_connections.csv"))
                {
                    file1.WriteLine("name,tileset,width,height,blocks,text_pointers,script,objects");
                    file2.WriteLine("map_name,direction,destination,local_position,remote_position");
                    foreach (var header in headers.Values)
                    {
                        file1.WriteLine($"{header.Label},{header.Tileset},{header.Width},{header.Width},{header.Blocks},{header.TextPointers},{header.Script},{header.Objects}");
                        foreach (var connection in header.Connections)
                        {
                            file2.WriteLine($"{header.Label},{connection.Position.ToString().Substring(0, 1)},{connection.Destination},{connection.LocalPosition},{connection.RemotePosition}");
                        }
                    }
                }
            }
            catch (Exception e)
            {
                Console.Error.WriteLine(e.Message);
            }
        }
    }
}
