#include "CppRed.h"

void CppRed::update_sprites(){
	if (this->wram.wUpdateSpritesEnabled != 1)
		return;

	unsigned i = 0;
	for (const auto &spr : this->wram.wSpriteStateData2){
		this->hram.H_CURRENTSPRITEOFFSET = i++ * SpriteStateData1::size;
		if (!spr.sprite_image_base_offset)
			continue;
		if (spr.sprite_image_base_offset == 1)
			this->update_player_sprite();
		else
			this->update_non_player_sprite(spr);
	}
}

void CppRed::update_player_sprite(){
	//wSpriteStateData2[0] is always the PC sprite.
	auto sprite1 = this->wram.wSpriteStateData1[0];
	auto sprite2 = this->wram.wSpriteStateData2[0];
	byte_t counter = sprite2.walk_animation_counter;
	bool disable_sprite = true;
	if (!counter){
		auto p = this->get_tilemap_location(8, 9);
		byte_t tile = *p;
		this->hram.hTilePlayerStandingOn = tile;
		disable_sprite = tile >= 0x60;
	} else{
		if (counter != 0xFF){
			counter--;
			sprite2.walk_animation_counter = counter;
		}
	}
	if (disable_sprite){
		sprite1.sprite_image_idx = 0xFF;
		return;
	}

	this->detect_sprite_collision();

	bool not_moving = false;
	if (!this->wram.wWalkCounter){
		PlayerDirectionBitmap pc_direction = this->wram.wPlayerMovingDirection;
		SpriteFacingDirection sprite_direction = SpriteFacingDirection::Down;
		if (check_flag((unsigned)pc_direction, (unsigned)PlayerDirectionBitmap::Down)){
			sprite_direction = SpriteFacingDirection::Down;
		} else if (check_flag((unsigned)pc_direction, (unsigned)PlayerDirectionBitmap::Up)){
			sprite_direction = SpriteFacingDirection::Up;
		} else if (check_flag((unsigned)pc_direction, (unsigned)PlayerDirectionBitmap::Left)){
			sprite_direction = SpriteFacingDirection::Left;
		} else if (check_flag((unsigned)pc_direction, (unsigned)PlayerDirectionBitmap::Right)){
			sprite_direction = SpriteFacingDirection::Right;
		} else
			not_moving = true;

		if (!not_moving){
			sprite1.facing_direction = sprite_direction;
			not_moving = this->wram.wFontLoaded.get_in_use();
		}
	}

	bool skip_sprite_animation = false;
	if (not_moving){
		sprite1.intra_anim_frame_counter = 0;
		sprite1.anim_frame_counter = 0;
	} else{
		skip_sprite_animation = this->wram.wd736.get_pc_spinning();
		if (!skip_sprite_animation){
			auto sprite = this->wram.wSpriteStateData1[(this->hram.H_CURRENTSPRITEOFFSET / SpriteStateData1::size) + 1];
			if (++sprite.intra_anim_frame_counter != 4)
				skip_sprite_animation = true;
			else{
				sprite.intra_anim_frame_counter = 0;
				sprite.anim_frame_counter = (sprite.anim_frame_counter + 1) % 4;
			}
		}
	}

	if (!skip_sprite_animation)
		sprite1.sprite_image_idx = sprite1.anim_frame_counter + (unsigned)sprite1.facing_direction;

	//If the player is standing on a grass tile, make the player's sprite have
	//lower priority than the background so that it's partially obscured by the
	//grass.Only the lower half of the sprite is permitted to have the priority
	//bit set by later logic.
	sprite2.grass_priority = 0x80 * (this->hram.hTilePlayerStandingOn == this->wram.wGrassTile);
}

void CppRed::update_non_player_sprite(const SpriteStateData2 &sprite){
	this->hram.hCurrentSpriteOffset2 = (sprite.sprite_image_base_offset + 1) << 4;
	if (this->wram.wNPCMovementScriptSpriteOffset == this->hram.H_CURRENTSPRITEOFFSET)
		this->do_scripted_npc_movement();
	else
		this->update_npc_sprite(sprite);
}

