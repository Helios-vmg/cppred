using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ParseRgbdsObj
{
    class Program
    {
        static void Main(string[] args)
        {
            try
            {
                var parser = new ObjParser(@"audio_red.o");
                //foreach (var symbol in parser.Symbols)
                //{
                //    Console.WriteLine(symbol.Name);
                //}

                var symbols = new[]
                {
                    "Audio1_WavePointers",
                    "Audio2_WavePointers",
                    "Audio3_WavePointers",
                };

                {
                    var symbol = parser.Symbols.Find(s => s.Name == symbols[0]);
                    var section = parser.Sections[(int)symbol.SectionId];
                    for (int i = 0; i < 5; i++)
                    {
                        var data = section.Data.Skip((int) symbol.SectionOffset + 9*2 + 16*i).Take(16).ToArray();
                        Console.WriteLine($"{{ 0x{BitConverter.ToString(data).ToUpper().Replace("-", ", 0x")}, }},");
                    }
                }

                foreach (var symbolName in symbols)
                {
                    var symbol = parser.Symbols.Find(s => s.Name == symbolName);
                    var section = parser.Sections[(int) symbol.SectionId];
                    var data = section.Data.Skip((int)symbol.SectionOffset + 9 * 2 + 16 * 5).Take(16).ToArray();
                    Console.WriteLine($"{{ 0x{BitConverter.ToString(data).ToUpper().Replace("-", ", 0x")}, }},");
                }
            }
            catch (Exception e)
            {
                Console.Error.WriteLine(e.Message);
            }
        }
    }
}
