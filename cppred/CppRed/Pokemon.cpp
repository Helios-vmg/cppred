#include "stdafx.h"
#include "Pokemon.h"
#include "Game.h"
#include "Trainer.h"
#include "../Renderer.h"
#ifndef HAVE_PCH
#include <sstream>
#include <cmath>
#endif

namespace CppRed{

bool Party::add_pokemon(SpeciesId species, int level, Trainer &ot, XorShift128 &rng){
	if (this->members.size() >= this->max_party_size)
		return false;
	this->members.emplace_back(species, level, ot, rng);
	return true;
}

bool Party::add_pokemon(const Pokemon &pokemon){
	if (this->members.size() >= this->max_party_size)
		return false;
	this->members.push_back(pokemon);
	return true;
}

Pokemon &Party::get_last_added_pokemon(){
	if (!this->members.size())
		throw std::runtime_error("Internal error: Party::get_last_added_pokemon() called when no pokemon had been added.");
	return this->members.back();
}

void Party::heal(){
	for (auto &member : this->members)
		member.heal();
}

int Pokemon::get_stat(PokemonStats::StatId which, bool ignore_xp){
	auto &stat = this->computed_stats.get_stat(which);
	int ret;
	if (stat < 0){
		auto &pokemon = *pokemon_by_species_id[(int)this->species];
		int base_stat = pokemon.base_stats.get_stat(which);
		int bonus = 0;
		if (!ignore_xp){
			int stat_xp = this->stat_experience.get_stat(which);
			bonus = (int)ceil(sqrt(stat_xp));
		}
		int iv = this->get_iv(which);
		ret = ((base_stat + iv) * 2 + bonus / 4) * this->level / 100;
		if (which == PokemonStats::StatId::Hp)
			ret += this->level + 10;
		else
			ret += 5;
		if (!ignore_xp)
			stat = ret;
	}else
		ret = stat;
	return ret;
}

/*
 *
 * Explanation:
 *
 * individual value (bits): (most significant) FEDC BA98 7654 3210 (least significant)
 * attack  individual value = 7654
 * defense individual value = 3210
 * speed   individual value = FEDC
 * special individual value = BA98
 * HP      individual value = 40C8
 *
 * Example:
 *
 * IV = 48161 (0xBC21, 1011 1100 0010 0001)
 * attack  IV = 2  (0010)
 * defense IV = 1  (0001)
 * speed   IV = 12 (1100)
 * special IV = 11 (1011)
 * HP      IV = 6  (0110)
 *
 */
int Pokemon::get_iv(PokemonStats::StatId which) const{
	if (which == PokemonStats::StatId::Hp){
		int ret = 0;
		for (int i = 0; i < 4; i++){
			int index = (i ^ 1) * 4;
			int val = (this->individual_values >> index) & 0x01;
			ret <<= 1;
			ret |= val;
		}
		return ret;
	}
	int index = (int)which - 1;
	index ^= 1;
	return (this->individual_values >> index) & 0x0F;
}


Pokemon::Pokemon(SpeciesId species, int level, const Trainer &ot, XorShift128 &rng, const PokemonStats &input_stats):
		computed_stats(-1, -1, -1, -1, -1){
	this->species = species;
	this->level = level;
	auto &pokemon = *pokemon_by_species_id[(int)species];
	fill(this->moves, MoveId::None);
	int i = 0;
	for (auto move : pokemon.get_initial_moves()){
		this->moves[i] = move;
		auto move_data = pokemon_moves_by_id[(int)move];
		this->pp[i] = move_data ? move_data->pp : 0;
		i++;
	}
	this->original_trainer_id = ot.get_trainer_id();
	this->original_trainer_name = ot.get_name();
	this->status = StatusCondition::Normal;
	rng.generate(this->individual_values);
	this->current_hp = this->get_max_hp();
	this->experience = this->calculate_min_xp_to_reach_level(this->species, this->level);
	if (!input_stats.null())
		this->computed_stats = input_stats;
	else
		this->computed_stats.set_all(-1);
}

struct GrowthPolynomial{
	int a;
	int b;
	int c;
	int d;
	int e;
	int compute(int x) const{
		return x * x * x * a / b + x * x * c + x * d + e;
	}
};

static const GrowthPolynomial polynomials[] = {
	{ 1, 1,   0,   0,    0 },
	{ 3, 4,  10,   0,  -30 },
	{ 3, 4,  20,   0,  -70 },
	{ 6, 5, -15, 100, -140 },
	{ 4, 5,   0,   0,    0 },
	{ 5, 4,   0,   0,    0 },
};

static void check_rate(const BasePokemonInfo &data){
	auto growth_rate = data.growth_rate;
	if (growth_rate < 0 || growth_rate >= array_length(polynomials)){
		std::stringstream stream;
		stream << "Internal error: species " << data.internal_name << " has invalid growth rate of " << (int)growth_rate;
		throw std::runtime_error(stream.str());
	}
}

int Pokemon::calculate_min_xp_to_reach_level(SpeciesId species, int level){
	auto &data = *pokemon_by_species_id[(int)species];
	auto growth_rate = data.growth_rate;
	check_rate(data);
	return polynomials[growth_rate].compute(level);
}

int Pokemon::calculate_level_at_xp(SpeciesId species, int xp){
	auto &data = *pokemon_by_species_id[(int)species];
	auto growth_rate = data.growth_rate;
	check_rate(data);
	auto &poly = polynomials[growth_rate];
	int lo = 1,
		hi = 100;
	if (xp <= poly.compute(lo))
		return lo;
	if (xp >= poly.compute(hi))
		return hi;
	hi++;
	int d = hi - lo;
	while (d > 1){
		auto pivot = lo + d / 2;
		auto y = poly.compute(pivot);
		if (xp > y)
			lo = pivot;
		else if (xp < y)
			hi = pivot;
		else
			return pivot;
		d = hi - lo;
	}
	return lo;
}

void Pokemon::heal(){
	this->current_hp = this->get_max_hp();
	this->status = StatusCondition::Normal;
}

const BasePokemonInfo &Pokemon::get_data() const{
	return *pokemon_by_species_id[(int)this->species];
}

StatusCondition2 Pokemon::get_status() const{
	return !this->current_hp ? StatusCondition2::Fainted : (StatusCondition2)(int)this->status;
}

const char *Pokemon::get_display_name() const{
	if (this->nickname.size())
		return this->nickname.c_str();
	return this->get_data().display_name;
}

Pokemon *Party::get_first_usable_pokemon(){
	for (auto &p : this->members)
		if (p.get_status() != StatusCondition2::Fainted)
			return &p;
	return nullptr;
}

Pokemon &Party::get(size_t i){
	if (i >= this->members.size())
		throw std::runtime_error("Party::get(): Bad member index.");
	return this->members[i];
}

const Pokemon &Party::get(size_t i) const{
	if (i >= this->members.size())
		throw std::runtime_error("Party::get(): Bad member index.");
	return this->members[i];
}

StatsScreenResult Pokemon::display_stats_screen(Game &game){
	auto &renderer = game.get_engine().get_renderer();
	auto &audio = game.get_audio_interface();
	AutoRendererPusher pusher(renderer);
	renderer.clear_screen();
	renderer.clear_sprites();
	renderer.set_enable_window(false);

	this->render_page2(renderer);
	auto &tilemap = renderer.get_tilemap(TileRegion::Background);
	auto tilemap_copy = tilemap;
	this->render_page1(renderer);

	audio.play_cry(this->species);
	audio.wait_for_sfx_to_end();

	StatsScreenResult ret = StatsScreenResult::Close;
	int current_page = 0;

	auto &coroutine = Coroutine::get_current_coroutine();
	game.reset_joypad_state();
	while (true){
		coroutine.yield();
		auto input = game.joypad_auto_repeat();
		if (input.get_b())
			break;
		if (input.get_a() || input.get_right() && current_page == 0){
			if (current_page == 1)
				break;
			current_page = 1;
			tilemap.swap(tilemap_copy);
			continue;
		}
		if (input.get_left() && current_page == 1){
			current_page = 0;
			tilemap.swap(tilemap_copy);
			continue;
		}
		if (input.get_down()){
			ret = StatsScreenResult::GoToNext;
			break;
		}
		if (input.get_up()){
			ret = StatsScreenResult::GoToPrevious;
			break;
		}
	}
	game.reset_joypad_state();
	return ret;
}

void Pokemon::render_common_data(Renderer &renderer, const GraphicsAsset &layout){
	auto &data = this->get_data();

	//Prepare layout.
	renderer.draw_image_to_tilemap({0, 0}, layout);
	
	//Display image with Pokedex ID.
	renderer.draw_image_to_tilemap_flipped({1, 0}, *data.front);
	renderer.put_string({3, 7}, TileRegion::Background, number_to_decimal_string((int)data.pokedex_id, 3, '0').data());
	
	//Display name.
	renderer.put_string({9, 1}, TileRegion::Background, this->get_display_name(), max_pokemon_name_size);
}

void Pokemon::render_page1(Renderer &renderer){
	auto &data = this->get_data();

	this->render_common_data(renderer, StatsPage1);

	//Display status.
	renderer.put_string({15, 2}, TileRegion::Background, number_to_decimal_string(this->level).data());
	auto hp = this->get_current_hp();
	auto max_hp = this->get_max_hp();
	renderer.draw_bar({13, 3}, TileRegion::Background, 6, max_hp, hp);
	renderer.put_string({12, 4}, TileRegion::Background, number_to_decimal_string(hp, 3).data());
	renderer.put_string({16, 4}, TileRegion::Background, number_to_decimal_string(max_hp, 3).data());
	renderer.put_string({16, 6}, TileRegion::Background, to_string(this->get_status()), 3);

	//Display stats.
	renderer.put_string({6, 10}, TileRegion::Background, number_to_decimal_string(this->get_stat(PokemonStats::StatId::Attack), 3, ' ').data());
	renderer.put_string({6, 12}, TileRegion::Background, number_to_decimal_string(this->get_stat(PokemonStats::StatId::Defense), 3, ' ').data());
	renderer.put_string({6, 14}, TileRegion::Background, number_to_decimal_string(this->get_stat(PokemonStats::StatId::Speed), 3, ' ').data());
	renderer.put_string({6, 16}, TileRegion::Background, number_to_decimal_string(this->get_stat(PokemonStats::StatId::Special), 3, ' ').data());

	//Display misc. info.
	renderer.put_string({11, 10}, TileRegion::Background, pokemon_type_strings[(int)data.type[0]], 8);
	if (data.type[1] == data.type[0])
		renderer.fill_rectangle(TileRegion::Background, {10, 11}, {9, 1}, Tile());
	else
		renderer.put_string({11, 12}, TileRegion::Background, pokemon_type_strings[(int)data.type[1]], 8);
	renderer.put_string({12, 14}, TileRegion::Background, number_to_decimal_string(this->original_trainer_id, 5, '0').data());
	renderer.put_string({12, 16}, TileRegion::Background, this->original_trainer_name.c_str(), max_character_name_size);
}

void Pokemon::render_page2(Renderer &renderer){
	auto &data = this->get_data();

	this->render_common_data(renderer, StatsPage2);

	//Display experience.
	renderer.put_string({9, 4}, TileRegion::Background, number_to_decimal_string(this->experience, 10, ' ').data());
	if (this->level < 100){
		renderer.put_string({8, 6}, TileRegion::Background, number_to_decimal_string(this->calculate_min_xp_to_reach_level(this->level + 1) - this->experience, 6, ' ').data());
		if (this->level < 99)
			renderer.put_string({17, 6}, TileRegion::Background, number_to_decimal_string(this->level + 1).data(), 2);
		else
			renderer.put_string({16, 6}, TileRegion::Background, "100");
	}else
		renderer.fill_rectangle(TileRegion::Background, {9, 6}, {10, 2}, Tile());

	//Display moves.
	for (int i = 0; i < max_moves; i++){
		Point first_line(2, 9 + i * 2);
		Point second_line1(11, 10 + i * 2);
		if (this->moves[i] == MoveId::None){
			renderer.put_string(first_line, TileRegion::Background, "-");
			renderer.put_string(second_line1, TileRegion::Background, "--", 8);
		}else{
			Point second_line2 = second_line1 + Point(3, 0);
			auto &move_data = *pokemon_moves_by_id[(int)this->moves[i]];
			renderer.put_string(first_line, TileRegion::Background, move_data.display_name, 13 - first_line.x);
			renderer.put_string(second_line2, TileRegion::Background, number_to_decimal_string(this->pp[i], 2).data());
			renderer.put_string(second_line2 + Point(3, 0), TileRegion::Background, number_to_decimal_string(move_data.pp, 2).data());
		}
	}
}

}
