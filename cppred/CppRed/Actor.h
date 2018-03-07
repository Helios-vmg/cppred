#pragma once
#include "Renderer.h"
#include "../utility.h"

class MapObjectInstance;

namespace CppRed{

enum class FacingDirection{
	Up = 0,
	Right,
	Down,
	Left,
};

class Game;

class Actor{
protected:
	Game *game;
	const GraphicsAsset *sprite;
	Renderer *renderer;
	std::string name;
	WorldCoordinates position;
	Point pixel_offset;
	std::shared_ptr<Sprite> standing_sprites[4];
	std::shared_ptr<Sprite> walking_sprites[4 * 4];
	FacingDirection facing_direction = FacingDirection::Down;
	bool moving = false;
	int walking_animation_state = 0;
	std::unique_ptr<Coroutine> coroutine;
	bool quit_coroutine = false;
	MapObjectInstance *object_instance = nullptr;

	template <typename T>
	void apply_to_all_sprites(const T &f){
		for (int i = 0; i < array_length(this->standing_sprites); i++)
			f(*this->standing_sprites[i]);
		for (int i = 0; i < array_length(this->walking_sprites); i++)
			f(*this->walking_sprites[i]);
	}
	virtual void initialize_sprites(const GraphicsAsset &graphics, Renderer &);
	virtual void coroutine_entry_point();
	void run_walking_animation(const Point &delta, FacingDirection);
	bool move(FacingDirection);
	bool move(const Point &delta, FacingDirection);
	virtual void entered_new_map(Map old_map, Map new_map){}
	virtual double movement_duration() const{
		return Renderer::tile_size * 2;
	}
	static Point direction_to_vector(FacingDirection);
	virtual bool can_move_to(const WorldCoordinates &current_position, const WorldCoordinates &next_position, FacingDirection direction);
public:
	Actor(Game &game, const std::string &name, Renderer &renderer, const GraphicsAsset &sprite);
	virtual void init();
	virtual ~Actor(){}
	virtual void uninit();
	virtual void update();
	virtual void update_sprites(){}
	void set_visible_sprite();

	void set_current_map(Map map){
		this->position.map = map;
	}
	void set_map_position(const Point &p){
		this->position.position = p;
	}
	Map get_current_map() const{
		return this->position.map;
	}
	Point get_map_position() const{
		return this->position.position;
	}
	DEFINE_GETTER_SETTER(facing_direction)
	DEFINE_GETTER_SETTER(pixel_offset)
};

template <typename T> 
using actor_ptr = std::unique_ptr<T, void (*)(T *)>;

template <typename T>
void actor_deleter(T *p){
	p->uninit();
	delete p;
}

template <typename T>
void actor_deleter2(Actor *p){
	p->uninit();
	delete (T *)p;
}

template <typename T>
actor_ptr<T> null_actor_ptr(){
	return actor_ptr<T>(nullptr, actor_deleter<T>);
}

template <typename T, typename... P>
actor_ptr<T> create_actor(P &&... params){
	actor_ptr<T> ret(new T(std::forward<P>(params)...), actor_deleter<T>);
	ret->init();
	return ret;
}

template <typename T, typename... P>
actor_ptr<Actor> create_actor2(P &&... params){
	actor_ptr<Actor> ret(new T(std::forward<P>(params)...), actor_deleter2<T>);
	ret->init();
	return ret;
}

}