void CppRed::do_scripted_npc_movement(){
	//This is an alternative method of scripting an NPC's movement and is only used
	//a few times in the game. It is used when the NPC and player must walk together
	//in sync, such as when the player is following the NPC somewhere. An NPC can't
	//be moved in sync with the player using the other method.

	if (!this->wram.wd730.get_input_simulated())
		return;

	bool initialized = this->wram.wd72e.get_npc_movement_initialized();
	if (!initialized){
		this->wram.wd72e.set_npc_movement_initialized(true);
		this->initialize_scripted_npc_movement();
		return;
	}

	auto direction = (NpcMovementDirection)this->wram.wNPCMovementDirections2[this->wram.wNPCMovementDirections2Index];
	//std::unique_ptr<SpriteStateData1::member_type> pointer;
	auto sprite = this->get_current_sprite1();
	SpriteFacingDirection sprite_direction = SpriteFacingDirection::Down;
	int displacement = 0;
	bool axis = false;
	switch (direction){
		case NpcMovementDirection::Down:
			axis = true;
			sprite_direction = SpriteFacingDirection::Down;
			displacement = 2;
			break;
		case NpcMovementDirection::Up:
			axis = true;
			sprite_direction = SpriteFacingDirection::Up;
			displacement = -2;
			break;
		case NpcMovementDirection::Left:
			axis = false;
			sprite_direction = SpriteFacingDirection::Left;
			displacement = -2;
			break;
		case NpcMovementDirection::Right:
			axis = false;
			sprite_direction = SpriteFacingDirection::Right;
			displacement = 2;
			break;
		default:
			//Is this an error condition?
			return;
	}

	if (!axis)
		sprite.x_pixels += displacement;
	else
		sprite.y_pixels += displacement;
	sprite.facing_direction = sprite_direction;
	this->anim_scripted_npc_movement();
	if (--this->wram.wScriptedNPCWalkCounter)
		return;
	this->wram.wScriptedNPCWalkCounter = 8;
	this->wram.wNPCMovementDirections2Index++;
}

void CppRed::initialize_scripted_npc_movement(){
	this->wram.wNPCMovementDirections2Index = 0;
	this->wram.wScriptedNPCWalkCounter = 8;
	this->anim_scripted_npc_movement();
}

void CppRed::anim_scripted_npc_movement(){
	{
		auto sprite1 = this->get_current_sprite1();
		auto sprite2 = this->get_current_sprite2();
		auto offset = (sprite2.sprite_image_base_offset - 1) << 4;
		auto direction = (SpriteFacingDirection)sprite1.facing_direction;
		switch (direction){
			case SpriteFacingDirection::Down:
			case SpriteFacingDirection::Up:
			case SpriteFacingDirection::Left:
			case SpriteFacingDirection::Right:
				break;
			default:
				return;
		}
		offset = (offset + (unsigned)direction) & 0xFF;
		this->hram.hSpriteVRAMSlotAndFacing = offset;
	}
	this->advance_scripted_npc_anim_frame_counter();
	{
		auto sprite1 = this->get_current_sprite1();
		sprite1.sprite_image_idx = this->hram.hSpriteVRAMSlotAndFacing + this->hram.hSpriteAnimFrameCounter;
	}
}

void CppRed::advance_scripted_npc_anim_frame_counter(){
	auto sprite1 = this->get_current_sprite1();
	if (++sprite1.intra_anim_frame_counter != 4)
		return;
	sprite1.intra_anim_frame_counter = 0;
	auto new_value = (sprite1.anim_frame_counter + 1) % 4;
	sprite1.anim_frame_counter = new_value;
	this->hram.hSpriteAnimFrameCounter = new_value;
}

unsigned transform_thing(unsigned x, unsigned val){
	const unsigned down = 0xD0;
	const unsigned up = 0xD1;
	const unsigned left = 0xD2;
	const unsigned right = 0xD3;

	assert((x >> 6) < 0x100);
	switch (x >> 6){
		case 0:
			return val == 2 ? left : down;
		case 1:
			return val == 2 ? right : up;
		case 2:
			return val == 1 ? up : left;
		case 3:
			return val == 1 ? down : right;
	}
	assert(false);
}

