using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Net.Configuration;
using System.Text;
using System.Threading.Tasks;
using ParseRgbdsObj;

namespace process_maps
{
    class Program
    {
        static void Main(string[] args)
        {
            try
            {
                var data = new Data("../../pokered/");
                data.WriteMaps();
                data.WriteMapData();
            }
            catch (Exception e)
            {
                Console.Error.WriteLine(e.Message);
            }
        }
    }
}
