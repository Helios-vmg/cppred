#include "ScriptEntryPoint.h"
#include "Renderer.h"

static const char copyright_notice[] = {
	0, 1, 2, 1, 3, 1, 4, -1, 5, 6, 7, 8, 9, 10, -2,
	0, 1, 2, 1, 3, 1, 4, -1, 11, 12, 13, 14, 15, 16, 17, 18, -2,
	0, 1, 2, 1, 3, 1, 4, -1
};

void display_copyright(Engine &engine){
#if 0
	auto copyright = NintendoCopyrightLogoGraphics;
	auto &bg = engine.get_renderer().get_tilemap(TileRegion::Background);
	
	const int x0 = 2;
	const int y0 = 7;
	int x = x0;
	int y = y0;
	for (auto offset : copyright_notice){
		if (offset == -2){
			x = x0;
			y += 2;
			continue;
		}
		auto tile = copyright.first_tile + offset;
		if (offset == -1)
			tile = 0;
		bg.tiles[x++ + y * Tilemap::w].tile_no = tile;
	}

	auto gf = GamefreakLogoGraphics;

	for (int i = 0; i < gf.width; i++)
		bg.tiles[x++ + y * Tilemap::w].tile_no = gf.first_tile + i;
#else
	engine.get_renderer().draw_image_to_tilemap(2, 7, CopyrightScreen);
#endif
	engine.wait(3);
}

void script_entry_point(Engine &engine){
	display_copyright(engine);
}
