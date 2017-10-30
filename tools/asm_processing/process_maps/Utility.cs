using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace process_maps
{
    static class Utility
    {
        public static string RemoveComment(string s)
        {
            var ret = new StringBuilder();
            foreach (var c in s)
            {
                if (c == ';')
                    break;
                ret.Append(c);
            }
            return ret.ToString();
        }

        public static string SNAKE_CASE_toCamelCase(string s)
        {
            var ret = new StringBuilder();
            bool useUpper = true;
            foreach (var c in s)
            {
                if (c == '_')
                {
                    useUpper = true;
                    continue;
                }
                if (!char.IsLetter(c))
                {
                    ret.Append(c);
                    useUpper = true;
                    continue;
                }
                ret.Append(useUpper ? char.ToUpper(c) : char.ToLower(c));
                useUpper = false;
            }
            return ret.ToString();
        }

        public static HashSet<T> ToSet<T>(this IEnumerable<T> xs)
        {
            var ret = new HashSet<T>();
            foreach (var x in xs)
                ret.Add(x);
            return ret;
        }

        public static string GetMd5(this byte[] buffer)
        {
            return BitConverter.ToString(System.Security.Cryptography.MD5.Create().ComputeHash(buffer)).ToLower().Replace("-", "");
        }
    }
}