void CppRed::update_npc_sprite(const SpriteStateData2 &){
	unsigned offset = this->hram.H_CURRENTSPRITEOFFSET;
	offset = (offset / 16) - 1;
	this->wram.wCurSpriteMovement2 = this->wram.wMapSpriteData[offset].movement_byte_2;
	{
		auto sprite = this->get_current_sprite1();
		if (sprite.movement_status.get_movement_status() == MovementStatus::Uninitialized){
			this->initialize_sprite_status();
			return;
		}
	}
	if (!this->check_sprite_availability())
		return;
	{
		auto sprite = this->get_current_sprite1();
		auto status = sprite.movement_status;
		if (status.get_face_player()){
			this->make_npc_face_player(sprite);
			return;
		}
		if (this->wram.wFontLoaded.get_in_use()){
			this->not_yet_moving(sprite);
			return;
		}
		switch (status.get_movement_status()){
			case MovementStatus::Delayed:
				this->update_sprite_movement_delay();
				return;
			case MovementStatus::Moving:
				this->update_sprite_in_walking_animation();
				return;
		}
		if (this->wram.wWalkCounter)
			return;
		this->initialize_sprite_screen_position();
	}
	{
		auto sprite = this->get_current_sprite2();
		auto movement = (unsigned)sprite.movement_byte1;
		std::uint32_t x;
		auto tile = this->get_tilemap_location(0, 0);
		if (movement == 0xFF || movement == 0xFE){
			tile = this->get_tile_sprite_stands_on();
			x = this->random() & 0xFF;
		} else{
			sprite.movement_byte1 = movement + 1;
			this->wram.wNPCNumScriptedSteps--;
			auto direction = (std::uint32_t)this->wram.wNPCMovementDirections[movement];
			if (direction == 0xE0){ //Turn?
				this->change_facing_direction();
				return;
			}
			if (direction == 0xFF){ //== STAY
				sprite.movement_byte1 = 0xFF;
				this->wram.wd730.set_sprite_automoved(false);
				this->wram.wSimulatedJoypadStatesIndex = 0;
				this->wram.wWastedByteCD3A = 0;
				return;
			}
			if (direction == 0xFE){ //== WALK
				sprite.movement_byte1 = 1;
				x = this->wram.wNPCMovementDirections[direction];
			} else
				x = direction;
		}
		auto test = (unsigned)this->wram.wCurSpriteMovement2;
		const auto break_loop = std::numeric_limits<unsigned>::max();

		const unsigned down = 0xD0;
		const unsigned up = 0xD1;
		const unsigned left = 0xD2;
		const unsigned right = 0xD3;

		while (true){
			switch (test){
				case down:
					tile += 2 * tilemap_width;
					this->try_walking(tile, 0, 1, DirectionBitmap::Down, SpriteFacingDirection::Down);
					return;
				case up:
					tile -= 2 * tilemap_width;
					this->try_walking(tile, 0, -1, DirectionBitmap::Up, SpriteFacingDirection::Up);
					return;
				case left:
					tile -= 2;
					this->try_walking(tile, -1, 0, DirectionBitmap::Left, SpriteFacingDirection::Left);
					return;
				case right:
					tile += 2;
					this->try_walking(tile, 1, 0, DirectionBitmap::Right, SpriteFacingDirection::Right);
				default:
					test = transform_thing(x, this->wram.wCurSpriteMovement2);
					continue;
			}
		}
	}
}

CppRed::tilemap_it CppRed::get_tile_sprite_stands_on(){
	auto sprite = this->get_current_sprite1();
	unsigned y = sprite.y_pixels;
	y = (y + 4) % 256;
	y = (y - y % 16) / 8;
	unsigned x = sprite.x_pixels;
	x = x / 8;
	return this->get_tilemap_location(x, y + 1);
}

void CppRed::change_facing_direction(){
	//TODO
	this->try_walking(this->get_tilemap_location(0, 0), 0, 0, (DirectionBitmap)0, (SpriteFacingDirection)0);
}

bool CppRed::try_walking(tilemap_it dst, int deltax, int deltay, DirectionBitmap movement_direction, SpriteFacingDirection facing_sprite_direction){
	auto sprite1 = this->get_current_sprite1();
	sprite1.facing_direction = facing_sprite_direction;
	sprite1.x_step_vector = deltax;
	sprite1.y_step_vector = deltay;
	if (!this->can_walk_onto_tile(*dst, movement_direction, deltax, deltay))
		return false;
	auto sprite2 = this->get_current_sprite2();
	sprite2.map_x += deltax;
	sprite2.map_y -= deltay;
	sprite2.walk_animation_counter = 10;
	sprite1.movement_status.clear();
	sprite1.movement_status.set_movement_status(MovementStatus::Moving);
	this->update_sprite_image(sprite1);
	return true;
}

