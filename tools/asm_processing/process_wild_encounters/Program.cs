using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Net.Configuration;
using System.Text;
using System.Threading.Tasks;

namespace process_wild_encounters
{
    enum EncounterLocation : byte
    {
        Grass = 0,
        Water = 1,
    }

    class SpeciesEncounter : IEquatable<SpeciesEncounter>
    {
        public byte Species;
        public byte Level;
        public int Repetition = 1;

        public SpeciesEncounter()
        {
        }

        public SpeciesEncounter(SpeciesEncounter other)
        {
            Species = other.Species;
            Level = other.Level;
            Repetition = other.Repetition;
        }

        public string SpeciesName => Program.PokemonById[Species];

        public int ToInteger()
        {
            return (Repetition << 16) | ((int)Species << 8) | Level;
        }

        public bool Equals(SpeciesEncounter other)
        {
            if (other == null)
                return false;
            return Species == other.Species && Level == other.Level && Repetition == other.Repetition;
        }
    }

    class RandomEncounters : IEquatable<RandomEncounters>
    {
        public int EncounterRate;
        public string Version;
        public readonly List<SpeciesEncounter> Encounters;

        public RandomEncounters(byte[] data)
        {
            EncounterRate = data[0];
            if (EncounterRate == 0)
            {
                Encounters = new List<SpeciesEncounter>();
                return;
            }
            var dictionary = new Dictionary<int, SpeciesEncounter>();
            for (int i = 0; i < 10; i++)
            {
                int baseOffset = 1 + i * 2;
                byte level = data[baseOffset + 0];
                byte species = data[baseOffset + 1];
                if (!Program.PokemonById.ContainsKey(species))
                    throw new Exception($"Error. Species {species} is unknown.");
                var encounter = new SpeciesEncounter
                {
                    Level = level,
                    Species = species,
                };
                var hash = encounter.ToInteger();
                while (dictionary.ContainsKey(hash))
                {
                    encounter.Repetition++;
                    hash = encounter.ToInteger();
                }
                dictionary[hash] = encounter;
            }

            Encounters = dictionary.Values.ToList();
        }

        public bool Equals(RandomEncounters other)
        {
            if (other == null)
                return false;
            return EncounterRate == other.EncounterRate && Encounters.Zip(other.Encounters, (x, y) => x.Equals(y)).All(x => x);
        }
    }

    class RandomEncountersForMap
    {
        public string Name;
        public RandomEncounters GrassEncounters;
        public RandomEncounters WaterEncounters;
    }

    class MergedRandomEncounters
    {
        public string Name;
        public int EncounterRateGrass;
        public int EncounterRateWater;
        public readonly List<SpeciesEncounter> RedOnlyGrassEncounters;
        public readonly List<SpeciesEncounter> RedOnlyWaterEncounters;
        public readonly List<SpeciesEncounter> BlueOnlyGrassEncounters;
        public readonly List<SpeciesEncounter> BlueOnlyWaterEncounters;
        public readonly List<SpeciesEncounter> GrassEncounters;
        public readonly List<SpeciesEncounter> WaterEncounters;

