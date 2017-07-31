using System;
using System.Data.Common;
using System.Linq;
using System.Runtime.Remoting.Messaging;

namespace process_text
{
    enum TextCommandType
    {
        None,
        CommandText,
        CommandLine,
        CommandCont,
        CommandDone,
        CommandMem,
        CommandPrompt,
        CommandParagraph,
        CommandNum,
        CommandDb,
        CommandBcd,
        CommandNext,
        CommandPage,
        CommandDex,
    }

    abstract class TextCommand
    {
        public abstract string CommandName { get; }
        public virtual string GetCode()
        {
            return $"{CommandName}";
        }

        public abstract TextCommand Clone();
        public abstract TextCommandType Type { get; }
    }

    abstract class UnaryTextCommand : TextCommand
    {
        private string _operand;

        public string Operand
        {
            get
            {
                return _operand;
            }
            set
            {
                _operand = value
                    .Replace("¥", "<CURRENCY>")
                    .Replace("#", "<POKE>");
                
            }
        }

        public UnaryTextCommand(string op)
        {
            Operand = op;
        }

        public override string GetCode()
        {
            if (Operand.Any(x => x >= 0x80))
                Console.WriteLine($"Non-ASCII character in \"{Operand}\"");
            return $"{CommandName} \"{Operand}\"";
        }
    }

    class CommandText : UnaryTextCommand
    {
        public CommandText(string op) : base(op)
        {
        }

        public override string CommandName => "TEXT";

        public override TextCommand Clone()
        {
            return new CommandText(Operand);
        }

        public override TextCommandType Type => TextCommandType.CommandText;
    }

    class CommandLine : UnaryTextCommand
    {
        public CommandLine(string op) : base(op)
        {
        }

        public override string CommandName => "LINE";

        public override TextCommand Clone()
        {
            return new CommandLine(Operand);
        }

        public override TextCommandType Type => TextCommandType.CommandLine;
    }

    class CommandCont : UnaryTextCommand
    {
        public CommandCont(string op) : base(op)
        {
        }

        public override string CommandName => "CONT";

        public override TextCommand Clone()
        {
            return new CommandCont(Operand);
        }

        public override TextCommandType Type => TextCommandType.CommandCont;
    }

    class CommandDone : TextCommand
    {
        public override string CommandName => "DONE";

        public override TextCommand Clone()
        {
            return new CommandDone();
        }

        public override TextCommandType Type => TextCommandType.CommandDone;
    }

    class CommandMem : UnaryTextCommand
    {
        public CommandMem(string op) : base(op)
        {
        }

        public override string CommandName => "MEM";

        public override string GetCode()
        {
            return $@"{CommandName} {Operand}";
        }

        public override TextCommand Clone()
        {
            return new CommandMem(Operand);
        }

        public override TextCommandType Type => TextCommandType.CommandMem;
    }

    class CommandPrompt : TextCommand
    {
        public override string CommandName => "PROMPT";

        public override TextCommand Clone()
        {
            return new CommandPrompt();
        }

        public override TextCommandType Type => TextCommandType.CommandPrompt;
    }

    class CommandParagraph : UnaryTextCommand
    {
        public CommandParagraph(string op) : base(op)
        {
        }

        public override string CommandName => "PARA";

        public override TextCommand Clone()
        {
            return new CommandParagraph(Operand);
        }

        public override TextCommandType Type => TextCommandType.CommandParagraph;
    }

    class CommandNum : TextCommand
    {
        public string Location;
        public int BytesToRead;
        public int DigitsToDisplay;

        public CommandNum(string l, int b, int d)
        {
            Location = l;
            BytesToRead = b;
            DigitsToDisplay = d;
        }

        public override string GetCode()
        {
            return $@"{CommandName} {Location} {BytesToRead} {DigitsToDisplay}";
        }

        public override string CommandName => "NUM";

        public override TextCommand Clone()
        {
            return new CommandNum(Location, BytesToRead, DigitsToDisplay);
        }

        public override TextCommandType Type => TextCommandType.CommandNum;
    }
    
    class CommandDb : UnaryTextCommand
    {
        public CommandDb(string s) : base(s)
        {
        }
        
        public override string CommandName => "DB";

        public override TextCommand Clone()
        {
            return new CommandDb(Operand);
        }

        public override TextCommandType Type => TextCommandType.CommandDb;
    }

    class CommandBcd : TextCommand
    {
        public string Location;
        public int Option;

        private CommandBcd()
        {
        }

        public static CommandBcd CreateLeadingZeroesLeftAligned(string location, int option)
        {
            return new CommandBcd
            {
                Location = location,
                Option = option | (1 << 7) | (1 << 6),
            };
        }

        public static CommandBcd CreateRaw(string location, int option)
        {
            return new CommandBcd
            {
                Location = location,
                Option = option,
            };
        }

        public override string GetCode()
        {
            return $"{CommandName} {Location} {Option}";
        }

        public override string CommandName => "BCD";

        public override TextCommand Clone()
        {
            return new CommandBcd
            {
                Location = Location,
                Option = Option,
            };
        }

        public override TextCommandType Type => TextCommandType.CommandBcd;
    }

    class CommandNext : UnaryTextCommand
    {
        public CommandNext(string s) : base(s)
        {
        }

        public override string CommandName => "NEXT";

        public override TextCommand Clone()
        {
            return new CommandNext(Operand);
        }

        public override TextCommandType Type => TextCommandType.CommandNext;
    }

    class CommandPage : UnaryTextCommand
    {
        public CommandPage(string s) : base(s)
        {
        }

        public override string CommandName => "PAGE";

        public override TextCommand Clone()
        {
            return new CommandPage(Operand);
        }

        public override TextCommandType Type => TextCommandType.CommandPage;
    }

    class CommandDex : TextCommand
    {
        public override string CommandName => $"DEX";

        public override TextCommand Clone()
        {
            return new CommandDex();
        }

        public override TextCommandType Type => TextCommandType.CommandDex;
    }

}
