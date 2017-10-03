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
            var parser = new ObjParser(@"audio_red.o");
            //foreach (var symbol in parser.Symbols)
            //{
            //    Console.WriteLine(symbol.Name);
            //}

            for (int i = 6; i <= 8; i++)
            {
                var section = parser.Sections[i];
                Console.WriteLine(Convert.ToBase64String(section.Data.Skip(198).Take(16).ToArray()));
            }
        }
    }
}
