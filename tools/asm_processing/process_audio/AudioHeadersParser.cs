using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace process_audio
{
    class AudioHeadersParser
    {
        class ParseState
        {
            public List<AudioHeader> FinalResult;
            public AudioHeader CurrentHeader;
            public Match LineMatch;
            public int Bank;
        }

        private List<Tuple<Regex, Action<ParseState>>> _regexes;
        
        private void InitializeRegexes()
        {
            _regexes = new List<Tuple<Regex, Action<ParseState>>>
            {
                new Tuple<Regex, Action<ParseState>>(new Regex(@"^\s*$"), HandleNothing),
                new Tuple<Regex, Action<ParseState>>(new Regex(@"^\s*\;.*"), HandleNothing),
                new Tuple<Regex, Action<ParseState>>(new Regex(@"^\s*([A-Za-z_][A-Za-z_0-9]*)\s*\:\:?\s*$"), HandleLabel),
                new Tuple<Regex, Action<ParseState>>(new Regex(@"^\s*db\s+\$ff\s*\,\s*\$ff\s*\,\s*\$ff.*"), HandleEmptyHeader),
            };
            for (int i = 1; i <= 5; i++)
            {
                var sb = new StringBuilder();
                sb.Append(@"^\s*audio\s+([A-Za-z_][A-Za-z_0-9]*)");
                for (int j = 0; j < i; j++)
                    sb.Append(@"\s*\,\s*(Ch[0-9])");
                sb.Append(@"\s*$");
                int c = i;
                _regexes.Add(new Tuple<Regex, Action<ParseState>>(new Regex(sb.ToString()), ps => HandleHeader(ps, c)));
            }
        }

        public AudioHeadersParser()
        {
            InitializeRegexes();
        }

        private static List<string> LoadFile(string path)
        {
            var ret = new List<string>();
            using (var file = new StreamReader(path))
            {
                string line;
                while ((line = file.ReadLine()) != null)
                    ret.Add(line);
            }
            return ret;
        }

        private Tuple<Match, Action<ParseState>> MatchLine(string line)
        {
            foreach (var kv in _regexes)
            {
                var match = kv.Item1.Match(line);
                if (!match.Success)
                    continue;
                return new Tuple<Match, Action<ParseState>>(match, kv.Item2);
            }
            return null;
        }

        private bool CurrentHeaderIsMusic = false;

        public List<AudioHeader> ParseFile(string path, int bank, bool isMusic)
        {
            CurrentHeaderIsMusic = isMusic;
            var ret = new List<AudioHeader>();
            var lines = LoadFile(path);
            int lineNo = 0;
            var state = new ParseState
            {
                FinalResult = ret,
                Bank = bank,
            };
            foreach (var line in lines)
            {
                lineNo++;
                var match = MatchLine(line);
                if (match == null)
                    throw new Exception($"Can't parse line {lineNo} of {path}: {line}");
                state.LineMatch = match.Item1;
                match.Item2(state);
            }
            return ret;
        }

        private void HandleNothing(ParseState state)
        {
        }

        private void HandleLabel(ParseState state)
        {
            if (state.FinalResult.Count > 0 && state.FinalResult[state.FinalResult.Count - 1].Channels.Count == 0)
                throw new Exception("Empty label.");

            state.CurrentHeader = new AudioHeader
            {
                Name = state.LineMatch.Groups[1].ToString(),
                Bank = state.Bank,
                IsMusic = CurrentHeaderIsMusic,
            };
            state.FinalResult.Add(state.CurrentHeader);
        }

        private void HandleHeader(ParseState state, int n)
        {
            if (state.CurrentHeader == null)
                throw new Exception("Header without a label!");
            var @base = state.LineMatch.Groups[1].ToString();
            for (int i = 0; i < n; i++)
            {
                var channel = state.LineMatch.Groups[i + 2].ToString();
                var channelNo = Convert.ToInt32(channel.Substring(2));
                channel = $"{@base}_{channel}";
                state.CurrentHeader.Channels.Add(new AudioHeader.Channel(channelNo, channel));
            }
        }

        private void HandleEmptyHeader(ParseState state)
        {
            if (state.CurrentHeader == null)
                throw new Exception("Header without a label!");
            state.FinalResult.RemoveAt(state.FinalResult.Count - 1);
            state.CurrentHeader = null;
        }

    }
}
