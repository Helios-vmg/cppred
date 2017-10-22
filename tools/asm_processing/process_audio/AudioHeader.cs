using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace process_audio
{
    enum ResourceType
    {
        Music,
        NoiseInstrument,
        Cry,
        Sfx,
    }

    class AudioHeader
    {
        public string Name;
        public int Bank;
        public List<Channel> Channels = new List<Channel>();
        public ResourceType ResourceType;

        private string TypeToString()
        {
            switch (ResourceType)
            {
                case ResourceType.Music:
                    return "music";
                case ResourceType.NoiseInstrument:
                    return "noise";
                case ResourceType.Cry:
                    return "cry";
                case ResourceType.Sfx:
                    return "sfx";
                default:
                    throw new ArgumentOutOfRangeException();
            }
        }

        public void Write(TextWriter tw)
        {
            tw.WriteLine($".{Name} {Bank} {TypeToString()}");
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
