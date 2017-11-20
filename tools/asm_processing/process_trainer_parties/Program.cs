using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ParseRgbdsObj;

namespace process_trainer_parties
{
    class TrainerPartyPokemon
    {
        public string SpeciesName;
        public int Level;
    }

    class TrainerParty
    {
        public readonly List<TrainerPartyPokemon> Pokemons = new List<TrainerPartyPokemon>();
    }

    class TrainerClass
    {
        public string Name;
        public readonly List<TrainerParty> Parties = new List<TrainerParty>();
    }

    class Program
    {
        #region Data
        private static Dictionary<string, string> Symbols = new Dictionary<string, string>
        {
            {"YoungsterData", "YOUNGSTER"},
            {"BugCatcherData", "BUG_CATCHER"},
            {"LassData", "LASS"},
            {"SailorData", "SAILOR"},
            {"JrTrainerMData", "JR_TRAINER_M"},
            {"JrTrainerFData", "JR_TRAINER_F"},
            {"PokemaniacData", "POKEMANIAC"},
            {"SuperNerdData", "SUPER_NERD"},
            {"HikerData", "HIKER"},
            {"BikerData", "BIKER"},
            {"BurglarData", "BURGLAR"},
            {"EngineerData", "ENGINEER"},
            {"Juggler1Data", "JUGGLER_X"},
            {"FisherData", "FISHER"},
            {"SwimmerData", "SWIMMER"},
            {"CueBallData", "CUE_BALL"},
            {"GamblerData", "GAMBLER"},
            {"BeautyData", "BEAUTY"},
            {"PsychicData", "PSYCHIC_TR"},
            {"RockerData", "ROCKER"},
            {"JugglerData", "JUGGLER"},
            {"TamerData", "TAMER"},
            {"BirdKeeperData", "BIRD_KEEPER"},
            {"BlackbeltData", "BLACKBELT"},
            {"Green1Data", "SONY1"},
            {"ProfOakData", "PROF_OAK"},
            {"ChiefData", "CHIEF"},
            {"ScientistData", "SCIENTIST"},
            {"GiovanniData", "GIOVANNI"},
            {"RocketData", "ROCKET"},
            {"CooltrainerMData", "COOLTRAINER_M"},
            {"CooltrainerFData", "COOLTRAINER_F"},
            {"BrunoData", "BRUNO"},
            {"BrockData", "BROCK"},
            {"MistyData", "MISTY"},
            {"LtSurgeData", "LT_SURGE"},
            {"ErikaData", "ERIKA"},
            {"KogaData", "KOGA"},
            {"BlaineData", "BLAINE"},
            {"SabrinaData", "SABRINA"},
            {"GentlemanData", "GENTLEMAN"},
            {"Green2Data", "SONY2"},
            {"Green3Data", "SONY3"},
            {"LoreleiData", "LORELEI"},
            {"ChannelerData", "CHANNELER"},
            {"AgathaData", "AGATHA"},
            {"LanceData", "LANCE"},
        };

