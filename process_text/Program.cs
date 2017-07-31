using System;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.Remoting.Lifetime;
using System.Text;
using System.Threading.Tasks;

namespace process_text
{
    class Program
    {
        static void Main(string[] args)
        {
            try
            {
                var processor = new TextProcessor();
                processor.ProcessText(@"f:\Data\Projects\Emulator and Pokemon port\pokered\text.asm");
                processor.Clean();
                processor.CheckAssertions();
                using (var file = new FileStream("output.txt", FileMode.Create, FileAccess.Write, FileShare.None))
                using (var source = new StreamWriter(file, Encoding.UTF8))
                    processor.GenerateOutput(source);
                using (var file = new FileStream("output2.txt", FileMode.Create, FileAccess.Write, FileShare.None))
                using (var source = new StreamWriter(file, Encoding.UTF8))
                    processor.GenerateOutput2(source);
            }
            catch (Exception e)
            {
                Console.Error.WriteLine(e.Message);
            }
        }
    }
}
