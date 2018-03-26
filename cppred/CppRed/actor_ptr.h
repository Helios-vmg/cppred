#pragma once

namespace CppRed{

class Actor;

template <typename T>
using actor_ptr = std::unique_ptr<T, void(*)(T *)>;
void actor_deleter_helper(Actor *p);

template <typename T>
actor_ptr<T> null_actor_ptr(){
	return actor_ptr<T>(nullptr, [](T *p){ actor_deleter_helper(p); });
}

template <typename T, typename... P>
actor_ptr<T> create_actor(P &&... params){
	actor_ptr<T> ret(new T(std::forward<P>(params)...), [](T *p){ actor_deleter_helper(p); });
	ret->init();
	return ret;
}

template <typename T, typename... P>
actor_ptr<Actor> create_actor2(P &&... params){
	actor_ptr<Actor> ret(new T(std::forward<P>(params)...), actor_deleter_helper);
	ret->init();
	return ret;
}

}
