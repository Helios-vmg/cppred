#pragma once

enum class PokemonVersion;
class Engine;
namespace CppRed{
class AudioProgramInterface;
namespace Scripts{

void entry_point(Engine &, PokemonVersion, CppRed::AudioProgramInterface &);

}
}
