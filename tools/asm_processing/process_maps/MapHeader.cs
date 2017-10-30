using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using System.Threading.Tasks;

namespace process_maps
{
    enum ConnectionPosition
    {
        North,
        East,
        South,
        West,
    }

    class MapConnection
    {
        public ConnectionPosition Position;
        public string Destination;
        public int LocalPosition;
        public int RemotePosition;
    }

    class MapHeader
    {
        public string Label;
        public string Tileset;
        public int Width, Height;
        public string Blocks;
        public string TextPointers;
        public string Script;
        public readonly List<MapConnection> Connections = new List<MapConnection>();
        public string Objects;

        private static Regex _labelRegex = new Regex(@"^\s*([A-Za-z_][A-Za-z_0-9]*)\s*\:\s*$");
        private static Regex _tilesetRegex = new Regex(@"^\s*db\s+([A-Za-z_][A-Za-z_0-9]*)\s*$");
        private static Regex _sizeRegex = new Regex(@"^\s*db\s+([A-Za-z_][A-Za-z_0-9]*)\s*\,\s*([A-Za-z_][A-Za-z_0-9]*)\s*$");
        private static Regex _pointersRegex = new Regex(@"^\s*dw\s+([A-Za-z_][A-Za-z_0-9]*)\s*\,\s*([A-Za-z_][A-Za-z_0-9]*)\s*\,\s*([A-Za-z_][A-Za-z_0-9]*)\s*$");
        private static Regex _noConnectionsRegex = new Regex(@"^\s*db\s+(?:\$00|0)\s*$");
        private static Regex _connectionRegex = new Regex(@"^\s*(NORTH|EAST|SOUTH|WEST)_MAP_CONNECTION\s+([A-Za-z_][A-Za-z_0-9]*)\s*\,\s*([A-Za-z_][A-Za-z_0-9]*)\s*\,\s*(-?[0-9]+)\s*\,\s*([0-9]+)\s*\,\s*([A-Za-z_][A-Za-z_0-9]*)\s*$");
        private static Regex _connection2Regex = new Regex(@"^\s*(NORTH|EAST|SOUTH|WEST)_MAP_CONNECTION\s+([A-Za-z_][A-Za-z_0-9]*)\s*\,\s*([A-Za-z_][A-Za-z_0-9]*)\s*\,\s*(-?[0-9]+)\s*\,\s*([0-9]+)\s*\,\s*([A-Za-z_][A-Za-z_0-9]*)\s*\,\s*[0-9]+\s*$");
        private static Regex _objectRegex = new Regex(@"^\s*dw\s+([A-Za-z_][A-Za-z_0-9]*)\s*$");

        private static Dictionary<string, ConnectionPosition> _positionMap = new Dictionary<string, ConnectionPosition>
        {
            {"NORTH", ConnectionPosition.North},
            {"EAST", ConnectionPosition.East},
            {"SOUTH", ConnectionPosition.South},
            {"WEST", ConnectionPosition.West},
        };

        public MapHeader(string path, Dictionary<string, int> constants)
        {
            using (var file = new StreamReader(path))
            {
                string line;
                int state = 0;
                while ((line = file.ReadLine()) != null)
                {
                    line = Utility.RemoveComment(line);
                    if (line.Length == 0)
                        continue;
                    switch (state)
                    {
                        case 0:
                            {
                                var match = _labelRegex.Match(line);
                                if (!match.Success)
                                    continue;
                                Label = match.Groups[1].ToString();
                                if (Label.EndsWith("_h"))
                                    Label = Label.Substring(0, Label.Length - 2);
                                state++;
                            }
                            break;
                        case 1:
                            {
                                var match = _tilesetRegex.Match(line);
                                if (!match.Success)
                                    continue;
                                Tileset = Utility.SNAKE_CASE_toCamelCase(match.Groups[1].ToString());
                                state++;
                            }
                            break;
                        case 2:
                            {
                                var match = _sizeRegex.Match(line);
                                if (!match.Success)
                                    continue;
                                var height = match.Groups[1].ToString();
                                var width = match.Groups[2].ToString();
                                Width = constants[width];
                                Height = constants[height];
                                state++;
                            }
                            break;
                        case 3:
                            {
                                var match = _pointersRegex.Match(line);
                                if (!match.Success)
                                    continue;
                                Blocks = match.Groups[1].ToString();
                                TextPointers = match.Groups[2].ToString();
                                Script = match.Groups[3].ToString();
                                state++;
                            }
                            break;
                        case 4:
                            {
                                var match = _noConnectionsRegex.Match(line);
                                if (match.Success)
                                {
                                    state += 2;
                                    continue;
                                }
                                state++;
                            }
                            break;
                        case 5:
                            {
                                var match = _connectionRegex.Match(line);
                                if (!match.Success)
                                    match = _connection2Regex.Match(line);
                                if (match.Success)
                                {
                                    Connections.Add(new MapConnection
                                    {
                                        Position = _positionMap[match.Groups[1].ToString()],
                                        Destination = match.Groups[3].ToString(),
                                        RemotePosition = Convert.ToInt32(match.Groups[4].ToString()),
                                        LocalPosition = Convert.ToInt32(match.Groups[5].ToString()),
                                    });
                                    continue;
                                }
                                match = _objectRegex.Match(line);
                                if (match.Success)
                                {
                                    Objects = match.Groups[1].ToString();
                                    return;
                                }
                                throw new Exception();
                            }
                            break;
                        case 6:
                            {
                                var match = _objectRegex.Match(line);
                                if (match.Success)
                                {
                                    Objects = match.Groups[1].ToString();
                                    return;
                                }
                                throw new Exception();
                            }
                    }
                }
            }
        }
    }
}
