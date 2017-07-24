using System;
using System.Collections.Generic;
using System.Linq;
using System.Diagnostics;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;

namespace ProcessWram
{
    class Program
    {
        enum SymbolType{
            SYM_LOCAL = 0,
            SYM_IMPORT,
            SYM_EXPORT
        }

        enum SectionType
        {
            SECT_WRAM0 = 0,
            SECT_VRAM,
            SECT_ROMX,
            SECT_ROM0,
            SECT_HRAM,
            SECT_WRAMX,
            SECT_SRAM
        }

        static uint ReadByte(FileStream input)
        {
            var buffer = new byte[4];
            input.Read(buffer, 0, 1);
            return BitConverter.ToUInt32(buffer, 0);
        }

        static uint ReadDword(FileStream input)
        {
            var buffer = new byte[4];
            input.Read(buffer, 0, buffer.Length);
            return BitConverter.ToUInt32(buffer, 0);
        }

        static string ReadString(FileStream input)
        {
            var list = new List<byte>();
            while (true)
            {
                var b = input.ReadByte();
                if (b == 0)
                    break;
                list.Add((byte)b);
            }
            return Encoding.UTF8.GetString(list.ToArray());
        }

        class Symbol
        {
            public string Name;
            public SymbolType Type;
            public uint SectionId;
            public uint Offset;

            public Symbol(FileStream file)
            {
                Name = ReadString(file);
                Type = (SymbolType)ReadByte(file);
                if (Type == SymbolType.SYM_IMPORT)
                    return;
                SectionId = ReadDword(file);
                Offset = ReadDword(file);
            }
        }

        class Section
        {
            public uint Pc;
            public SectionType Type;
            public uint Org;
            public uint Bank;

            public Section(FileStream input)
            {
                Pc = ReadDword(input);
                Type = (SectionType) ReadByte(input);
                Org = ReadDword(input);
                Bank = ReadDword(input);
                if (Type == SectionType.SECT_ROM0 || Type == SectionType.SECT_ROMX)
                    Debugger.Break();
            }
        }

        static void Main(string[] args)
        {
            var symbols = new List<Symbol>();
            var sections = new List<Section>();
            using (var file = new FileStream(@"f:\Data\Projects\Emulator and Pokemon port\pokered\wram_red.o", FileMode.Open, FileAccess.Read, FileShare.Read))
            {
                file.Seek(4, SeekOrigin.Begin);
                var symbolCount = ReadDword(file);
                var sectionCount = ReadDword(file);
                for (uint i = 0; i < symbolCount; i++)
                    symbols.Add(new Symbol(file));
                for (uint i = 0; i < sectionCount; i++)
                    sections.Add(new Section(file));

            }
            //symbols = symbols.OrderBy(x => sections[(int) x.SectionId].Org + x.Offset).ToList();
            symbols = symbols.OrderBy(x => x.SectionId).ThenBy(x => x.Offset).ToList();
            sections[3].Org = 0xD000;
            sections[5].Org = 0x0A000;
            sections[6].Org = 0x1A000;
            sections[7].Org = 0x2A000;
            sections[8].Org = 0x3A000;
            foreach (var symbol in symbols)
            {
                var org = sections[(int) symbol.SectionId].Org;
                Console.WriteLine($"{symbol.Name}\t{org + symbol.Offset:X4}");
            }
            /*
            var lines = new List<string>();
            using (var input = new StreamReader("wram.asm"))
            {
                while (true)
                {
                    var line = input.ReadLine();
                    if (line == null)
                        break;
                    lines.Add(line);
                }
            }

            var regexes = new List<Regex>
            {
                new Regex(@"(w[a-zA-Z_0-9]+)\:\:"),
                new Regex(@"()"),
            };
            */
        }
    }
}
