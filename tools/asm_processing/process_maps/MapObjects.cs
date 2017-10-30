using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace process_maps
{
    class Teleporter
    {
        public int PositionX;
        public int PositionY;
        public MapHeader Destination;
    }

    class MapObjects
    {
        public string LabelName;
        public int BorderBlock;
        public List<Teleporter> Teleporters;

        public MapObjects(string path)
        {
            /*
            using (var file = new StreamReader(path))
            {
                string line;
                int state = 0;
                while ((line = file.ReadLine()) != null)
                {
                    switch (state)
                    {
                        case 0:
                            break;
                    }
                }
            }
            */
        }
    }
}
