using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace process_audio
{
    class AudioSequencesParser
    {
        class ParseState
        {
            public List<AudioSequence> FinalResult;
            public AudioSequence CurrentSequence;
            public Match LineMatch;
        }

        private List<Tuple<Regex, Action<ParseState>>> _regexes;

        private static Tuple<Regex, Action<ParseState>> NaryCommand(string name, int parameters, Action<ParseState> callback)
        {
            name = Regex.Escape(name);
            if (parameters == 0)
                return new Tuple<Regex, Action<ParseState>>(new Regex($@"^\s*{name}\s*$"), callback);
            var sb = new StringBuilder();
            sb.Append($@"^\s*{name}\s+([0-9]+)");
            for (int i = 1; i < parameters; i++)
                sb.Append(@"\,\s*([0-9]+)");
            sb.Append(@"\s*$");
            return new Tuple<Regex, Action<ParseState>>(new Regex(sb.ToString()), callback);
        }

        private Action<ParseState> note(NoteType type)
        {
            return s => HandleNote(s, type);
        }

        private Action<ParseState> snare(int type)
        {
            return s => HandleSnare(s, type);
        }

        private Action<ParseState> mutedsnare(int type)
        {
            return s => HandleMutedSnare(s, type);
        }

        private Action<ParseState> triangle(int type)
        {
            return s => HandleTriangle(s, type);
        }

        private Action<ParseState> cymbal(int type)
        {
            return s => HandleCymbal(s, type);
        }

        private void InitializeRegexes()
        {
            _regexes = new List<Tuple<Regex, Action<ParseState>>>
            {
                new Tuple<Regex, Action<ParseState>>(new Regex(@"^\s*$"), HandleNothing),
                new Tuple<Regex, Action<ParseState>>(new Regex(@"^\s*([A-Za-z_][A-Za-z_0-9]*)\s*\:\:?\s*$"), HandleLabel),
                NaryCommand("tempo", 1, HandleTempo),
                NaryCommand("volume", 2, HandleVolume),
                NaryCommand("duty", 1, HandleDuty),
                NaryCommand("dutycycle", 1, HandleDutyCycle),
                NaryCommand("vibrato", 3, HandleVibrato),
                NaryCommand("toggleperfectpitch", 0, HandleTogglePerfectPitch),
                NaryCommand("notetype", 3, HandleNoteType),
                NaryCommand("rest", 1, HandleRest),
                NaryCommand("octave", 1, HandleOctave),
                NaryCommand("C_", 1, note(NoteType.C)),
                NaryCommand("C#", 1, note(NoteType.CSharp)),
                NaryCommand("D_", 1, note(NoteType.D)),
                NaryCommand("D#", 1, note(NoteType.DSharp)),
                NaryCommand("E_", 1, note(NoteType.E)),
                NaryCommand("F_", 1, note(NoteType.F)),
                NaryCommand("F#", 1, note(NoteType.FSharp)),
                NaryCommand("G_", 1, note(NoteType.G)),
                NaryCommand("G#", 1, note(NoteType.GSharp)),
                NaryCommand("A_", 1, note(NoteType.A)),
                NaryCommand("A#", 1, note(NoteType.ASharp)),
                NaryCommand("B_", 1, note(NoteType.B)),
                NaryCommand("endchannel", 0, HandleEndChannel),
                NaryCommand("dspeed", 1, HandleDSpeed),
                NaryCommand("snare1", 1, snare(1)),
                NaryCommand("snare2", 1, snare(2)),
                NaryCommand("snare3", 1, snare(3)),
                NaryCommand("snare4", 1, snare(4)),
                NaryCommand("snare5", 1, snare(5)),
                NaryCommand("snare6", 1, snare(6)),
                NaryCommand("snare7", 1, snare(7)),
                NaryCommand("snare8", 1, snare(8)),
                NaryCommand("snare9", 1, snare(9)),
                NaryCommand("mutedsnare1", 1, mutedsnare(1)),
                NaryCommand("mutedsnare2", 1, mutedsnare(2)),
                NaryCommand("mutedsnare3", 1, mutedsnare(3)),
                NaryCommand("mutedsnare4", 1, mutedsnare(4)),
                NaryCommand("unknownsfx0x10", 1, HandleUnknownFx10),
                NaryCommand("unknownsfx0x20", 4, HandleUnknownFx20),
                NaryCommand("unknownnoise0x20", 3, HandleUnknownNoise20),
                NaryCommand("executemusic", 0, HandleExecuteMusic),
                NaryCommand("pitchbend", 2, HandlePitchBend),
                NaryCommand("triangle1", 1, triangle(1)),
                NaryCommand("triangle2", 1, triangle(2)),
                NaryCommand("triangle3", 1, triangle(3)),
                NaryCommand("stereopanning", 1, HandleStereoPanning),
                NaryCommand("cymbal1", 1, cymbal(1)),
                NaryCommand("cymbal2", 1, cymbal(2)),
                NaryCommand("cymbal3", 1, cymbal(3)),
                new Tuple<Regex, Action<ParseState>>(new Regex(@"^\s*loopchannel\s+([0-9]+)\s*\,\s*([A-Za-z_][A-Za-z_0-9]*)\s*$"), HandleLoopChannel),
                new Tuple<Regex, Action<ParseState>>(new Regex(@"^\s*callchannel\s+([A-Za-z_][A-Za-z_0-9]*)\s*$"), HandleCallChannel),
                new Tuple<Regex, Action<ParseState>>(new Regex(@"^\s*IF\s*DEF\s*\(\s*_RED\s*\)\s*$"), HandleIfRed),
                new Tuple<Regex, Action<ParseState>>(new Regex(@"^\s*ELSE\s*$"), HandleElse),
                new Tuple<Regex, Action<ParseState>>(new Regex(@"^\s*ENDC\s*$"), HandleEndIf),
            };
        }

        public AudioSequencesParser()
        {
            InitializeRegexes();
        }

        private static List<string> LoadFile(string path)
        {
            var ret = new List<string>();
            using (var file = new StreamReader(path))
            {
                string line;
                while ((line = file.ReadLine()) != null)
                    ret.Add(line);
            }
            return ret;
        }

        public List<AudioSequence> ParseFile(string path)
        {
            var ret = new List<AudioSequence>();
            var lines = LoadFile(path);
            int lineNo = 0;
            var state = new ParseState {FinalResult = ret};
            foreach (var line in lines)
            {
                lineNo++;
                var match = MatchLine(line);
                if (match == null)
                    throw new Exception($"Can't parse line {lineNo} of {path}: {line}");
                state.LineMatch = match.Item1;
                match.Item2(state);
            }
            return ret;
        }

        private Tuple<Match, Action<ParseState>>  MatchLine(string line)
        {
            foreach (var kv in _regexes)
            {
                var match = kv.Item1.Match(line);
                if (!match.Success)
                    continue;
                return new Tuple<Match, Action<ParseState>>(match, kv.Item2);
            }
            return null;
        }

        private void HandleNothing(ParseState state)
        {
        }

        private void HandleLabel(ParseState state)
        {
            var last = state.CurrentSequence;
            state.CurrentSequence = new AudioSequence(state.LineMatch.Groups[1].ToString());
            state.FinalResult.Add(state.CurrentSequence);
            if (last != null)
            {
                bool doGoto = false;
                if (last.Commands.Count == 0)
                    doGoto = true;
                else
                {
                    var lastCommand = last.Commands[last.Commands.Count - 1];
                    if (!(lastCommand is EndChannelCommand || lastCommand is LoopChannelCommand && ((LoopChannelCommand) lastCommand).Param1 == 0))
                        doGoto = true;
                }
                if (doGoto)
                    last.Commands.Add(new GotoCommand {Param1 = state.CurrentSequence.Name});
            }
        }

        private void HandleTempo(ParseState state)
        {
            state.CurrentSequence.Commands.Add(new TempoCommand
            {
                Param1 = Convert.ToInt32(state.LineMatch.Groups[1].ToString())
            });
        }

        private void HandleVolume(ParseState state)
        {
            state.CurrentSequence.Commands.Add(new VolumeCommand
            {
                Param1 = Convert.ToInt32(state.LineMatch.Groups[1].ToString()),
                Param2 = Convert.ToInt32(state.LineMatch.Groups[2].ToString()),
            });
        }

        private void HandleDuty(ParseState state)
        {
            state.CurrentSequence.Commands.Add(new DutyCommand
            {
                Param1 = Convert.ToInt32(state.LineMatch.Groups[1].ToString()),
            });
        }

        private void HandleDutyCycle(ParseState state)
        {
            state.CurrentSequence.Commands.Add(new DutyCycleCommand
            {
                Param1 = Convert.ToInt32(state.LineMatch.Groups[1].ToString()),
            });
        }

        private void HandleVibrato(ParseState state)
        {
            state.CurrentSequence.Commands.Add(new VibratoCommand
            {
                Param1 = Convert.ToInt32(state.LineMatch.Groups[1].ToString()),
                Param2 = Convert.ToInt32(state.LineMatch.Groups[2].ToString()),
                Param3 = Convert.ToInt32(state.LineMatch.Groups[3].ToString()),
            });
        }

        private void HandleTogglePerfectPitch(ParseState state)
        {
            state.CurrentSequence.Commands.Add(new TogglePerfectPitchCommand());
        }

        private void HandleNoteType(ParseState state)
        {
            state.CurrentSequence.Commands.Add(new NoteTypeCommand
            {
                Param1 = Convert.ToInt32(state.LineMatch.Groups[1].ToString()),
                Param2 = Convert.ToInt32(state.LineMatch.Groups[2].ToString()),
                Param3 = Convert.ToInt32(state.LineMatch.Groups[3].ToString()),
            });
        }

        private void HandleRest(ParseState state)
        {
            state.CurrentSequence.Commands.Add(new RestCommand
            {
                Param1 = Convert.ToInt32(state.LineMatch.Groups[1].ToString()),
            });
        }

        private void HandleOctave(ParseState state)
        {
            state.CurrentSequence.Commands.Add(new OctaveCommand
            {
                Param1 = Convert.ToInt32(state.LineMatch.Groups[1].ToString()),
            });
        }

        private void HandleNote(ParseState state, NoteType type)
        {
            state.CurrentSequence.Commands.Add(new NoteCommand
            {
                Type = type,
                Param1 = Convert.ToInt32(state.LineMatch.Groups[1].ToString()),
            });
        }

        private void HandleEndChannel(ParseState state)
        {
            state.CurrentSequence.Commands.Add(new EndChannelCommand());
        }

        private void HandleDSpeed(ParseState state)
        {
            state.CurrentSequence.Commands.Add(new DSpeedCommand
            {
                Param1 = Convert.ToInt32(state.LineMatch.Groups[1].ToString()),
            });
        }

        private void HandleSnare(ParseState state, int type)
        {
            state.CurrentSequence.Commands.Add(new SnareCommand
            {
                Type = type,
                Param1 = Convert.ToInt32(state.LineMatch.Groups[1].ToString()),
            });
        }

        private void HandleMutedSnare(ParseState state, int type)
        {
            state.CurrentSequence.Commands.Add(new MutedSnareCommand
            {
                Type = type,
                Param1 = Convert.ToInt32(state.LineMatch.Groups[1].ToString()),
            });
        }

        private void HandleUnknownFx10(ParseState state)
        {
            state.CurrentSequence.Commands.Add(new UnknownSfx10Command
            {
                Param1 = Convert.ToInt32(state.LineMatch.Groups[1].ToString()),
            });
        }

        private void HandleUnknownFx20(ParseState state)
        {
            state.CurrentSequence.Commands.Add(new UnknownSfx20Command
            {
                Param1 = Convert.ToInt32(state.LineMatch.Groups[1].ToString()),
                Param2 = Convert.ToInt32(state.LineMatch.Groups[2].ToString()),
                Param3 = Convert.ToInt32(state.LineMatch.Groups[3].ToString()),
                Param4 = Convert.ToInt32(state.LineMatch.Groups[4].ToString()),
            });
        }

        private void HandleUnknownNoise20(ParseState state)
        {
            state.CurrentSequence.Commands.Add(new UnknownNoise20Command
            {
                Param1 = Convert.ToInt32(state.LineMatch.Groups[1].ToString()),
                Param2 = Convert.ToInt32(state.LineMatch.Groups[2].ToString()),
                Param3 = Convert.ToInt32(state.LineMatch.Groups[3].ToString()),
            });
        }

        private void HandleExecuteMusic(ParseState state)
        {
            state.CurrentSequence.Commands.Add(new ExecuteMusicCommand());
        }

        private void HandlePitchBend(ParseState state)
        {
            state.CurrentSequence.Commands.Add(new PitchBendCommand
            {
                Param1 = Convert.ToInt32(state.LineMatch.Groups[1].ToString()),
                Param2 = Convert.ToInt32(state.LineMatch.Groups[2].ToString()),
            });
        }

        private void HandleTriangle(ParseState state, int type)
        {
            state.CurrentSequence.Commands.Add(new TriangleCommand
            {
                Type = type,
                Param1 = Convert.ToInt32(state.LineMatch.Groups[1].ToString()),
            });
        }

        private void HandleStereoPanning(ParseState state)
        {
            state.CurrentSequence.Commands.Add(new StereoPanningCommand
            {
                Param1 = Convert.ToInt32(state.LineMatch.Groups[1].ToString()),
            });
        }

        private void HandleCymbal(ParseState state, int type)
        {
            state.CurrentSequence.Commands.Add(new CymbalCommand
            {
                Type = type,
                Param1 = Convert.ToInt32(state.LineMatch.Groups[1].ToString()),
            });
        }

        private void HandleLoopChannel(ParseState state)
        {
            state.CurrentSequence.Commands.Add(new LoopChannelCommand
            {
                Param1 = Convert.ToInt32(state.LineMatch.Groups[1].ToString()),
                Param2 = state.LineMatch.Groups[2].ToString(),
            });
        }

        private void HandleCallChannel(ParseState state)
        {
            state.CurrentSequence.Commands.Add(new CallChannelCommand
            {
                Param1 = state.LineMatch.Groups[1].ToString(),
            });
        }

        private void HandleIfRed(ParseState state)
        {
            state.CurrentSequence.Commands.Add(new IfRedCommand());
        }

        private void HandleElse(ParseState state)
        {
            state.CurrentSequence.Commands.Add(new ElseCommand());
        }

        private void HandleEndIf(ParseState state)
        {
            state.CurrentSequence.Commands.Add(new EndIfCommand());
        }
    }
}
