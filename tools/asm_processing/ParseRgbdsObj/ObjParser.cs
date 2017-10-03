using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ParseRgbdsObj
{
    class ObjParser
    {
        static byte ReadByte(byte[] buffer, ref int offset)
        {
            if (offset + 1 >= buffer.Length)
                throw new Exception();
            return buffer[offset++];
        }

        static uint ReadUint32(byte[] buffer, ref int offset)
        {
            if (offset + 4 > buffer.Length)
                throw new Exception();
            uint ret = 0;
            for (int i = 4; i-- != 0;)
            {
                ret <<= 8;
                ret |= buffer[offset + i];
            }
            offset += 4;
            return ret;
        }

        static string ReadString(byte[] buffer, ref int offset)
        {
            var ret = new List<byte>();
            while (true)
            {
                if (offset >= buffer.Length)
                    throw new Exception();
                if (buffer[offset] == 0)
                {
                    offset++;
                    break;
                }
                ret.Add(buffer[offset++]);
            }
            return Encoding.UTF8.GetString(ret.ToArray());
        }

        static byte[] ReadBuffer(byte[] buffer, ref int offset, uint size)
        {
            if (offset + size > buffer.Length)
                throw new Exception();
            var ret = new byte[size];
            Array.Copy(buffer, offset, ret, 0, size);
            offset += (int)size;
            return ret;
        }

        public readonly List<Symbol> Symbols = new List<Symbol>(); 
        public readonly List<Section> Sections = new List<Section>(); 

        public ObjParser(string path)
        {
            byte[] buffer;
            using (var file = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read))
            {
                buffer = new byte[file.Length];
                file.Read(buffer, 0, buffer.Length);
            }
            int offset = 0;
            var magicWord = ReadUint32(buffer, ref offset);
            var symbolCount = ReadUint32(buffer, ref offset);
            var sectionCount = ReadUint32(buffer, ref offset);
            for (uint i = 0; i < symbolCount; i++)
                Symbols.Add(new Symbol(buffer, ref offset));
            for (uint i = 0; i < sectionCount; i++)
                Sections.Add(new Section(buffer, ref offset));
            Debug.Assert(offset == buffer.Length);
        }

        public enum SymbolType
        {
            Local = 0,
            Import = 1,
            Export = 2,
        }

        public class Symbol
        {
            public readonly string Name;
            public readonly SymbolType Type;
            public readonly string Filename;
            public readonly uint LineNumber;
            public readonly uint SectionId;
            public readonly uint SectionOffset;

            public Symbol(byte[] buffer, ref int offset)
            {
                Name = ReadString(buffer, ref offset);
                Type = (SymbolType) ReadByte(buffer, ref offset);
                if (Type != SymbolType.Import)
                {
                    Filename = ReadString(buffer, ref offset);
                    LineNumber = ReadUint32(buffer, ref offset);
                    SectionId = ReadUint32(buffer, ref offset);
                    SectionOffset = ReadUint32(buffer, ref offset);
                }
            }
        }

        public enum SectionType
        {
            WRam0 = 0,
            Vram = 1,
            RomX = 2,
            Rom0 = 3,
            HRam = 4,
            WRamX = 5,
            SRam = 6,
            Oam = 7,
        }

        public enum SectionPatchType
        {
            BytePatch = 0,
            WordPatch = 1,
            DWordPath = 2,
        }

        public class Section
        {
            public readonly string Name;
            public readonly SectionType Type;
            public readonly uint Org;
            public readonly uint Bank;
            public readonly uint Align;
            public readonly byte[] Data;
            public readonly List<SectionPatch> Patches = new List<SectionPatch>();

            public Section(byte[] buffer, ref int offset)
            {
                var firstOffset = offset;
                Name = ReadString(buffer, ref offset);
                var length = ReadUint32(buffer, ref offset);
                Type = (SectionType) ReadByte(buffer, ref offset);
                Org = ReadUint32(buffer, ref offset);
                Bank = ReadUint32(buffer, ref offset);
                Align = ReadUint32(buffer, ref offset);
                if (Type == SectionType.Rom0 || Type == SectionType.RomX)
                {
                    Data = ReadBuffer(buffer, ref offset, length);
                    var patchCount = ReadUint32(buffer, ref offset);
                    for (uint i = 0; i < patchCount; i++)
                        Patches.Add(new SectionPatch(buffer, ref offset));
                }
            }
        }

        public class SectionPatch
        {
            public readonly string SourceFile;
            public readonly uint Line;
            public readonly uint Offset;
            public readonly SectionPatchType Type;
            public readonly byte[] RpnDta;

            public SectionPatch(byte[] buffer, ref int offset)
            {
                SourceFile = ReadString(buffer, ref offset);
                Line = ReadUint32(buffer, ref offset);
                Offset = ReadUint32(buffer, ref offset);
                Type = (SectionPatchType) ReadByte(buffer, ref offset);
                var length = ReadUint32(buffer, ref offset);
                RpnDta = ReadBuffer(buffer, ref offset, length);
            }
        }
    }
}
