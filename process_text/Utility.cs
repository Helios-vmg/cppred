using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace process_text
{
    static class Utility
    {
        public static string[] ToStrings(this Match match)
        {
            var list = new List<string>();
            foreach (var g in match.Groups)
                list.Add(g.ToString());
            return list.Skip(1).ToArray();
        }
    }
}