        public MergedRandomEncounters(RandomEncountersForMap red, RandomEncountersForMap blue)
        {
            Debug.Assert(red.Name == blue.Name && red.GrassEncounters.EncounterRate == blue.GrassEncounters.EncounterRate && red.WaterEncounters.EncounterRate == blue.WaterEncounters.EncounterRate);
            Name = red.Name;
            EncounterRateGrass = red.GrassEncounters.EncounterRate;
            EncounterRateWater = red.WaterEncounters.EncounterRate;

            var redGrass0 = red.GrassEncounters.Encounters.ToDictionary(x => x.ToInteger());
            var blueGrass0 = blue.GrassEncounters.Encounters.ToDictionary(x => x.ToInteger());
            var redWater0 = red.WaterEncounters.Encounters.ToDictionary(x => x.ToInteger());
            var blueWater0 = blue.WaterEncounters.Encounters.ToDictionary(x => x.ToInteger());

            var mergedGrass = Merge(redGrass0, blueGrass0, red.GrassEncounters.Encounters, blue.GrassEncounters.Encounters);
            var mergedWater = Merge(redWater0, blueWater0, red.WaterEncounters.Encounters, blue.WaterEncounters.Encounters);

            RedOnlyGrassEncounters = mergedGrass[0].Values.OrderBy(x => x.Species).ThenBy(x => x.Level).ToList();
            BlueOnlyGrassEncounters = mergedGrass[1].Values.OrderBy(x => x.Species).ThenBy(x => x.Level).ToList();
            GrassEncounters = mergedGrass[2].Values.OrderBy(x => x.Species).ThenBy(x => x.Level).ToList();

            RedOnlyWaterEncounters = mergedWater[0].Values.OrderBy(x => x.Species).ThenBy(x => x.Level).ToList();
            BlueOnlyWaterEncounters = mergedWater[1].Values.OrderBy(x => x.Species).ThenBy(x => x.Level).ToList();
            WaterEncounters = mergedWater[2].Values.OrderBy(x => x.Species).ThenBy(x => x.Level).ToList();
        }

        private static Dictionary<int, SpeciesEncounter>[] Merge(Dictionary<int, SpeciesEncounter> red0, Dictionary<int, SpeciesEncounter> blue0, List<SpeciesEncounter> red, List<SpeciesEncounter> blue)
        {
            var redOnly = new Dictionary<int, SpeciesEncounter>();
            var blueOnly = new Dictionary<int, SpeciesEncounter>();
            var both = new Dictionary<int, SpeciesEncounter>();
            for (int i = 0; i < red.Count; i++)
            {
                var j = red[i].ToInteger();
                (blue0.ContainsKey(j) ? both : redOnly)[j] = red[i];

                j = blue[i].ToInteger();
                if (!red0.ContainsKey(j))
                    blueOnly[j] = blue[i];
            }

            return new[]
            {
                redOnly,
                blueOnly,
                both,
            };
        }

        public int GetRate(EncounterLocation l)
        {
            switch (l)
            {
                case EncounterLocation.Grass:
                    return EncounterRateGrass;
                case EncounterLocation.Water:
                    return EncounterRateWater;
                default:
                    throw new ArgumentOutOfRangeException(nameof(l), l, null);
            }
        }
    }


    class Program
    {
        #region Data

        public static string[] Symbols =
        {
            "CaveMons", "DungeonMons1", "DungeonMons2", "DungeonMonsB1", "ForestMons", "IslandMons1", "IslandMonsB1", "IslandMonsB2", "IslandMonsB3", "IslandMonsB4", "MansionMons1", "MansionMons2", "MansionMons3", "MansionMonsB1", "MoonMons1", "MoonMonsB1", "MoonMonsB2", "PlateauMons1", "PlateauMons2", "PlateauMons3", "PowerPlantMons", "Route10Mons", "Route11Mons", "Route12Mons", "Route13Mons", "Route14Mons", "Route15Mons", "Route16Mons", "Route17Mons", "Route18Mons", "Route1Mons", "Route21Mons", "Route22Mons", "Route23Mons", "Route24Mons", "Route25Mons", "Route2Mons", "Route3Mons", "Route4Mons", "Route5Mons", "Route6Mons", "Route7Mons", "Route8Mons", "Route9Mons", "TowerMons1", "TowerMons2", "TowerMons3", "TowerMons4", "TowerMons5", "TowerMons6", "TowerMons7", "TunnelMonsB1", "TunnelMonsB2", "WaterMons", "ZoneMons1", "ZoneMons2", "ZoneMons3", "ZoneMonsCenter",
        };

