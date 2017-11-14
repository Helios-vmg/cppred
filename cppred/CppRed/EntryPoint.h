#pragma once

enum class PokemonVersion;
class Engine;
namespace CppRed{
class AudioProgram;
namespace Scripts{

void entry_point(Engine &, PokemonVersion, CppRed::AudioProgram &);

}
}