        public static Dictionary<byte, string> PokemonById = new Dictionary<byte, string>
        {
            {0, "None"}, {1, "Rhydon"}, {2, "Kangaskhan"}, {3, "NidoranMale"}, {4, "Clefairy"}, {5, "Spearow"}, {6, "Voltorb"}, {7, "Nidoking"}, {8, "Slowbro"}, {9, "Ivysaur"}, {10, "Exeggutor"}, {11, "Lickitung"}, {12, "Exeggcute"}, {13, "Grimer"}, {14, "Gengar"}, {15, "NidoranFemale"}, {16, "Nidoqueen"}, {17, "Cubone"}, {18, "Rhyhorn"}, {19, "Lapras"}, {20, "Arcanine"}, {21, "Mew"}, {22, "Gyarados"}, {23, "Shellder"}, {24, "Tentacool"}, {25, "Gastly"}, {26, "Scyther"}, {27, "Staryu"}, {28, "Blastoise"}, {29, "Pinsir"}, {30, "Tangela"}, {31, "MissingNo0x1F"}, {32, "MissingNo0x20"}, {33, "Growlithe"}, {34, "Onix"}, {35, "Fearow"}, {36, "Pidgey"}, {37, "Slowpoke"}, {38, "Kadabra"}, {39, "Graveler"}, {40, "Chansey"}, {41, "Machoke"}, {42, "MrMime"}, {43, "Hitmonlee"}, {44, "Hitmonchan"}, {45, "Arbok"}, {46, "Parasect"}, {47, "Psyduck"}, {48, "Drowzee"}, {49, "Golem"}, {50, "MissingNo0x32"}, {51, "Magmar"}, {52, "MissingNo0x34"}, {53, "Electabuzz"}, {54, "Magneton"}, {55, "Koffing"}, {56, "MissingNo0x38"}, {57, "Mankey"}, {58, "Seel"}, {59, "Diglett"}, {60, "Tauros"}, {61, "MissingNo0x3D"}, {62, "MissingNo0x3E"}, {63, "MissingNo0x3F"}, {64, "Farfetchd"}, {65, "Venonat"}, {66, "Dragonite"}, {67, "MissingNo0x43"}, {68, "MissingNo0x44"}, {69, "MissingNo0x45"}, {70, "Doduo"}, {71, "Poliwag"}, {72, "Jynx"}, {73, "Moltres"}, {74, "Articuno"}, {75, "Zapdos"}, {76, "Ditto"}, {77, "Meowth"}, {78, "Krabby"}, {79, "MissingNo0x4F"}, {80, "MissingNo0x50"}, {81, "MissingNo0x51"}, {82, "Vulpix"}, {83, "Ninetales"}, {84, "Pikachu"}, {85, "Raichu"}, {86, "MissingNo0x56"}, {87, "MissingNo0x57"}, {88, "Dratini"}, {89, "Dragonair"}, {90, "Kabuto"}, {91, "Kabutops"}, {92, "Horsea"}, {93, "Seadra"}, {94, "MissingNo0x5E"}, {95, "MissingNo0x5F"}, {96, "Sandshrew"}, {97, "Sandslash"}, {98, "Omanyte"}, {99, "Omastar"}, {100, "Jigglypuff"}, {101, "Wigglytuff"}, {102, "Eevee"}, {103, "Flareon"}, {104, "Jolteon"}, {105, "Vaporeon"}, {106, "Machop"}, {107, "Zubat"}, {108, "Ekans"}, {109, "Paras"}, {110, "Poliwhirl"}, {111, "Poliwrath"}, {112, "Weedle"}, {113, "Kakuna"}, {114, "Beedrill"}, {115, "MissingNo0x73"}, {116, "Dodrio"}, {117, "Primeape"}, {118, "Dugtrio"}, {119, "Venomoth"}, {120, "Dewgong"}, {121, "MissingNo0x79"}, {122, "MissingNo0x7A"}, {123, "Caterpie"}, {124, "Metapod"}, {125, "Butterfree"}, {126, "Machamp"}, {127, "MissingNo0x7F"}, {128, "Golduck"}, {129, "Hypno"}, {130, "Golbat"}, {131, "Mewtwo"}, {132, "Snorlax"}, {133, "Magikarp"}, {134, "MissingNo0x86"}, {135, "MissingNo0x87"}, {136, "Muk"}, {137, "MissingNo0x89"}, {138, "Kingler"}, {139, "Cloyster"}, {140, "MissingNo0x8C"}, {141, "Electrode"}, {142, "Clefable"}, {143, "Weezing"}, {144, "Persian"}, {145, "Marowak"}, {146, "MissingNo0x92"}, {147, "Haunter"}, {148, "Abra"}, {149, "Alakazam"}, {150, "Pidgeotto"}, {151, "Pidgeot"}, {152, "Starmie"}, {153, "Bulbasaur"}, {154, "Venusaur"}, {155, "Tentacruel"}, {156, "MissingNo0x9C"}, {157, "Goldeen"}, {158, "Seaking"}, {159, "MissingNo0x9F"}, {160, "MissingNo0xA0"}, {161, "MissingNo0xA1"}, {162, "MissingNo0xA2"}, {163, "Ponyta"}, {164, "Rapidash"}, {165, "Rattata"}, {166, "Raticate"}, {167, "Nidorino"}, {168, "Nidorina"}, {169, "Geodude"}, {170, "Porygon"}, {171, "Aerodactyl"}, {172, "MissingNo0xAC"}, {173, "Magnemite"}, {174, "MissingNo0xAE"}, {175, "MissingNo0xAF"}, {176, "Charmander"}, {177, "Squirtle"}, {178, "Charmeleon"}, {179, "Wartortle"}, {180, "Charizard"}, {181, "MissingNo0xB5"}, {182, "FossilKabutops"}, {183, "FossilAerodactyl"}, {184, "MonGhost"}, {185, "Oddish"}, {186, "Gloom"}, {187, "Vileplume"}, {188, "Bellsprout"}, {189, "Weepinbell"}, {190, "Victreebel"},
        };

