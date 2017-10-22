using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace process_audio
{
    class Program
    {
        static void Main(string[] args)
        {
            try
            {
                Tuple<string, AudioLibrary.DuplicateBehavior, bool>[] array =
                {
                    new Tuple<string, AudioLibrary.DuplicateBehavior, bool>("audio.txt", AudioLibrary.DuplicateBehavior.LeaveEverythingUntouched, false),
                    new Tuple<string, AudioLibrary.DuplicateBehavior, bool>("custom_audio_headers.txt", AudioLibrary.DuplicateBehavior.NormalizeAndRemoveDuplicates, true),
                };
                foreach (var tuple in array)
                {
                    var lib = new AudioLibrary("../../pokered/", tuple.Item2);
                    using (var file = new StreamWriter(tuple.Item1))
                    {
                        if (!tuple.Item3)
                            lib.Output(file);
                        else
                            lib.OutputHeaders(file);
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
