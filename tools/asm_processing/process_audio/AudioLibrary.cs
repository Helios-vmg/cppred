using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace process_audio
{
    class AudioLibrary
    {
        #region Paths

        private static string[] DataPaths =
        {
            "audio/sfx/snare1_1.asm",
            "audio/sfx/snare2_1.asm",
            "audio/sfx/snare3_1.asm",
            "audio/sfx/snare4_1.asm",
            "audio/sfx/snare5_1.asm",
            "audio/sfx/triangle1_1.asm",
            "audio/sfx/triangle2_1.asm",
            "audio/sfx/snare6_1.asm",
            "audio/sfx/snare7_1.asm",
            "audio/sfx/snare8_1.asm",
            "audio/sfx/snare9_1.asm",
            "audio/sfx/cymbal1_1.asm",
            "audio/sfx/cymbal2_1.asm",
            "audio/sfx/cymbal3_1.asm",
            "audio/sfx/muted_snare1_1.asm",
            "audio/sfx/triangle3_1.asm",
            "audio/sfx/muted_snare2_1.asm",
            "audio/sfx/muted_snare3_1.asm",
            "audio/sfx/muted_snare4_1.asm",
            "audio/sfx/start_menu_1.asm",
            "audio/sfx/pokeflute.asm",
            "audio/sfx/cut_1.asm",
            "audio/sfx/go_inside_1.asm",
            "audio/sfx/swap_1.asm",
            "audio/sfx/tink_1.asm",
            "audio/sfx/59_1.asm",
            "audio/sfx/purchase_1.asm",
            "audio/sfx/collision_1.asm",
            "audio/sfx/go_outside_1.asm",
            "audio/sfx/press_ab_1.asm",
            "audio/sfx/save_1.asm",
            "audio/sfx/heal_hp_1.asm",
            "audio/sfx/poisoned_1.asm",
            "audio/sfx/heal_ailment_1.asm",
            "audio/sfx/trade_machine_1.asm",
            "audio/sfx/turn_on_pc_1.asm",
            "audio/sfx/turn_off_pc_1.asm",
            "audio/sfx/enter_pc_1.asm",
            "audio/sfx/shrink_1.asm",
            "audio/sfx/switch_1.asm",
            "audio/sfx/healing_machine_1.asm",
            "audio/sfx/teleport_exit1_1.asm",
            "audio/sfx/teleport_enter1_1.asm",
            "audio/sfx/teleport_exit2_1.asm",
            "audio/sfx/ledge_1.asm",
            "audio/sfx/teleport_enter2_1.asm",
            "audio/sfx/fly_1.asm",
            "audio/sfx/denied_1.asm",
            "audio/sfx/arrow_tiles_1.asm",
            "audio/sfx/push_boulder_1.asm",
            "audio/sfx/ss_anne_horn_1.asm",
            "audio/sfx/withdraw_deposit_1.asm",
            "audio/sfx/safari_zone_pa.asm",
            "audio/sfx/unused_1.asm",
            "audio/sfx/cry09_1.asm",
            "audio/sfx/cry23_1.asm",
            "audio/sfx/cry24_1.asm",
            "audio/sfx/cry11_1.asm",
            "audio/sfx/cry25_1.asm",
            "audio/sfx/cry03_1.asm",
            "audio/sfx/cry0f_1.asm",
            "audio/sfx/cry10_1.asm",
            "audio/sfx/cry00_1.asm",
            "audio/sfx/cry0e_1.asm",
            "audio/sfx/cry06_1.asm",
            "audio/sfx/cry07_1.asm",
            "audio/sfx/cry05_1.asm",
            "audio/sfx/cry0b_1.asm",
            "audio/sfx/cry0c_1.asm",
            "audio/sfx/cry02_1.asm",
            "audio/sfx/cry0d_1.asm",
            "audio/sfx/cry01_1.asm",
            "audio/sfx/cry0a_1.asm",
            "audio/sfx/cry08_1.asm",
            "audio/sfx/cry04_1.asm",
            "audio/sfx/cry19_1.asm",
            "audio/sfx/cry16_1.asm",
            "audio/sfx/cry1b_1.asm",
            "audio/sfx/cry12_1.asm",
            "audio/sfx/cry13_1.asm",
            "audio/sfx/cry14_1.asm",
            "audio/sfx/cry1e_1.asm",
            "audio/sfx/cry15_1.asm",
            "audio/sfx/cry17_1.asm",
            "audio/sfx/cry1c_1.asm",
            "audio/sfx/cry1a_1.asm",
            "audio/sfx/cry1d_1.asm",
            "audio/sfx/cry18_1.asm",
            "audio/sfx/cry1f_1.asm",
            "audio/sfx/cry20_1.asm",
            "audio/sfx/cry21_1.asm",
            "audio/sfx/cry22_1.asm",
            "audio/sfx/snare1_2.asm",
            "audio/sfx/snare2_2.asm",
            "audio/sfx/snare3_2.asm",
            "audio/sfx/snare4_2.asm",
            "audio/sfx/snare5_2.asm",
            "audio/sfx/triangle1_2.asm",
            "audio/sfx/triangle2_2.asm",
            "audio/sfx/snare6_2.asm",
            "audio/sfx/snare7_2.asm",
            "audio/sfx/snare8_2.asm",
            "audio/sfx/snare9_2.asm",
            "audio/sfx/cymbal1_2.asm",
            "audio/sfx/cymbal2_2.asm",
            "audio/sfx/cymbal3_2.asm",
            "audio/sfx/muted_snare1_2.asm",
            "audio/sfx/triangle3_2.asm",
            "audio/sfx/muted_snare2_2.asm",
            "audio/sfx/muted_snare3_2.asm",
            "audio/sfx/muted_snare4_2.asm",
            "audio/sfx/press_ab_2.asm",
            "audio/sfx/start_menu_2.asm",
            "audio/sfx/tink_2.asm",
            "audio/sfx/heal_hp_2.asm",
            "audio/sfx/heal_ailment_2.asm",
            "audio/sfx/silph_scope.asm",
            "audio/sfx/ball_toss.asm",
            "audio/sfx/ball_poof.asm",
            "audio/sfx/faint_thud.asm",
            "audio/sfx/run.asm",
            "audio/sfx/dex_page_added.asm",
            "audio/sfx/pokeflute_ch6.asm",
            "audio/sfx/peck.asm",
            "audio/sfx/faint_fall.asm",
            "audio/sfx/battle_09.asm",
            "audio/sfx/pound.asm",
            "audio/sfx/battle_0b.asm",
            "audio/sfx/battle_0c.asm",
            "audio/sfx/battle_0d.asm",
            "audio/sfx/battle_0e.asm",
            "audio/sfx/battle_0f.asm",
            "audio/sfx/damage.asm",
            "audio/sfx/not_very_effective.asm",
            "audio/sfx/battle_12.asm",
            "audio/sfx/battle_13.asm",
            "audio/sfx/battle_14.asm",
            "audio/sfx/vine_whip.asm",
            "audio/sfx/battle_16.asm",
            "audio/sfx/battle_17.asm",
            "audio/sfx/battle_18.asm",
            "audio/sfx/battle_19.asm",
            "audio/sfx/super_effective.asm",
            "audio/sfx/battle_1b.asm",
            "audio/sfx/battle_1c.asm",
            "audio/sfx/doubleslap.asm",
            "audio/sfx/battle_1e.asm",
            "audio/sfx/horn_drill.asm",
            "audio/sfx/battle_20.asm",
            "audio/sfx/battle_21.asm",
            "audio/sfx/battle_22.asm",
            "audio/sfx/battle_23.asm",
            "audio/sfx/battle_24.asm",
            "audio/sfx/battle_25.asm",
            "audio/sfx/battle_26.asm",
            "audio/sfx/battle_27.asm",
            "audio/sfx/battle_28.asm",
            "audio/sfx/battle_29.asm",
            "audio/sfx/battle_2a.asm",
            "audio/sfx/battle_2b.asm",
            "audio/sfx/battle_2c.asm",
            "audio/sfx/psybeam.asm",
            "audio/sfx/battle_2e.asm",
            "audio/sfx/battle_2f.asm",
            "audio/sfx/psychic_m.asm",
            "audio/sfx/battle_31.asm",
            "audio/sfx/battle_32.asm",
            "audio/sfx/battle_33.asm",
            "audio/sfx/battle_34.asm",
            "audio/sfx/battle_35.asm",
            "audio/sfx/battle_36.asm",
            "audio/sfx/unused_2.asm",
            "audio/sfx/cry09_2.asm",
            "audio/sfx/cry23_2.asm",
            "audio/sfx/cry24_2.asm",
            "audio/sfx/cry11_2.asm",
            "audio/sfx/cry25_2.asm",
            "audio/sfx/cry03_2.asm",
            "audio/sfx/cry0f_2.asm",
            "audio/sfx/cry10_2.asm",
            "audio/sfx/cry00_2.asm",
            "audio/sfx/cry0e_2.asm",
            "audio/sfx/cry06_2.asm",
            "audio/sfx/cry07_2.asm",
            "audio/sfx/cry05_2.asm",
            "audio/sfx/cry0b_2.asm",
            "audio/sfx/cry0c_2.asm",
            "audio/sfx/cry02_2.asm",
            "audio/sfx/cry0d_2.asm",
            "audio/sfx/cry01_2.asm",
            "audio/sfx/cry0a_2.asm",
            "audio/sfx/cry08_2.asm",
            "audio/sfx/cry04_2.asm",
            "audio/sfx/cry19_2.asm",
            "audio/sfx/cry16_2.asm",
            "audio/sfx/cry1b_2.asm",
            "audio/sfx/cry12_2.asm",
            "audio/sfx/cry13_2.asm",
            "audio/sfx/cry14_2.asm",
            "audio/sfx/cry1e_2.asm",
            "audio/sfx/cry15_2.asm",
            "audio/sfx/cry17_2.asm",
            "audio/sfx/cry1c_2.asm",
            "audio/sfx/cry1a_2.asm",
            "audio/sfx/cry1d_2.asm",
            "audio/sfx/cry18_2.asm",
            "audio/sfx/cry1f_2.asm",
            "audio/sfx/cry20_2.asm",
            "audio/sfx/cry21_2.asm",
            "audio/sfx/cry22_2.asm",
            "audio/sfx/snare1_3.asm",
            "audio/sfx/snare2_3.asm",
            "audio/sfx/snare3_3.asm",
            "audio/sfx/snare4_3.asm",
            "audio/sfx/snare5_3.asm",
            "audio/sfx/triangle1_3.asm",
            "audio/sfx/triangle2_3.asm",
            "audio/sfx/snare6_3.asm",
            "audio/sfx/snare7_3.asm",
            "audio/sfx/snare8_3.asm",
            "audio/sfx/snare9_3.asm",
            "audio/sfx/cymbal1_3.asm",
            "audio/sfx/cymbal2_3.asm",
            "audio/sfx/cymbal3_3.asm",
            "audio/sfx/muted_snare1_3.asm",
            "audio/sfx/triangle3_3.asm",
            "audio/sfx/muted_snare2_3.asm",
            "audio/sfx/muted_snare3_3.asm",
            "audio/sfx/muted_snare4_3.asm",
            "audio/sfx/start_menu_3.asm",
            "audio/sfx/cut_3.asm",
            "audio/sfx/go_inside_3.asm",
            "audio/sfx/swap_3.asm",
            "audio/sfx/tink_3.asm",
            "audio/sfx/59_3.asm",
            "audio/sfx/purchase_3.asm",
            "audio/sfx/collision_3.asm",
            "audio/sfx/go_outside_3.asm",
            "audio/sfx/press_ab_3.asm",
            "audio/sfx/save_3.asm",
            "audio/sfx/heal_hp_3.asm",
            "audio/sfx/poisoned_3.asm",
            "audio/sfx/heal_ailment_3.asm",
            "audio/sfx/trade_machine_3.asm",
            "audio/sfx/turn_on_pc_3.asm",
            "audio/sfx/turn_off_pc_3.asm",
            "audio/sfx/enter_pc_3.asm",
            "audio/sfx/shrink_3.asm",
            "audio/sfx/switch_3.asm",
            "audio/sfx/healing_machine_3.asm",
            "audio/sfx/teleport_exit1_3.asm",
            "audio/sfx/teleport_enter1_3.asm",
            "audio/sfx/teleport_exit2_3.asm",
            "audio/sfx/ledge_3.asm",
            "audio/sfx/teleport_enter2_3.asm",
            "audio/sfx/fly_3.asm",
            "audio/sfx/denied_3.asm",
            "audio/sfx/arrow_tiles_3.asm",
            "audio/sfx/push_boulder_3.asm",
            "audio/sfx/ss_anne_horn_3.asm",
            "audio/sfx/withdraw_deposit_3.asm",
            "audio/sfx/intro_lunge.asm",
            "audio/sfx/intro_hip.asm",
            "audio/sfx/intro_hop.asm",
            "audio/sfx/intro_raise.asm",
            "audio/sfx/intro_crash.asm",
            "audio/sfx/intro_whoosh.asm",
            "audio/sfx/slots_stop_wheel.asm",
            "audio/sfx/slots_reward.asm",
            "audio/sfx/slots_new_spin.asm",
            "audio/sfx/shooting_star.asm",
            "audio/sfx/unused_3.asm",
            "audio/sfx/cry09_3.asm",
            "audio/sfx/cry23_3.asm",
            "audio/sfx/cry24_3.asm",
            "audio/sfx/cry11_3.asm",
            "audio/sfx/cry25_3.asm",
            "audio/sfx/cry03_3.asm",
            "audio/sfx/cry0f_3.asm",
            "audio/sfx/cry10_3.asm",
            "audio/sfx/cry00_3.asm",
            "audio/sfx/cry0e_3.asm",
            "audio/sfx/cry06_3.asm",
            "audio/sfx/cry07_3.asm",
            "audio/sfx/cry05_3.asm",
            "audio/sfx/cry0b_3.asm",
            "audio/sfx/cry0c_3.asm",
            "audio/sfx/cry02_3.asm",
            "audio/sfx/cry0d_3.asm",
            "audio/sfx/cry01_3.asm",
            "audio/sfx/cry0a_3.asm",
            "audio/sfx/cry08_3.asm",
            "audio/sfx/cry04_3.asm",
            "audio/sfx/cry19_3.asm",
            "audio/sfx/cry16_3.asm",
            "audio/sfx/cry1b_3.asm",
            "audio/sfx/cry12_3.asm",
            "audio/sfx/cry13_3.asm",
            "audio/sfx/cry14_3.asm",
            "audio/sfx/cry1e_3.asm",
            "audio/sfx/cry15_3.asm",
            "audio/sfx/cry17_3.asm",
            "audio/sfx/cry1c_3.asm",
            "audio/sfx/cry1a_3.asm",
            "audio/sfx/cry1d_3.asm",
            "audio/sfx/cry18_3.asm",
            "audio/sfx/cry1f_3.asm",
            "audio/sfx/cry20_3.asm",
            "audio/sfx/cry21_3.asm",
            "audio/sfx/cry22_3.asm",
            "audio/music/pkmnhealed.asm",
            "audio/music/routes1.asm",
            "audio/music/routes2.asm",
            "audio/music/routes3.asm",
            "audio/music/routes4.asm",
            "audio/music/indigoplateau.asm",
            "audio/music/pallettown.asm",
            "audio/music/unusedsong.asm",
            "audio/music/cities1.asm",
            "audio/sfx/get_item1_1.asm",
            "audio/music/museumguy.asm",
            "audio/music/meetprofoak.asm",
            "audio/music/meetrival.asm",
            "audio/sfx/pokedex_rating_1.asm",
            "audio/sfx/get_item2_1.asm",
            "audio/sfx/get_key_item_1.asm",
            "audio/music/ssanne.asm",
            "audio/music/cities2.asm",
            "audio/music/celadon.asm",
            "audio/music/cinnabar.asm",
            "audio/music/vermilion.asm",
            "audio/music/lavender.asm",
            "audio/music/safarizone.asm",
            "audio/music/gym.asm",
            "audio/music/pokecenter.asm",
            "audio/sfx/pokeflute_ch4_ch5.asm",
            "audio/sfx/unused2_2.asm",
            "audio/music/gymleaderbattle.asm",
            "audio/music/trainerbattle.asm",
            "audio/music/wildbattle.asm",
            "audio/music/finalbattle.asm",
            "audio/sfx/level_up.asm",
            "audio/sfx/get_item2_2.asm",
            "audio/sfx/caught_mon.asm",
            "audio/music/defeatedtrainer.asm",
            "audio/music/defeatedwildmon.asm",
            "audio/music/defeatedgymleader.asm",
            "audio/music/bikeriding.asm",
            "audio/music/dungeon1.asm",
            "audio/music/gamecorner.asm",
            "audio/music/titlescreen.asm",
            "audio/sfx/get_item1_3.asm",
            "audio/music/dungeon2.asm",
            "audio/music/dungeon3.asm",
            "audio/music/cinnabarmansion.asm",
            "audio/sfx/pokedex_rating_3.asm",
            "audio/sfx/get_item2_3.asm",
            "audio/sfx/get_key_item_3.asm",
            "audio/music/oakslab.asm",
            "audio/music/pokemontower.asm",
            "audio/music/silphco.asm",
            "audio/music/meeteviltrainer.asm",
            "audio/music/meetfemaletrainer.asm",
            "audio/music/meetmaletrainer.asm",
            "audio/music/introbattle.asm",
            "audio/music/surfing.asm",
            "audio/music/jigglypuffsong.asm",
            "audio/music/halloffame.asm",
            "audio/music/credits.asm",
        };

        public Tuple<int, string, bool>[] HeaderPaths =
        {
            new Tuple<int, string, bool>(1, "audio/headers/musicheaders1.asm", true),
            new Tuple<int, string, bool>(2, "audio/headers/musicheaders2.asm", true),
            new Tuple<int, string, bool>(3, "audio/headers/musicheaders3.asm", true),
            new Tuple<int, string, bool>(1, "audio/headers/sfxheaders1.asm", false),
            new Tuple<int, string, bool>(2, "audio/headers/sfxheaders2.asm", false),
            new Tuple<int, string, bool>(3, "audio/headers/sfxheaders3.asm", false),
        };

        #endregion

        public Dictionary<string, AudioSequence> Sequences = new Dictionary<string, AudioSequence>();
        public Dictionary<string, AudioHeader> Headers = new Dictionary<string, AudioHeader>();

        public enum DuplicateBehavior
        {
            LeaveEverythingUntouched,
            NormalizeButLeaveDuplicates,
            NormalizeAndRemoveDuplicates,
        }

        public AudioLibrary(string basePath, DuplicateBehavior behavior = DuplicateBehavior.NormalizeAndRemoveDuplicates)
        {
            LoadAudioSequences(basePath);
            LoadAudioHeaders(basePath, behavior);
            CheckReferences();
        }

        public void LoadAudioSequences(string basePath)
        {
            var parser = new AudioSequencesParser();
            foreach (var path in DataPaths)
                foreach (var sequence in parser.ParseFile(basePath + path))
                    Sequences[sequence.Name] = sequence;
        }

        private static HashSet<string> NoiseInstruments = new HashSet<string>
        {
            "SFX_Snare1",
            "SFX_Snare2",
            "SFX_Snare3",
            "SFX_Snare4",
            "SFX_Snare5",
            "SFX_Triangle1",
            "SFX_Triangle2",
            "SFX_Snare6",
            "SFX_Snare7",
            "SFX_Snare8",
            "SFX_Snare9",
            "SFX_Cymbal1",
            "SFX_Cymbal2",
            "SFX_Cymbal3",
            "SFX_Muted_Snare1",
            "SFX_Triangle3",
            "SFX_Muted_Snare2",
            "SFX_Muted_Snare3",
            "SFX_Muted_Snare4",
        };

        private static HashSet<string> PokemonCries = new HashSet<string>
        {
            "SFX_Cry00",
            "SFX_Cry01",
            "SFX_Cry02",
            "SFX_Cry03",
            "SFX_Cry04",
            "SFX_Cry05",
            "SFX_Cry06",
            "SFX_Cry07",
            "SFX_Cry08",
            "SFX_Cry09",
            "SFX_Cry0A",
            "SFX_Cry0B",
            "SFX_Cry0C",
            "SFX_Cry0D",
            "SFX_Cry0E",
            "SFX_Cry0F",
            "SFX_Cry10",
            "SFX_Cry11",
            "SFX_Cry12",
            "SFX_Cry13",
            "SFX_Cry14",
            "SFX_Cry15",
            "SFX_Cry16",
            "SFX_Cry17",
            "SFX_Cry18",
            "SFX_Cry19",
            "SFX_Cry1A",
            "SFX_Cry1B",
            "SFX_Cry1C",
            "SFX_Cry1D",
            "SFX_Cry1E",
            "SFX_Cry1F",
            "SFX_Cry20",
            "SFX_Cry21",
            "SFX_Cry22",
            "SFX_Cry23",
            "SFX_Cry24",
            "SFX_Cry25",
        };

        public void LoadAudioHeaders(string basePath, DuplicateBehavior behavior)
        {
            var parser = new AudioHeadersParser();
            foreach (var path in HeaderPaths)
            {
                foreach (var header in parser.ParseFile(basePath + path.Item2, path.Item1, path.Item3))
                {
                    var reducedName = header.Name;
                    bool reduced = reducedName.EndsWith($"_{header.Bank}");
                    if (reduced)
                        reducedName = reducedName.Substring(0, reducedName.Length - 2);

                    if (behavior != DuplicateBehavior.LeaveEverythingUntouched)
                    {
                        if (reduced)
                        {
                            if (Headers.ContainsKey(reducedName) && behavior == DuplicateBehavior.NormalizeAndRemoveDuplicates)
                            {
                                Console.WriteLine($"Ignoring duplicate header {header.Name}");
                                continue;
                            }
                            header.Name = reducedName;
                        }
                    }

                    if (NoiseInstruments.Contains(reducedName))
                        header.ResourceType = ResourceType.NoiseInstrument;
                    else if (PokemonCries.Contains(reducedName))
                        header.ResourceType = ResourceType.Cry;

                    Headers[header.Name] = header;
                }
            }
        }

        public void Output(TextWriter tw)
        {
            foreach (var sequence in Sequences.Values)
                sequence.Write(tw);
            tw.WriteLine(":headers");
            OutputHeaders(tw);
        }

        public void OutputHeaders(TextWriter tw)
        {
            foreach (var header in Headers.Values)
                header.Write(tw);
        }

        public int TotalSize => Sequences.Values.Sum(x => x.TotalSize);
        public int MaxSize => (TotalSize > 0 ? 1 : 0) * AudioCommand.MaxSize;

        private void CheckReferences()
        {
            foreach (var sequence in Sequences.Values)
                foreach (var referenced in sequence.ReferencedSequences())
                    if (!Sequences.ContainsKey(referenced))
                        throw new Exception($"Sequence {sequence.Name} contains a reference to non-existing sequence {referenced}.");

            foreach (var header in Headers.Values)
                foreach (var channel in header.Channels)
                    if (!Sequences.ContainsKey(channel.Sequence))
                        throw new Exception($"Header {header.Name} contains a reference to non-existing sequence {channel.Sequence}.");
        }

        public void RemoveUnreachableSequences()
        {
            var used = new HashSet<string>();
            var stack = new Stack<string>();
            foreach (var header in Headers.Values)
            {
                foreach (var channel in header.Channels)
                    stack.Push(channel.Sequence);
            }
            while (stack.Count > 0)
            {
                var head = stack.Pop();
                if (used.Contains(head))
                    continue;
                used.Add(head);
                var sequence = Sequences[head];
                foreach (var referencedSequence in sequence.ReferencedSequences())
                    stack.Push(referencedSequence);
            }

            foreach (var s in Sequences.Keys.Where(x => !used.Contains(x)).ToArray())
            {
                Console.WriteLine($"Removing sequence {s}");
                Sequences.Remove(s);
            }
        }
    }
}
