using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace process_audio
{
    class AudioSequence
    {
        public readonly string Name;
        public readonly List<AudioCommand> Commands = new List<AudioCommand>();

        public AudioSequence(string name)
        {
            Name = name;
        }

        public void Write(TextWriter tw)
        {
            tw.WriteLine($".{Name}");
            foreach (var command in Commands)
            {
                tw.Write("\t");
                command.Write(tw);
            }
            tw.WriteLine();
        }

        public int TotalSize => Commands.Sum(x => x.TotalSize);

        public IEnumerable<string> ReferencedSequences()
        {
            foreach (var command in Commands)
            {

                var lcc = command as LoopChannelCommand;
                if (lcc != null)
                    yield return lcc.Param2;
                var ccc = command as CallChannelCommand;
                if (ccc != null)
                    yield return ccc.Param1;
                var gc = command as GotoCommand;
                if (gc != null)
                    yield return gc.Param1;
            }
        }
    }

    abstract class AudioCommand
    {
        public abstract void Write(TextWriter tw);
        public abstract int TotalSize { get; }

        private static int EstimateSize2(int n)
        {
            if (n < 0x80)
                return 1;
            if (n < 0xC000)
                return 2;
            if (n < 0xE00000)
                return 3;
            return 4;
        }

        public static int MaxSize = 0;

        protected static int EstimateSize(int n)
        {
            var ret = EstimateSize2(n);
            if (ret > MaxSize)
                MaxSize = ret;
            return ret;
        }
    }

    class TempoCommand : AudioCommand
    {
        public int Param1;
        public override void Write(TextWriter tw)
        {
            tw.WriteLine($"tempo {Param1}");
        }
        public override int TotalSize => 1 + EstimateSize(Param1);
    }

    class VolumeCommand : AudioCommand
    {
        public int Param1, Param2;
        public override void Write(TextWriter tw)
        {
            tw.WriteLine($"volume {Param1} {Param2}");
        }
        public override int TotalSize => 1 + EstimateSize(Param1) + EstimateSize(Param2);
    }

    class DutyCommand : AudioCommand
    {
        public int Param1;
        public override void Write(TextWriter tw)
        {
            tw.WriteLine($"duty {Param1}");
        }
        public override int TotalSize => 1 + EstimateSize(Param1);
    }

    class DutyCycleCommand : AudioCommand
    {
        public int Param1;
        public override void Write(TextWriter tw)
        {
            tw.WriteLine($"dutycycle {Param1}");
        }
        public override int TotalSize => 1 + EstimateSize(Param1);
    }

    class VibratoCommand : AudioCommand
    {
        public int Param1, Param2, Param3;
        public override void Write(TextWriter tw)
        {
            tw.WriteLine($"vibrato {Param1} {Param2} {Param3}");
        }
        public override int TotalSize => 1 + EstimateSize(Param1) + EstimateSize(Param2) + EstimateSize(Param3);
    }

    class TogglePerfectPitchCommand : AudioCommand
    {
        public override void Write(TextWriter tw)
        {
            tw.WriteLine($"toggle_perfect_pitch");
        }
        public override int TotalSize => 1;
    }

    class NoteTypeCommand : AudioCommand
    {
        public int Param1, Param2, Param3;
        public override void Write(TextWriter tw)
        {
            tw.WriteLine($"note_type {Param1} {Param2} {Param3}");
        }
        public override int TotalSize => 1 + EstimateSize(Param1) + EstimateSize(Param2) + EstimateSize(Param3);
    }

    class RestCommand : AudioCommand
    {
        public int Param1;
        public override void Write(TextWriter tw)
        {
            tw.WriteLine($"rest {Param1}");
        }
        public override int TotalSize => 1 + EstimateSize(Param1);
    }

    class OctaveCommand : AudioCommand
    {
        public int Param1;
        public override void Write(TextWriter tw)
        {
            tw.WriteLine($"octave {Param1}");
        }
        public override int TotalSize => 1 + EstimateSize(Param1);
    }

    enum NoteType
    {
        C,
        CSharp,
        D,
        DSharp,
        E,
        F,
        FSharp,
        G,
        GSharp,
        A,
        ASharp,
        B
    }

    class NoteCommand : AudioCommand
    {
        public NoteType Type;
        public int Param1;
        public override void Write(TextWriter tw)
        {
            tw.WriteLine($"note {Type} {Param1}");
        }
        public override int TotalSize => 1 + EstimateSize((int)Type) + EstimateSize(Param1);
    }

    class EndChannelCommand : AudioCommand
    {
        public override void Write(TextWriter tw)
        {
            tw.WriteLine($"end");
        }
        public override int TotalSize => 1;
    }

    class DSpeedCommand : AudioCommand
    {
        public int Param1;
        public override void Write(TextWriter tw)
        {
            tw.WriteLine($"dspeed {Param1}");
        }
        public override int TotalSize => 1 + EstimateSize(Param1);
    }

    class SnareCommand : AudioCommand
    {
        public int Type, Param1;
        public override void Write(TextWriter tw)
        {
            tw.WriteLine($"snare {Type} {Param1}");
        }
        public override int TotalSize => 1 + EstimateSize(Type) + EstimateSize(Param1);
    }

    class MutedSnareCommand : AudioCommand
    {
        public int Type, Param1;
        public override void Write(TextWriter tw)
        {
            tw.WriteLine($"muted_snare {Type} {Param1}");
        }
        public override int TotalSize => 1 + EstimateSize(Type) + EstimateSize(Param1);
    }

    class UnknownSfx10Command : AudioCommand
    {
        public int Param1;
        public override void Write(TextWriter tw)
        {
            tw.WriteLine($"unknown_sfx_10 {Param1}");
        }
        public override int TotalSize => 1 + EstimateSize(Param1);
    }

    class UnknownSfx20Command : AudioCommand
    {
        public int Param1;
        public int Param2;
        public int Param3;
        public int Param4;
        public override void Write(TextWriter tw)
        {
            tw.WriteLine($"unknown_sfx_20 {Param1} {Param2} {Param3} {Param4}");
        }
        public override int TotalSize => 1 + EstimateSize(Param1) + EstimateSize(Param2) + EstimateSize(Param3) + EstimateSize(Param4);
    }

    class UnknownNoise20Command : AudioCommand
    {
        public int Param1;
        public int Param2;
        public int Param3;
        public override void Write(TextWriter tw)
        {
            tw.WriteLine($"unknown_noise_20 {Param1} {Param2} {Param3}");
        }
        public override int TotalSize => 1 + EstimateSize(Param1) + EstimateSize(Param2) + EstimateSize(Param3);
    }

    class ExecuteMusicCommand : AudioCommand
    {
        public override void Write(TextWriter tw)
        {
            tw.WriteLine($"execute_music");
        }
        public override int TotalSize => 1;
    }

    class PitchBendCommand : AudioCommand
    {
        public int Param1;
        public int Param2;
        public override void Write(TextWriter tw)
        {
            tw.WriteLine($"pitch_bend {Param1} {Param2}");
        }
        public override int TotalSize => 1 + EstimateSize(Param1) + EstimateSize(Param2);
    }

    class TriangleCommand : AudioCommand
    {
        public int Type;
        public int Param1;
        public override void Write(TextWriter tw)
        {
            tw.WriteLine($"triangle {Type} {Param1}");
        }
        public override int TotalSize => 1 + EstimateSize(Type);
    }

    class StereoPanningCommand : AudioCommand
    {
        public int Param1;
        public override void Write(TextWriter tw)
        {
            tw.WriteLine($"stereo_panning {Param1}");
        }
        public override int TotalSize => 1 + EstimateSize(Param1);
    }

    class CymbalCommand : AudioCommand
    {
        public int Type;
        public int Param1;
        public override void Write(TextWriter tw)
        {
            tw.WriteLine($"cymbal {Type} {Param1}");
        }
        public override int TotalSize => 1 + EstimateSize(Type) + EstimateSize(Param1);
    }

    class LoopChannelCommand : AudioCommand
    {
        public int Param1;
        public string Param2;
        public override void Write(TextWriter tw)
        {
            tw.WriteLine($"loop {Param1} {Param2}");
        }
        public override int TotalSize => 1 + EstimateSize(Param1) + 4;
    }

    class CallChannelCommand : AudioCommand
    {
        public string Param1;
        public override void Write(TextWriter tw)
        {
            tw.WriteLine($"call {Param1}");
        }
        public override int TotalSize => 1 + 4;
    }

    class GotoCommand : AudioCommand
    {
        public string Param1;
        public override void Write(TextWriter tw)
        {
            tw.WriteLine($"goto {Param1}");
        }
        public override int TotalSize => 1 + 4;
    }

    class IfRedCommand : AudioCommand
    {
        public override void Write(TextWriter tw)
        {
            tw.WriteLine("ifred");
        }
        public override int TotalSize => 1;
    }

    class ElseCommand : AudioCommand
    {
        public override void Write(TextWriter tw)
        {
            tw.WriteLine("else");
        }
        public override int TotalSize => 1;
    }

    class EndIfCommand : AudioCommand
    {
        public override void Write(TextWriter tw)
        {
            tw.WriteLine("endif");
        }
        public override int TotalSize => 1;
    }
}
