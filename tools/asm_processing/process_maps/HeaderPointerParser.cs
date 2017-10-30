using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace process_maps
{
    static class HeaderPointerParser
    {
        private static Regex _regex = new Regex(@"^dw\s+([A-Za-z_][A-Za-z_0-9]*)$");

        public static List<string> Parse(string path)
        {
            var ret = new List<string>();
            using (var file = new StreamReader(path))
            {
                string line;
                while ((line = file.ReadLine()) != null)
                {
                    line = Utility.RemoveComment(line).Trim();
                    if (line.Length == 0)
                        continue;
                    var match = _regex.Match(line);
                    if (!match.Success)
                        continue;
                    ret.Add(match.Groups[1].ToString());
                }
            }
            return ret;
        }
    }
}
