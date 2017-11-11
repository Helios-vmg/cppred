using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace process_wild_encounters
{
    static class Utility
    {
        public static Dictionary<K, V> ToDictionaryRemoveDuplicates<K, V>(this IEnumerable<V> xs, Func<V, K> f)
        {
            var ret = new Dictionary<K, V>();
            foreach (var x in xs)
            {
                var k = f(x);
                ret[k] = x;
            }
            return ret;
        }

        public static Tuple<A, B> ToTuple<A, B>(A a, B b)
        {
            return new Tuple<A, B>(a, b);
        }

        public static Tuple<A, B, C> ToTuple<A, B, C>(A a, B b, C c)
        {
            return new Tuple<A, B, C>(a, b, c);
        }
    }
}
