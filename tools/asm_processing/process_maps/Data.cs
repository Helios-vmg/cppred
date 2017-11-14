using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using ParseRgbdsObj;

namespace process_maps
{
    class Data
    {
        private Dictionary<string, MapHeader> _headers;
        private Dictionary<string, byte[]> _mapData = new Dictionary<string, byte[]>();
        private List<MapHeader> _headerLabels;

        public Data(string basePath)
        {
            var parser = new ObjParser("main_red.o");
            LoadHeaderData(basePath);
            NormalizeMapData(LoadMapData(parser, _headers));
            RenameMapData();
            LoadTextPointers(basePath);
        }

        private void LoadHeaderData(string basePath)
        {
            var mapConstants = MapConstantsParser.Parse(basePath + "/constants/map_constants.asm");
            _headers = Directory.EnumerateFiles(basePath + "data/mapHeaders/").Select(x => new MapHeader(x, mapConstants)).ToDictionary(x => x.Label);
            _headerLabels = HeaderPointerParser.Parse(basePath + "/data/map_header_pointers.asm")
                .Select(x => x.EndsWith("_h") ? x.Substring(0, x.Length - 2) : x)
                .Select(x => _headers[x])
                .ToList();

            foreach (var header in _headers.Values)
                foreach (var connection in header.Connections)
                    connection.Destination = _headerLabels[mapConstants[connection.Destination]].Label;
        }

        static Dictionary<string, byte[]> LoadMapData(ObjParser parser, Dictionary<string, MapHeader> headers)
        {
            var ret = new Dictionary<string, byte[]>();
            foreach (var header in headers.Values)
                ret[header.MapData] = parser.CopySymbolData(header.MapData, header.Width*header.Height);
            return ret;
        }

        private void NormalizeMapData(Dictionary<string, byte[]> mapData)
        {
            var mapDataByHash = new Dictionary<string, string>();
            var mapDataMap = new Dictionary<string, string>();
            foreach (var kv in mapData)
            {
                var hash = kv.Value.GetMd5();
                string value;
                if (!mapDataByHash.TryGetValue(hash, out value))
                {
                    mapDataByHash[hash] = kv.Key;
                    _mapData[kv.Key] = kv.Value;
                    mapDataMap[kv.Key] = kv.Key;
                }
                else
                    mapDataMap[kv.Key] = value;
            }
            foreach (var header in _headers.Values)
                header.MapData = mapDataMap[header.MapData];
        }

        private void RenameMapData()
        {
            foreach (var header in _headers.Values)
                header.MapData = RenameMapData(header.MapData);

            var mapData = new Dictionary<string, byte[]>();
            foreach (var kv in _mapData)
                mapData[RenameMapData(kv.Key)] = kv.Value;
            _mapData = mapData;
        }

        private static string RenameMapData(string s)
        {

            if (s.EndsWith("Blocks"))
                return s.Substring(0, s.Length - 6) + "MapData";
            return s;
        }

        private void LoadTextPointers(string basePath)
        {
            var scripts = Directory.EnumerateFiles(basePath + "scripts/")
                .Select(Utility.LoadFile)
                .Aggregate(string.Empty, (x, y) => x + y);

            var dict = _headers.Values.ToDictionary(x => x.TextPointers);
            var labelRegex = new Regex(@"^\s*([A-Za-z_][A-Za-z_0-9]*)\s*\:\s*$");
            var pointerRegex = new Regex(@"^\s*dw\s*([A-Za-z_][A-Za-z_0-9]*)\s*$");
            List<string> accum = null;
            using (var reader = new StringReader(scripts))
            {
                string line;
                while ((line = reader.ReadLine()) != null)
                {
                    if (accum == null)
                    {
                        var match = labelRegex.Match(line);
                        if (!match.Success)
                            continue;
                        var label = match.Groups[1].ToString();
                        MapHeader header;
                        if (!dict.TryGetValue(label, out header))
                            continue;
                        accum = header.Texts = new List<string>();
                    }
                    else
                    {
                        var match = labelRegex.Match(line);
                        if (match.Success)
                        {
                            accum = null;
                            continue;
                        }
                        match = pointerRegex.Match(line);
                        if (!match.Success)
                            continue;
                        accum.Add(match.Groups[1].ToString());
                    }
                }
            }
        }

        private void LoadObjects(ObjParser parser)
        {
            /*
            foreach (var header in _headers.Values)
            {
                int length = 128;
                do
                {
                    var data = parser.CopySymbolData(header.ObjectsName, length);
                    header.Objects = MapObjects.Parse(data, _headerLabels);
                    length *= 2;
                }
                while (header.Objects == null);
            }
            */
        }

        public void WriteMaps()
        {
            using (var file1 = new StreamWriter("maps.csv"))
            using (var file2 = new StreamWriter("map_connections.csv"))
            using (var file3 = new StreamWriter("map_text.csv"))
            {
                file1.WriteLine("name,tileset,width,height,map_data,script,objects");
                file2.WriteLine("map_name,direction,destination,local_position,remote_position");
                file3.WriteLine("map_name,text");
                foreach (var header in _headers.Values)
                {
                    file1.WriteLine($"{header.Label},{header.Tileset},{header.Width},{header.Height},{header.MapData},{header.Script},{header.ObjectsName}");
                    foreach (var connection in header.Connections)
                        file2.WriteLine($"{header.Label},{connection.Position.ToString().Substring(0, 1)},{connection.Destination},{connection.LocalPosition},{connection.RemotePosition}");
                    foreach (var text in header.Texts)
                        file3.WriteLine($"{header.Label},{text}");
                }
            }
        }

        public void WriteMapData()
        {
            using (var file = new StreamWriter("map_data.csv"))
            {
                file.WriteLine("name,data");
                foreach (var kv in _mapData)
                    file.WriteLine($"{kv.Key},{Convert.ToBase64String(kv.Value)}");
            }
        }
    }
}
