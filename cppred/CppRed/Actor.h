#pragma once
#include "Renderer.h"
#include "../utility.h"
#include <deque>
#include <functional>

class MapObjectInstance;

namespace CppRed{

class Game;
class ScreenOwner;

struct PathStep{
	FacingDirection movement_direction;
	Point after_state;
};

enum class EmotionBubble{
	Surprise = 0,
	Confusion = 1,
	Happiness = 2,
};

class Actor{
protected:
	Game *game;
	const GraphicsAsset *sprite;
	Renderer *renderer;
	int object_id;
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
	bool visible = true;
	bool ignore_occupancy = false;
	std::deque<std::function<void()>> saved_actions;
	bool aborting_movement = false;

	template <typename T>
	void apply_to_all_sprites(const T &f){
		for (int i = 0; i < array_length(this->standing_sprites); i++)
			f(*this->standing_sprites[i]);
		for (int i = 0; i < array_length(this->walking_sprites); i++)
			f(*this->walking_sprites[i]);
	}
	virtual void initialize_sprites(const GraphicsAsset &graphics, Renderer &);
	void initialize_full_sprite(const GraphicsAsset &graphics, Renderer &);
	void initialize_reduced_sprite(const GraphicsAsset &graphics, Renderer &);
	void initialize_single_sprite(const GraphicsAsset &graphics, Renderer &);
	virtual void coroutine_entry_point();
	bool run_walking_animation(const Point &delta, FacingDirection);
	bool move(const Point &delta, FacingDirection);
	virtual void entered_new_map(Map old_map, Map new_map, bool warped){}
	virtual bool can_move_to(const WorldCoordinates &current_position, const WorldCoordinates &next_position, FacingDirection direction);
	virtual void update_sprites(){}
	virtual void about_to_move(){}
	virtual bool move_internal(FacingDirection);
	bool run_saved_actions();
public:
	Actor(Game &game, Coroutine &parent_coroutine, const std::string &name, Renderer &renderer, const GraphicsAsset &sprite);
	virtual ~Actor();
	virtual void init();
	virtual void uninit();
	virtual void update();
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
	void pause();
	virtual void set_facing_direction(FacingDirection direction);
	DEFINE_GETTER(facing_direction)
	DEFINE_GETTER_SETTER(pixel_offset)
	DEFINE_GETTER_SETTER(ignore_occupancy)
	DEFINE_GETTER(name)
	DEFINE_GETTER_SETTER(object_id)
	DEFINE_GETTER(visible)
	bool is_moving() const{
		return this->moving;
	}
	void set_visible(bool visible);
	bool move(FacingDirection);
	void stop(){
		this->coroutine.reset();
	}
	virtual double movement_duration() const{
		return Renderer::tile_size * 2;
	}
	std::vector<PathStep> find_path(const Point &destination);
	void follow_path(const std::vector<PathStep> &);
	virtual bool get_random_facing_direction() const{
		return false;
	}
	virtual void set_random_facing_direction(bool value){}
	void abort_movement(){
		this->aborting_movement = true;
	}
	std::shared_ptr<Sprite> show_emotion_bubble(Renderer &, EmotionBubble);
};

class NonPlayerActor : public Actor{
protected:
	virtual void coroutine_entry_point() override;
	virtual void update_sprites() override;
public:
	NonPlayerActor(Game &game, Coroutine &parent_coroutine, const std::string &name, Renderer &renderer, const GraphicsAsset &sprite, MapObjectInstance &);
};

template <typename T> 
using actor_ptr = std::unique_ptr<T, void (*)(T *)>;

template <typename T>
void actor_deleter(T *p){
	if (!p)
		return;
	p->uninit();
	delete p;
}

template <typename T>
void actor_deleter2(Actor *p){
	if (!p)
		return;
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