void CppRed::update_sprite_image(SpriteStateData1 &sprite){
	sprite.sprite_image_idx = sprite.anim_frame_counter + (unsigned)sprite.facing_direction + this->hram.hCurrentSpriteOffset2;
}

void CppRed::initialize_sprite_status(){
	auto sprite1 = this->get_current_sprite1();
	sprite1.movement_status.clear();
	sprite1.movement_status.set_movement_status(MovementStatus::Ready);
	sprite1.sprite_image_idx = 0xFF;
	auto sprite2 = this->get_current_sprite2();
	sprite2.y_displacement = 8;
	sprite2.x_displacement = 8;
}

void CppRed::initialize_sprite_screen_position(){
	auto sprite2 = this->get_current_sprite2();
	auto y = (sprite2.map_y - this->wram.wYCoord) * 16 - 4;
	auto sprite1 = this->get_current_sprite1();
	sprite1.y_pixels = y;
	auto x = (sprite2.map_x - this->wram.wXCoord) * 16;
	sprite1.x_pixels = x;
}

bool CppRed::is_object_hidden(){
	auto b = this->hram.H_CURRENTSPRITEOFFSET / 16;
	bool ret = false;
	for (auto &it : this->wram.wMissableObjectList){
		unsigned sprite_id = it.sprite_id;
		if (sprite_id == 0xFF)
			break;
		if (sprite_id != b)
			continue;
		ret = this->missable_objects_flag_action(FlagAction::Test, this->wram.wMissableObjectFlags, it.missable_object_index);
		break;
	}
	this->hram.hIsObjectHidden = ret;
	return ret;
}

SpriteStateData1 CppRed::get_current_sprite1(){
	return this->wram.wSpriteStateData1[this->hram.H_CURRENTSPRITEOFFSET / SpriteStateData1::size];
}

SpriteStateData2 CppRed::get_current_sprite2(){
	return this->wram.wSpriteStateData2[this->hram.H_CURRENTSPRITEOFFSET / SpriteStateData2::size];
}

bool CppRed::check_sprite_availability(){
	this->call_predef(Predef::IsObjectHidden);
	bool ret = true;
	if (this->hram.hIsObjectHidden)
		ret = false;

	unsigned tile_value = 0;
	if (ret){
		auto sprite2 = this->get_current_sprite2();
		if (sprite2.movement_byte1 >= 0xFE){
			ret &= this->wram.wYCoord <= sprite2.map_y && sprite2.map_y < this->wram.wYCoord + tilemap_height / 2;
			ret &= this->wram.wXCoord <= sprite2.map_x && sprite2.map_x < this->wram.wXCoord + tilemap_width / 2;
		}
		auto tile = this->get_tile_sprite_stands_on();
		ret = ret &&
			tile[0] < 0x60 &&
			tile[1] < 0x60 &&
			tile[-tilemap_width] < 0x60 &&
			tile[-tilemap_width + 1] < 0x60;
		tile_value = tile[-tilemap_width + 1];
	}

	auto sprite = this->get_current_sprite1();
	if (!ret){
		sprite.sprite_image_idx = 0xFF;
	}else if (!this->wram.wWalkCounter){
		this->update_sprite_image(sprite);
		auto sprite2 = this->get_current_sprite2();
		sprite2.grass_priority = this->wram.wGrassTile != tile_value ? 0 : 0x80;
	}
	return ret;
}

