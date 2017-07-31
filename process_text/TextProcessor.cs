using System;
using System.CodeDom;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;

namespace process_text
{
    class TextProcessor
    {
        private static Regex WhitespaceRegex = new Regex(@"^\s*$");
        private static Regex IncludeRegex = new Regex(@"^\s*INCLUDE\s+""(.*)""$");
        private static Regex ConstantRegex = new Regex(@"^\s*[A-Za-z_][A-Za-z_0-9]*\s+EQU\s+.*$");
        private static Regex SectionRegex = new Regex(@"^\s*SECTION\s+.*");
        private static Regex LabelRegex = new Regex(@"^\s*([A-Za-z_][A-Za-z_0-9]*)\s*\:\:\s*");
        private static Regex TextElementRegex = new Regex(@"^\s*text\s*\""(.*)\""\s*$");
        private static Regex Text2ElementRegex = new Regex(@"^\s*text\s*\$([0-9a-fA-F]+)\s*\,\s*\""(.*)\""\s*$");
        private static Regex Text3ElementRegex = new Regex(@"^\s*text\s*\""(.*)\""\s*\,\s*\$([0-9a-fA-F]+)\s*$");
        private static Regex LineElementRegex = new Regex(@"^\s*line\s*\""(.*)\""\s*$");
        private static Regex Line2ElementRegex = new Regex(@"^\s*line\s*\$([0-9a-fA-F]+)\s*\,\s*\""(.*)\""\s*$");
        private static Regex ContElementRegex = new Regex(@"^\s*cont\s*\""(.*)\""\s*$");
        private static Regex Cont2ElementRegex = new Regex(@"^\s*cont\s*\$([0-9a-fA-F]+)\s*\,\s*\""(.*)\""\s*$");
        private static Regex DoneElementRegex = new Regex(@"^\s*done\s*$");
        private static Regex TxRamElementRegex = new Regex(@"^\s*TX_RAM\s+(.*)\s*$");
        private static Regex PromptElementRegex = new Regex(@"^\s*prompt\s*$");
        private static Regex ParaElementRegex = new Regex(@"^\s*para\s*\""(.*)\""\s*$");
        private static Regex Para2ElementRegex = new Regex(@"^\s*para\s*\$([0-9a-fA-F]+)\s*\,\s*\""(.*)\""\s*$");
        private static Regex NextElementRegex = new Regex(@"^\s*next\s*\""(.*)\""\s*$");
        private static Regex TxNumElementRegex = new Regex(@"^\s*TX_NUM\s+([A-Za-z_][A-Za-z_0-9]*)\s*\,\s*([0-9]+)\s*\,\s*([0-9]+)\s*$");
        private static Regex TxNum2ElementRegex = new Regex(@"^\s*TX_NUM\s+([A-Za-z_][A-Za-z_0-9]*)\s*\,\s*\$([0-9]+)\s*\,\s*\$([0-9]+)\s*$");
        private static Regex DbRegex = new Regex(@"^\s*db\s+\$([0-9a-fA-F]+)\s*$");
        private static Regex Db2Regex = new Regex(@"^\s*db\s+\""(.+)\""\s*$");
        private static Regex TxBcdRegex = new Regex(@"^\s*TX_BCD\s+([A-Za-z_][A-Za-z_0-9]*)\s*\,\s*([0-9]+)\s*\|\s*LEADING_ZEROES\s*\|\s*LEFT_ALIGN\s*$");
        private static Regex TxBcd2Regex = new Regex(@"^\s*TX_BCD\s+([A-Za-z_][A-Za-z_0-9]*)\s*\,\s*\$([0-9A-Fa-f]+)\s*$");
        private static Regex Comment = new Regex(@"^\s*;.*$");
        private static Regex PageElementRegex = new Regex(@"^\s*page\s*\""(.*)\""\s*$");
        private static Regex DexElementRegex = new Regex(@"^\s*dex\s*$");

        private Dictionary<Regex, Action<string[]>> _handlers;

        private void InitHandlers()
        {
            _handlers = new Dictionary<Regex, Action<string[]>>
            {
                {WhitespaceRegex, null},
                {IncludeRegex, HandleInclude},
                {ConstantRegex, null},
                {SectionRegex, null},
                {LabelRegex, HandleLabel},
                {TextElementRegex, HandleTextElement},
                {Text2ElementRegex, HandleText2Element},
                {Text3ElementRegex, HandleText3Element},
                {LineElementRegex, HandleLineElement},
                {Line2ElementRegex, HandleLine2Element},
                {ContElementRegex, HandleContElement},
                {Cont2ElementRegex, HandleCont2Element},
                {DoneElementRegex, HandleDoneElement},
                {TxRamElementRegex, HandleTxRam},
                {PromptElementRegex, HandlePrompt},
                {ParaElementRegex, HandlePara},
                {Para2ElementRegex, HandlePara2},
                {TxNumElementRegex, HandleTxNum},
                {TxNum2ElementRegex, HandleTxNum2},
                {DbRegex, HandleDb},
                {Db2Regex, HandleDb2},
                {TxBcdRegex, HandleTxBcd},
                {TxBcd2Regex, HandleTxBcd2},
                {NextElementRegex, HandleNextElement},
                {Comment, null},
                {PageElementRegex, HandlePageElement},
                {DexElementRegex, HandleDexElement},
            };
        }

        public TextProcessor()
        {
            InitHandlers();
        }

        private readonly Stack<string> _filesBeingProcessed = new Stack<string>();
        private string _currentLabel;
        private Dictionary<string, List<TextCommand>> _texts = new Dictionary<string, List<TextCommand>>();

        public void ProcessText(string path)
        {
            _filesBeingProcessed.Push(path);
            try
            {
                using (var file = new StreamReader(path, Encoding.UTF8))
                {
                    int lineNo = 0;
                    while (true)
                    {
                        lineNo++;
                        var line = file.ReadLine();
                        if (line == null)
                            break;
                        bool found = false;
                        foreach (var kv in _handlers)
                        {
                            var match = kv.Key.Match(line);
                            if (match.Success)
                            {
                                try
                                {
                                    kv.Value?.Invoke(match.ToStrings());
                                }
                                catch (Exception e)
                                {
                                    throw new Exception($"Error interpreting line {lineNo} of {path}: {line}\n{e.Message}");
                                }
                                found = true;
                                break;
                            }
                        }
                        if (!found)
                            throw new Exception($"Don't know how to parse line {lineNo} of {path}: {line}");
                    }
                }
            }
            finally
            {
                _filesBeingProcessed.Pop();
            }
        }

        private static HashSet<string> IgnoredFiles = new HashSet<string>
        {
            "charmap.asm",
            "constants/text_constants.asm",
            "macros.asm",
            "hram.asm",
        };

        private void HandleInclude(string[] groups)
        {
            if (IgnoredFiles.Contains(groups[0]))
                return;
            var dir = _filesBeingProcessed.Peek();
            dir = Path.GetDirectoryName(dir) + "\\";
            ProcessText(dir + groups[0].Replace("/", "\\"));
        }

        private void HandleLabel(string[] groups)
        {
            _currentLabel = groups[0];
        }

        private List<TextCommand> GetList()
        {
            List<TextCommand> ret;
            if (!_texts.TryGetValue(_currentLabel, out ret))
                _texts[_currentLabel] = ret = new List<TextCommand>();
            return ret;
        } 

        private void HandleTextElement(string[] groups)
        {
            var list = GetList();
            list.Add(new CommandText(groups[0]));
        }

        private static string ParseHexString(string s)
        {
            var val = Convert.ToInt32(s, 16);
            switch (val)
            {
                case 0x4B:
                case 0x4C:
                case 0x51:
                case 0x6D:
                    return $"<${val:X2}>";
                default:
                    throw new Exception($"Can't handle byte 0x{val:X2}");
            }
        }

        private static string ParseGroups2(string[] groups)
        {
            return ParseHexString(groups[0]) + groups[1];
        }

        private static string ParseGroups3(string[] groups)
        {
            return groups[0] + ParseHexString(groups[1]);
        }

        private void HandleText2Element(string[] groups)
        {
            var list = GetList();
            list.Add(new CommandText(ParseGroups2(groups)));
        }

        private void HandleText3Element(string[] groups)
        {
            var list = GetList();
            list.Add(new CommandText(ParseGroups3(groups)));
        }

        private void HandleLineElement(string[] groups)
        {
            var list = GetList();
            list.Add(new CommandLine(groups[0]));
        }

        private void HandleLine2Element(string[] groups)
        {
            var list = GetList();
            list.Add(new CommandLine(ParseGroups2(groups)));
        }

        private void HandleContElement(string[] groups)
        {
            var list = GetList();
            list.Add(new CommandCont(groups[0]));
        }

        private void HandleCont2Element(string[] groups)
        {
            var list = GetList();
            list.Add(new CommandCont(ParseGroups2(groups)));
        }

        private void HandleDoneElement(string[] groups)
        {
            var list = GetList();
            list.Add(new CommandDone());
        }

        private void HandleTxRam(string[] groups)
        {
            var list = GetList();
            list.Add(new CommandMem(groups[0]));
        }

        private void HandlePrompt(string[] groups)
        {
            var list = GetList();
            list.Add(new CommandPrompt());
        }

        private void HandlePara(string[] groups)
        {
            var list = GetList();
            list.Add(new CommandParagraph(groups[0]));
        }

        private void HandlePara2(string[] groups)
        {
            var list = GetList();
            list.Add(new CommandParagraph(ParseGroups2(groups)));
        }

        private void HandleTxNum(string[] groups)
        {
            var list = GetList();
            list.Add(new CommandNum(groups[0], Convert.ToInt32(groups[1]), Convert.ToInt32(groups[2])));
        }

        private void HandleTxNum2(string[] groups)
        {
            var list = GetList();
            list.Add(new CommandNum(groups[0], Convert.ToInt32(groups[1], 16), Convert.ToInt32(groups[2], 16)));
        }

        private void HandleDb(string[] groups)
        {
            var list = GetList();
            var val = Convert.ToInt32(groups[0], 16);
            if (val == 0)
                list.Add(new CommandText(""));
            else
                throw new Exception();
        }

        private void HandleDb2(string[] groups)
        {
            var list = GetList();
            list.Add(new CommandDb(groups[0]));
        }

        private void HandleTxBcd(string[] groups)
        {
            var list = GetList();
            list.Add(CommandBcd.CreateLeadingZeroesLeftAligned(groups[0], Convert.ToInt32(groups[1], 16)));
        }

        private void HandleTxBcd2(string[] groups)
        {
            var list = GetList();
            list.Add(CommandBcd.CreateRaw(groups[0], Convert.ToInt32(groups[1], 16)));
        }

        private void HandleNextElement(string[] groups)
        {
            var list = GetList();
            list.Add(new CommandNext(groups[0]));
        }

        private void HandlePageElement(string[] groups)
        {
            var list = GetList();
            list.Add(new CommandPage(groups[0]));
        }

        private void HandleDexElement(string[] groups)
        {
            var list = GetList();
            list.Add(new CommandDex());
        }
        
        public void GenerateOutput(TextWriter source)
        {
            foreach (var text in _texts)
            {
                source.WriteLine($".{text.Key}");
                foreach (var cmd in text.Value)
                    source.WriteLine($"{cmd.GetCode()}");
                source.WriteLine("");
            }
        }

        public void GenerateOutput2(TextWriter source)
        {
            foreach (var text in _texts)
            {
                source.WriteLine($"SEQ({text.Key})");
                foreach (var cmd in text.Value)
                {
                    string operand = null;
                    var casted = cmd as UnaryTextCommand;
                    if (casted != null)
                        operand = casted.Operand.Replace("@", "");
                    switch (cmd.Type)
                    {
                        case TextCommandType.CommandText:
                            if (!string.IsNullOrEmpty(operand))
                                source.Write($" << \"{operand}\"");
                            break;
                        case TextCommandType.CommandLine:
                        case TextCommandType.CommandCont:
                        case TextCommandType.CommandNext:
                        case TextCommandType.CommandPage:
                        case TextCommandType.CommandParagraph:
                            source.WriteLine($" << {cmd.CommandName}");
                            if (!string.IsNullOrEmpty(operand))
                                source.Write($" << \"{operand}\"");
                            break;
                        case TextCommandType.CommandDone:
                        case TextCommandType.CommandPrompt:
                        case TextCommandType.CommandDex:
                            source.WriteLine($" << {cmd.CommandName}");
                            break;
                        case TextCommandType.CommandMem:
                            source.Write($" << MEM({operand})");
                            break;
                        case TextCommandType.CommandNum:
                            {
                                var num = (CommandNum) cmd;
                                source.Write($" << NUM({num.Location}, {num.BytesToRead}, {num.DigitsToDisplay})");
                            }
                            break;
                        case TextCommandType.CommandBcd:
                            {
                                var bcd = (CommandBcd)cmd;
                                source.Write($" << BCD({bcd.Location}, {bcd.Option})");
                            }
                            break;
                        case TextCommandType.CommandDb:
                            break;
                        default:
                            throw new ArgumentOutOfRangeException();
                    }
                }
                source.WriteLine(";");
                source.WriteLine("");
            }
        }

        public void Clean()
        {
            if (_texts.ContainsKey("MoveNames"))
                _texts.Remove("MoveNames");
        }

        private int _assertion;

        private void Assert(bool x)
        {
            _assertion++;
            if (!x)
                throw new Exception();
        }

        public void CheckAssertions()
        {
            _assertion = 0;
            try
            {
                var firstCommands = new HashSet<TextCommandType>()
                {
                    TextCommandType.CommandText,
                    TextCommandType.CommandMem,
                    TextCommandType.CommandNum,
                };
                var lastCommandsNullary = new HashSet<TextCommandType>()
                {
                    TextCommandType.CommandDone,
                    TextCommandType.CommandDex,
                    TextCommandType.CommandPrompt,
                };
                var mustBeLast = new HashSet<TextCommandType>()
                {
                    TextCommandType.CommandDb,
                };
                mustBeLast.UnionWith(lastCommandsNullary);
                var lastCommands = new HashSet<TextCommandType>()
                {
                    TextCommandType.CommandText,
                    TextCommandType.CommandLine,
                    TextCommandType.CommandCont,
                    TextCommandType.CommandParagraph,
                };
                lastCommands.UnionWith(mustBeLast);
                var specialPrints = new HashSet<TextCommandType>()
                {
                    TextCommandType.CommandMem,
                    TextCommandType.CommandNum,
                    TextCommandType.CommandBcd,
                };
                var beforeSpecialPrints = new HashSet<TextCommandType>()
                {
                    TextCommandType.CommandText,
                    TextCommandType.CommandLine,
                    TextCommandType.CommandParagraph,
                    TextCommandType.CommandCont,
                };
                var afterSpecialPrints = new HashSet<TextCommandType>()
                {
                    TextCommandType.CommandText,
                    TextCommandType.CommandDb,
                };

                //Assertion: The first command of every sequence is TEXT, MEM, or NUM.
                Assert(_texts.All(x => firstCommands.Contains(x.Value.First().Type)));

                //Assertion: The last command of every sequence is TEXT, LINE, CONT, PARA, DONE, DB, PROMPT, or DEX.
                Assert(_texts.All(x => lastCommands.Contains(x.Value.Last().Type)));

                //Assertion: The operand of the last command of every sequence that is not DONE, DEX, or PROMPT ends in "@"
                {
                    bool all = true;
                    foreach (var last in _texts.Values.Select(x => x.Last()).Where(x => !lastCommandsNullary.Contains(x.Type)))
                    {
                        var cmd = last as UnaryTextCommand;
                        all &= cmd != null && cmd.Operand.EndsWith("@");
                    }
                    Assert(all);
                }

                //Assertion: DONE, DEX, PROMPT, or DB must only be the last command of a sequence.
                {
                    bool all = true;
                    foreach (var text in _texts.Values)
                    {
                        all &= text.Take(text.Count - 1).All(x => !mustBeLast.Contains(x.Type));
                        if (!all)
                            break;
                    }
                    Assert(all);
                }

                //Assertion: After every MEM, NUM, or BCD, always follows either a TEXT or a DB.
                {
                    bool all = true;
                    foreach (var text in _texts.Values)
                    {
                        for (int i = 0; i < text.Count - 1 && all; i++)
                        {
                            var c = specialPrints.Contains(text[i].Type);
                            all &= !c || c && afterSpecialPrints.Contains(text[i + 1].Type);
                        }
                        if (!all)
                            break;
                    }
                    Assert(all);
                }

                //Assertion: Before every TEXT or a DB that's not the first one in the sequence, there is either a MEM, a NUM, or a BCD.
                {
                    bool all = true;
                    foreach (var text in _texts.Values)
                    {
                        for (int i = 1; i < text.Count && all; i++)
                        {
                            var c = afterSpecialPrints.Contains(text[i].Type);
                            all &= !c || c && specialPrints.Contains(text[i - 1].Type);
                        }
                        if (!all)
                            break;
                    }
                    Assert(all);
                }

                //Assertion: Before every MEM, NUM, or BCD that's not the first one in the sequence, there is either a TEXT, a LINE, a PARA, or a CONT.
                {
                    bool all = true;
                    foreach (var text in _texts)
                    {
                        var value = text.Value;
                        for (int i = 1; i < value.Count && all; i++)
                        {
                            var c = specialPrints.Contains(value[i].Type);
                            all &= !c || c && beforeSpecialPrints.Contains(value[i - 1].Type);
                        }
                        if (!all)
                            break;
                    }
                    Assert(all);
                }

                //Assertion: Before every MEM, NUM, or BCD that's not the first one in the sequence, the previous command ends with a "@".
                {
                    bool all = true;
                    foreach (var text in _texts)
                    {
                        var value = text.Value;
                        for (int i = 1; i < value.Count && all; i++)
                        {
                            if (!specialPrints.Contains(value[i].Type))
                                continue;

                            var previous = (UnaryTextCommand) value[i - 1];
                            all &= previous.Operand.EndsWith("@");
                        }
                        if (!all)
                            break;
                    }
                    Assert(all);
                }

                //Assertion: DONE is always the last command in a sequence.
                Assert(_texts.All(x => !x.Value.Select(y => y.Type).Contains(TextCommandType.CommandDone) || x.Value.Select(y => y.Type).ToList().IndexOf(TextCommandType.CommandDone) == x.Value.Count - 1));

                //Assertion: Every sequence uses at most one of LINE or NEXT.
                {
                    bool all = true;
                    foreach (var text in _texts.Values)
                        all &= !(text.Select(x => x.Type).Contains(TextCommandType.CommandLine) & text.Select(x => x.Type).Contains(TextCommandType.CommandNext));
                    Assert(all);
                }

                //Assertion: Every sequence uses at most one of PARA or PAGE.
                {
                    bool all = true;
                    foreach (var text in _texts.Values)
                        all &= !(text.Select(x => x.Type).Contains(TextCommandType.CommandParagraph) & text.Select(x => x.Type).Contains(TextCommandType.CommandPage));
                    Assert(all);
                }
            }
            catch
            {
                Console.WriteLine($"Assertion {_assertion} failed.");
                return;
            }
            Console.WriteLine("All assertions passed.");
        }
    }
}