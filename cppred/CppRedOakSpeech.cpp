#include "CppRedOakSpeech.h"
#include "CppRedEngine.h"

static void fade_in(CppRedEngine &cppred){
	const Palette palettes[] = {
		BITMAP(01010100),
		BITMAP(10101000),
		BITMAP(11111100),
		BITMAP(11111000),
		BITMAP(11110100),
		BITMAP(11100100),
	};

	auto &engine = cppred.get_engine();
	auto &renderer = engine.get_renderer();

	for (auto &p : palettes){
		renderer.set_palette(PaletteRegion::Background, p);
		engine.wait_frames(10);
	}
}

namespace CppRedScripts{

NamesChosenDuringOakSpeech oak_speech(CppRedEngine &cppred){
	auto &engine = cppred.get_engine();
	auto &renderer = engine.get_renderer();

	NamesChosenDuringOakSpeech ret;

	cppred.play_sound(SoundId::Stop);
	cppred.play_sound(SoundId::Music_Routes2);
	renderer.clear_screen();
	engine.wait(1);
	renderer.draw_image_to_tilemap({ 6, 4 }, ProfOakPic);
	fade_in(cppred);

	cppred.run_dialog(TextResourceId::OakSpeechText1);
	engine.wait(3600);

	return ret;
}

}
