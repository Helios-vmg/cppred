using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace process_warp_tiles
{
    class Program
    {
        static void Main(string[] args)
        {
            string[] arrays = new[]
            {
                "CavernWarpTileIDs",
                "CemeteryWarpTileIDs",
                "ClubWarpTileIDs",
                "FacilityWarpTileIDs",
                "ForestWarpTileIDs",
                "GateWarpTileIDs",
                "GymWarpTileIDs",
                "HouseWarpTileIDs",
                "InteriorWarpTileIDs",
                "LabWarpTileIDs",
                "LobbyWarpTileIDs",
                "MansionWarpTileIDs",
                "OverworldWarpTileIDs",
                "PlateauWarpTileIDs",
                "PokecenterWarpTileIDs",
                "RedsHouse2WarpTileIDs",
                "ShipWarpTileIDs",
                "ShipPortWarpTileIDs",
                "UndergroundWarpTileIDs",
            };

            var parser = new ParseRgbdsObj.ObjParser("main_red.o");
            foreach (var array in arrays)
            {
                var data = parser.CopySymbolData(array, 32);
                Console.Write($"{array},");
                bool first = true;
                foreach (var b in data)
                {
                    if (first)
                        first = false;
                    else
                        Console.Write(" ");
                    if (b == 0xFF)
                        break;
                    Console.Write($"{(int)b:X2}");
                }
                Console.WriteLine();
            }
        }
    }
}