bool CppRed::can_walk_onto_tile_helper(unsigned tile_id, DirectionBitmap direction, int deltax, int deltay){
	auto collision = (const byte_t *)this->map_pointer(this->wram.wTilesetCollisionPtr);
	while (true){
		auto value = *(collision++);
		if (value == 0xFF)
			return false;
		if (value == tile_id)
			break;
	}
	{
		auto sprite2 = this->get_current_sprite2();
		if (sprite2.movement_byte1 == 0xFF)
			return false;
	}
	{
		auto sprite1 = this->get_current_sprite1();
		//Explanation: The sprite location is at the top left, and every sprite is
		//2x2 tiles big, and every tile is 8x8 pixels big. So take the size of the
		//tilemap in dimension Q, subtract the size of the sprite in that
		//dimension, and multiply by 8.
		if (sprite1.y_pixels + 4 + deltay >= (tilemap_height - 2) * 8)
			return false;
		if (sprite1.x_pixels + deltax >= (tilemap_width - 2) * 8)
			return false;
	}

	this->detect_sprite_collision();

	auto sprite1 = this->get_current_sprite1();
	if (sprite1.collision_bits & (unsigned)direction)
		return false;
	auto sprite2 = this->get_current_sprite2();
	int x = sprite2.x_displacement;
	int y = sprite2.y_displacement;
	if (deltay >= 0){
		y += deltay;
		if (y < 5)
			//Bug: this tests probably were supposed to prevent sprites
			//from walking out too far, but this line makes sprites get stuck
			//whenever they walked upwards 5 steps
			//on the other hand, the amount a sprite can walk out to the
			//right or bottom is not limited (until the counter overflows)
			return false;
	} else if (--y < 0)
		return false;
	if (deltax >= 0){
		x += deltax;
		if (x < 5)
			//Same as above.
			return false;
	} else if (--x < 0)
		return false;
	sprite2.x_displacement = reduce_sign(x);
	sprite2.y_displacement = reduce_sign(y);
	return true;
}

bool CppRed::can_walk_onto_tile(unsigned tile_id, DirectionBitmap direction, int deltax, int deltay){
	{
		auto sprite2 = this->get_current_sprite2();
		if (sprite2.movement_byte1 < 0xFE)
			//Always allow walking if the movement is scripted.
			return true;
	}
	if (this->can_walk_onto_tile_helper(tile_id, direction, deltax, deltay))
		return true;

	auto sprite1 = this->get_current_sprite1();
	sprite1.movement_status.clear();
	sprite1.movement_status.set_movement_status(MovementStatus::Delayed);
	sprite1.y_step_vector = 0;
	sprite1.x_step_vector = 0;
	auto sprite2 = this->get_current_sprite2();
	sprite2.movement_delay = this->random() % 128;
	return false;
}

void CppRed::make_npc_face_player(SpriteStateData1 &sprite){
	if (this->wram.wd72d.get_npcs_dont_turn_when_spoken_to()){
		this->not_yet_moving(sprite);
		return;
	}
	sprite.movement_status.set_face_player(false);
	SpriteFacingDirection direction;
	auto player_direction = (unsigned)this->wram.wPlayerDirection;
	if (check_flag(player_direction, (unsigned)PlayerDirectionBitmap::Up))
		direction = SpriteFacingDirection::Down;
	else if (check_flag(player_direction, (unsigned)PlayerDirectionBitmap::Down))
		direction = SpriteFacingDirection::Up;
	else if (check_flag(player_direction, (unsigned)PlayerDirectionBitmap::Left))
		direction = SpriteFacingDirection::Right;
	else
		direction = SpriteFacingDirection::Left;

	sprite.facing_direction = direction;
	this->not_yet_moving(sprite);
}

void CppRed::not_yet_moving(SpriteStateData1 &sprite){
	sprite.anim_frame_counter = 0;
	this->update_sprite_image(sprite);
}

void CppRed::update_sprite_movement_delay(){
	auto sprite1 = this->get_current_sprite1();
	auto sprite2 = this->get_current_sprite2();
	if (sprite2.movement_byte1 >= 0xFE)
		sprite2.movement_delay--;
	else{
		sprite2.movement_delay = 0;
		sprite1.movement_status.clear();
		sprite1.movement_status.set_movement_status(MovementStatus::Ready);
	}
	this->not_yet_moving(sprite1);
}

void CppRed::update_sprite_in_walking_animation(){
	auto sprite1 = this->get_current_sprite1();
	if (++sprite1.intra_anim_frame_counter == 4){
		sprite1.intra_anim_frame_counter = 0;
		sprite1.anim_frame_counter = sprite1.anim_frame_counter % 4;
	}
	sprite1.y_pixels += sprite1.y_step_vector;
	sprite1.x_pixels += sprite1.x_step_vector;

	auto sprite2 = this->get_current_sprite2();
	if (++sprite2.walk_animation_counter)
		return;
	if (sprite2.movement_byte1 < 0xFE){
		sprite1.movement_status.clear();
		sprite1.movement_status.set_movement_status(MovementStatus::Ready);
		return;
	}
	sprite2.movement_delay = this->random() % 128;
	sprite1.movement_status.clear();
	sprite1.movement_status.set_movement_status(MovementStatus::Delayed);
	sprite1.y_step_vector = 0;
	sprite1.x_step_vector = 0;
}
