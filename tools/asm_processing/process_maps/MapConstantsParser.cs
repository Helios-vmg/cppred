using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace process_maps
{
    static class MapConstantsParser
    {
        private static Regex _regex = new Regex(@"^\s*mapconst\s+([A-Za-z_][A-Za-z_0-9]*)\s*\,\s*([0-9]+)\s*\,\s*([0-9]+)\s*$");

        public static Dictionary<string, int> Parse(string path)
        {
            var ret = new Dictionary<string, int>();
            int n = 0;
            using (var file = new StreamReader(path))
            {
                string line;
                while ((line = file.ReadLine()) != null)
                {
                    line = Utility.RemoveComment(line);
                    if (line.Length == 0)
                        continue;
                    var match = _regex.Match(line);
                    if (!match.Success)
                        continue;
                    ret[match.Groups[1].ToString()] = n++;
                    ret[match.Groups[1].ToString() + "_HEIGHT"] = Convert.ToInt32(match.Groups[2].ToString());
                    ret[match.Groups[1].ToString() + "_WIDTH"] = Convert.ToInt32(match.Groups[3].ToString());
                }
            }
            return ret;
        }
    }
}