        public static Dictionary<byte, string> PokemonById = new Dictionary<byte, string>
        {
            {0, "None"}, {1, "Rhydon"}, {2, "Kangaskhan"}, {3, "NidoranMale"}, {4, "Clefairy"}, {5, "Spearow"}, {6, "Voltorb"}, {7, "Nidoking"}, {8, "Slowbro"}, {9, "Ivysaur"}, {10, "Exeggutor"}, {11, "Lickitung"}, {12, "Exeggcute"}, {13, "Grimer"}, {14, "Gengar"}, {15, "NidoranFemale"}, {16, "Nidoqueen"}, {17, "Cubone"}, {18, "Rhyhorn"}, {19, "Lapras"}, {20, "Arcanine"}, {21, "Mew"}, {22, "Gyarados"}, {23, "Shellder"}, {24, "Tentacool"}, {25, "Gastly"}, {26, "Scyther"}, {27, "Staryu"}, {28, "Blastoise"}, {29, "Pinsir"}, {30, "Tangela"}, {31, "MissingNo0x1F"}, {32, "MissingNo0x20"}, {33, "Growlithe"}, {34, "Onix"}, {35, "Fearow"}, {36, "Pidgey"}, {37, "Slowpoke"}, {38, "Kadabra"}, {39, "Graveler"}, {40, "Chansey"}, {41, "Machoke"}, {42, "MrMime"}, {43, "Hitmonlee"}, {44, "Hitmonchan"}, {45, "Arbok"}, {46, "Parasect"}, {47, "Psyduck"}, {48, "Drowzee"}, {49, "Golem"}, {50, "MissingNo0x32"}, {51, "Magmar"}, {52, "MissingNo0x34"}, {53, "Electabuzz"}, {54, "Magneton"}, {55, "Koffing"}, {56, "MissingNo0x38"}, {57, "Mankey"}, {58, "Seel"}, {59, "Diglett"}, {60, "Tauros"}, {61, "MissingNo0x3D"}, {62, "MissingNo0x3E"}, {63, "MissingNo0x3F"}, {64, "Farfetchd"}, {65, "Venonat"}, {66, "Dragonite"}, {67, "MissingNo0x43"}, {68, "MissingNo0x44"}, {69, "MissingNo0x45"}, {70, "Doduo"}, {71, "Poliwag"}, {72, "Jynx"}, {73, "Moltres"}, {74, "Articuno"}, {75, "Zapdos"}, {76, "Ditto"}, {77, "Meowth"}, {78, "Krabby"}, {79, "MissingNo0x4F"}, {80, "MissingNo0x50"}, {81, "MissingNo0x51"}, {82, "Vulpix"}, {83, "Ninetales"}, {84, "Pikachu"}, {85, "Raichu"}, {86, "MissingNo0x56"}, {87, "MissingNo0x57"}, {88, "Dratini"}, {89, "Dragonair"}, {90, "Kabuto"}, {91, "Kabutops"}, {92, "Horsea"}, {93, "Seadra"}, {94, "MissingNo0x5E"}, {95, "MissingNo0x5F"}, {96, "Sandshrew"}, {97, "Sandslash"}, {98, "Omanyte"}, {99, "Omastar"}, {100, "Jigglypuff"}, {101, "Wigglytuff"}, {102, "Eevee"}, {103, "Flareon"}, {104, "Jolteon"}, {105, "Vaporeon"}, {106, "Machop"}, {107, "Zubat"}, {108, "Ekans"}, {109, "Paras"}, {110, "Poliwhirl"}, {111, "Poliwrath"}, {112, "Weedle"}, {113, "Kakuna"}, {114, "Beedrill"}, {115, "MissingNo0x73"}, {116, "Dodrio"}, {117, "Primeape"}, {118, "Dugtrio"}, {119, "Venomoth"}, {120, "Dewgong"}, {121, "MissingNo0x79"}, {122, "MissingNo0x7A"}, {123, "Caterpie"}, {124, "Metapod"}, {125, "Butterfree"}, {126, "Machamp"}, {127, "MissingNo0x7F"}, {128, "Golduck"}, {129, "Hypno"}, {130, "Golbat"}, {131, "Mewtwo"}, {132, "Snorlax"}, {133, "Magikarp"}, {134, "MissingNo0x86"}, {135, "MissingNo0x87"}, {136, "Muk"}, {137, "MissingNo0x89"}, {138, "Kingler"}, {139, "Cloyster"}, {140, "MissingNo0x8C"}, {141, "Electrode"}, {142, "Clefable"}, {143, "Weezing"}, {144, "Persian"}, {145, "Marowak"}, {146, "MissingNo0x92"}, {147, "Haunter"}, {148, "Abra"}, {149, "Alakazam"}, {150, "Pidgeotto"}, {151, "Pidgeot"}, {152, "Starmie"}, {153, "Bulbasaur"}, {154, "Venusaur"}, {155, "Tentacruel"}, {156, "MissingNo0x9C"}, {157, "Goldeen"}, {158, "Seaking"}, {159, "MissingNo0x9F"}, {160, "MissingNo0xA0"}, {161, "MissingNo0xA1"}, {162, "MissingNo0xA2"}, {163, "Ponyta"}, {164, "Rapidash"}, {165, "Rattata"}, {166, "Raticate"}, {167, "Nidorino"}, {168, "Nidorina"}, {169, "Geodude"}, {170, "Porygon"}, {171, "Aerodactyl"}, {172, "MissingNo0xAC"}, {173, "Magnemite"}, {174, "MissingNo0xAE"}, {175, "MissingNo0xAF"}, {176, "Charmander"}, {177, "Squirtle"}, {178, "Charmeleon"}, {179, "Wartortle"}, {180, "Charizard"}, {181, "MissingNo0xB5"}, {182, "FossilKabutops"}, {183, "FossilAerodactyl"}, {184, "MonGhost"}, {185, "Oddish"}, {186, "Gloom"}, {187, "Vileplume"}, {188, "Bellsprout"}, {189, "Weepinbell"}, {190, "Victreebel"},
        };

