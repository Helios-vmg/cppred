using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ParseRgbdsObj;

namespace process_dex_data
{
    class Program
    {
        private static string[] Symbols =
        {
            "BulbasaurDexEntry",
            "IvysaurDexEntry",
            "VenusaurDexEntry",
            "CharmanderDexEntry",
            "CharmeleonDexEntry",
            "CharizardDexEntry",
            "SquirtleDexEntry",
            "WartortleDexEntry",
            "BlastoiseDexEntry",
            "CaterpieDexEntry",
            "MetapodDexEntry",
            "ButterfreeDexEntry",
            "WeedleDexEntry",
            "KakunaDexEntry",
            "BeedrillDexEntry",
            "PidgeyDexEntry",
            "PidgeottoDexEntry",
            "PidgeotDexEntry",
            "RattataDexEntry",
            "RaticateDexEntry",
            "SpearowDexEntry",
            "FearowDexEntry",
            "EkansDexEntry",
            "ArbokDexEntry",
            "PikachuDexEntry",
            "RaichuDexEntry",
            "SandshrewDexEntry",
            "SandslashDexEntry",
            "NidoranFDexEntry",
            "NidorinaDexEntry",
            "NidoqueenDexEntry",
            "NidoranMDexEntry",
            "NidorinoDexEntry",
            "NidokingDexEntry",
            "ClefairyDexEntry",
            "ClefableDexEntry",
            "VulpixDexEntry",
            "NinetalesDexEntry",
            "JigglypuffDexEntry",
            "WigglytuffDexEntry",
            "ZubatDexEntry",
            "GolbatDexEntry",
            "OddishDexEntry",
            "GloomDexEntry",
            "VileplumeDexEntry",
            "ParasDexEntry",
            "ParasectDexEntry",
            "VenonatDexEntry",
            "VenomothDexEntry",
            "DiglettDexEntry",
            "DugtrioDexEntry",
            "MeowthDexEntry",
            "PersianDexEntry",
            "PsyduckDexEntry",
            "GolduckDexEntry",
            "MankeyDexEntry",
            "PrimeapeDexEntry",
            "GrowlitheDexEntry",
            "ArcanineDexEntry",
            "PoliwagDexEntry",
            "PoliwhirlDexEntry",
            "PoliwrathDexEntry",
            "AbraDexEntry",
            "KadabraDexEntry",
            "AlakazamDexEntry",
            "MachopDexEntry",
            "MachokeDexEntry",
            "MachampDexEntry",
            "BellsproutDexEntry",
            "WeepinbellDexEntry",
            "VictreebelDexEntry",
            "TentacoolDexEntry",
            "TentacruelDexEntry",
            "GeodudeDexEntry",
            "GravelerDexEntry",
            "GolemDexEntry",
            "PonytaDexEntry",
            "RapidashDexEntry",
            "SlowpokeDexEntry",
            "SlowbroDexEntry",
            "MagnemiteDexEntry",
            "MagnetonDexEntry",
            "FarfetchdDexEntry",
            "DoduoDexEntry",
            "DodrioDexEntry",
            "SeelDexEntry",
            "DewgongDexEntry",
            "GrimerDexEntry",
            "MukDexEntry",
            "ShellderDexEntry",
            "CloysterDexEntry",
            "GastlyDexEntry",
            "HaunterDexEntry",
            "GengarDexEntry",
            "OnixDexEntry",
            "DrowzeeDexEntry",
            "HypnoDexEntry",
            "KrabbyDexEntry",
            "KinglerDexEntry",
            "VoltorbDexEntry",
            "ElectrodeDexEntry",
            "ExeggcuteDexEntry",
            "ExeggutorDexEntry",
            "CuboneDexEntry",
            "MarowakDexEntry",
            "HitmonleeDexEntry",
            "HitmonchanDexEntry",
            "LickitungDexEntry",
            "KoffingDexEntry",
            "WeezingDexEntry",
            "RhyhornDexEntry",
            "RhydonDexEntry",
            "ChanseyDexEntry",
            "TangelaDexEntry",
            "KangaskhanDexEntry",
            "HorseaDexEntry",
            "SeadraDexEntry",
            "GoldeenDexEntry",
            "SeakingDexEntry",
            "StaryuDexEntry",
            "StarmieDexEntry",
            "MrMimeDexEntry",
            "ScytherDexEntry",
            "JynxDexEntry",
            "ElectabuzzDexEntry",
            "MagmarDexEntry",
            "PinsirDexEntry",
            "TaurosDexEntry",
            "MagikarpDexEntry",
            "GyaradosDexEntry",
            "LaprasDexEntry",
            "DittoDexEntry",
            "EeveeDexEntry",
            "VaporeonDexEntry",
            "JolteonDexEntry",
            "FlareonDexEntry",
            "PorygonDexEntry",
            "OmanyteDexEntry",
            "OmastarDexEntry",
            "KabutoDexEntry",
            "KabutopsDexEntry",
            "AerodactylDexEntry",
            "SnorlaxDexEntry",
            "ArticunoDexEntry",
            "ZapdosDexEntry",
            "MoltresDexEntry",
            "DratiniDexEntry",
            "DragonairDexEntry",
            "DragoniteDexEntry",
            "MewtwoDexEntry",
            "MewDexEntry",
        };

        private static Dictionary<byte, char> Charmap = new Dictionary<byte, char>
        {
            {127, (char)32},
            {80, (char)64},
            {128, (char)65},
            {129, (char)66},
            {130, (char)67},
            {131, (char)68},
            {132, (char)69},
            {133, (char)70},
            {134, (char)71},
            {135, (char)72},
            {136, (char)73},
            {137, (char)74},
            {138, (char)75},
            {139, (char)76},
            {140, (char)77},
            {141, (char)78},
            {142, (char)79},
            {143, (char)80},
            {144, (char)81},
            {145, (char)82},
            {146, (char)83},
            {147, (char)84},
            {148, (char)85},
            {149, (char)86},
            {150, (char)87},
            {151, (char)88},
            {152, (char)89},
            {153, (char)90},
        };

        static void Main(string[] args)
        {
            var parser = new ObjParser("main_red.o");

            foreach (var symbol in Symbols)
            {
                var data = parser.CopySymbolData(symbol, 128);
                var description = new StringBuilder();
                int offset = 0;
                while (true)
                {
                    char c = Charmap[data[offset++]];
                    if (c == '@')
                        break;
                    description.Append(c);
                }
                var feet = (int)data[offset++];
                var inches = (int)data[offset++];
                var tenthsOfPounds = (int) data[offset] | ((int) data[offset + 1] << 8);
                Console.WriteLine($"{description},{feet},{inches},{tenthsOfPounds / 10}.{tenthsOfPounds % 10}");
            }
        }
    }
}
