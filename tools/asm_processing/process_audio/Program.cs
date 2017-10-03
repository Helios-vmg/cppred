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
                var lib = new AudioLibrary("../../pokered/", AudioLibrary.DuplicateBehavior.NormalizeAndRemoveDuplicates);
                //lib.RemoveUnreachableSequences();
                using (var file = new StreamWriter("audio.txt"))
                    lib.Output(file);
            }
            catch (Exception e)
            {
                Console.Error.WriteLine(e.Message);
            }
        }
    }
}