        #endregion

        static Dictionary<string, RandomEncountersForMap> LoadEncounters(string objectFile)
        {
            var parser = new ParseRgbdsObj.ObjParser(objectFile);
            var ret = new Dictionary<string, RandomEncountersForMap>();
            foreach (var symbol in Symbols)
            {
                var data = parser.CopySymbolData(symbol, 21*2);
                var rme = new RandomEncountersForMap
                {
                    Name = symbol
                };
                rme.GrassEncounters = new RandomEncounters(data);
                rme.WaterEncounters = new RandomEncounters(data.Skip(rme.GrassEncounters.EncounterRate == 0 ? 1 : 21).ToArray());
                ret[symbol] = rme;
            }
            return ret;
        }

        static string ToString(EncounterLocation l)
        {
            switch (l)
            {
                case EncounterLocation.Grass:
                    return "g";
                case EncounterLocation.Water:
                    return "w";
                default:
                    throw new ArgumentOutOfRangeException(nameof(l), l, null);
            }
        }

        static void Main(string[] args)
        {
            var redEncounters = LoadEncounters("main_red.o");
            var blueEncounters = LoadEncounters("main_blue.o");

            var mergedEncounters = redEncounters.Keys.Select(x => new MergedRandomEncounters(redEncounters[x], blueEncounters[x])).ToList();

            Console.WriteLine("name,encounter_rate,species,level,location,version");
            foreach (var randomEncounter in mergedEncounters)
            {
                var list = new[]
                {
                    Utility.ToTuple(randomEncounter.GrassEncounters, "rb", EncounterLocation.Grass), Utility.ToTuple(randomEncounter.RedOnlyGrassEncounters, "r", EncounterLocation.Grass), Utility.ToTuple(randomEncounter.BlueOnlyGrassEncounters, "b", EncounterLocation.Grass), Utility.ToTuple(randomEncounter.WaterEncounters, "rb", EncounterLocation.Water), Utility.ToTuple(randomEncounter.RedOnlyWaterEncounters, "r", EncounterLocation.Water), Utility.ToTuple(randomEncounter.BlueOnlyWaterEncounters, "b", EncounterLocation.Water),
                };
                foreach (var i in list)
                    foreach (var encounter in i.Item1)
                        Console.WriteLine($"{randomEncounter.Name},{randomEncounter.GetRate(i.Item3)},{encounter.SpeciesName},{encounter.Level},{ToString(i.Item3)},{i.Item2}");
            }
        }
    }
}
