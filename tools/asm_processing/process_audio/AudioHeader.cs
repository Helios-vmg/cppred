using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace process_audio
{
    class AudioHeader
    {
        public string Name;
        public int Bank;
        public bool IsMusic;
        public List<Channel> Channels = new List<Channel>();

        public void Write(TextWriter tw)
        {
            var isMusic = IsMusic ? 1 : 0;
            tw.WriteLine($".{Name} {Bank} {isMusic}");
            foreach (var channel in Channels)
                tw.WriteLine($"\tchannel {channel.Sequence} {channel.ChannelNo}");
            tw.WriteLine();
        }

        public class Channel
        {
            public int ChannelNo;
            public string Sequence;

            public Channel(int channel, string sequence)
            {
                ChannelNo = channel;
                Sequence = sequence;
            }
        }
    }
}