        #endregion

        public static string SNAKE_CASE_toCamelCase(string s)
        {
            var ret = new StringBuilder();
            bool useUpper = true;
            foreach (var c in s)
            {
                if (c == '_')
                {
                    useUpper = true;
                    continue;
                }
                if (!char.IsLetter(c))
                {
                    ret.Append(c);
                    useUpper = true;
                    continue;
                }
                ret.Append(useUpper ? char.ToUpper(c) : char.ToLower(c));
                useUpper = false;
            }
            return ret.ToString();
        }

        static void Main(string[] args)
        {
            var parser = new ObjParser("main_red.o");
            Debug.Assert(Symbols.Keys.All(x => parser.Symbols[x].SectionId == parser.Symbols[Symbols.Keys.First()].SectionId));

            var tuples = Symbols.Keys.Select(x => new Tuple<string, int>(x, (int)parser.Symbols[x].SectionOffset))
                .OrderBy(x => x.Item2).ToList();

            const int lastLength = 12;

            var classes = new List<TrainerClass>();

            for (int i = 0; i < tuples.Count; i++)
            {
                var t = tuples[i];
                int length = i + 1 < tuples.Count ? tuples[i + 1].Item2 - tuples[i].Item2 : lastLength;
                var data = parser.CopySymbolData(t.Item1, length);
                var tc = new TrainerClass {Name = "Opp" + SNAKE_CASE_toCamelCase(Symbols[t.Item1])};
                classes.Add(tc);

                Debug.Assert(data.Length > 1);

                for (int j = 0; j < data.Length; j++)
                {
                    if (data[j] != 0xFF)
                    {
                        int level = data[j++];
                        var party = new TrainerParty();
                        tc.Parties.Add(party);
                        for (; data[j] != 0; j++)
                        {
                            var name = PokemonById[data[j]];
                            party.Pokemons.Add(new TrainerPartyPokemon {Level = level, SpeciesName = name});
                        }
                    }
                    else
                    {
                        j++;
                        var party = new TrainerParty();
                        tc.Parties.Add(party);
                        for (; data[j] != 0; j++)
                        {
                            int level = data[j];
                            j++;
                            var name = PokemonById[data[j]];
                            party.Pokemons.Add(new TrainerPartyPokemon {Level = level, SpeciesName = name});
                        }
                    }
                }
            }

            using (var file = new StreamWriter("trainer_parties.csv"))
            {
                file.WriteLine("trainer_class_name,party_index,species,level");
                foreach (var tc in classes)
                {
                    int index = 1;
                    foreach (var trainerParty in tc.Parties)
                    {
                        foreach (var trainerPartyPokemon in trainerParty.Pokemons)
                        {
                            file.WriteLine($"{tc.Name},{index},{trainerPartyPokemon.SpeciesName},{trainerPartyPokemon.Level}");
                        }
                        index++;
                    }
                }
            }
        }
    }
}
