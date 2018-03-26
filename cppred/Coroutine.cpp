#include "Coroutine.h"
#include "utility.h"
#include "Engine.h"
#include <boost/coroutine2/all.hpp>
#include <stdexcept>

class Coroutine::Pimpl{
	thread_local static Pimpl *coroutine_stack;
	Pimpl *next_coroutine;
	Coroutine *owner;
	std::string name;
	PausableClock clock;
	bool active = false;
	std::thread::id resume_thread_id;
	typedef boost::coroutines2::asymmetric_coroutine<void>::pull_type coroutine_t;
	typedef boost::coroutines2::asymmetric_coroutine<void>::push_type yielder_t;
	std::unique_ptr<coroutine_t> coroutine;
	on_yield_t on_yield;
	entry_point_t entry_point;
	yielder_t *yielder = nullptr;
	bool first_run;
	double wait_remainder = 0;
	
	void push(){
		this->next_coroutine = coroutine_stack;
		coroutine_stack = this;
	}
	void pop(){
		coroutine_stack = this->next_coroutine;
	}
	void init(){
		this->first_run = true;
		this->push();
		this->coroutine.reset(new coroutine_t([this](yielder_t &y){
			this->resume_thread_id = std::this_thread::get_id();
			this->yielder = &y;
			if (this->first_run){
				this->yield();
				this->first_run = false;
			}
			this->entry_point(*this->owner);
		}));
		this->pop();
	}
public:
	Pimpl(Coroutine &owner, const std::string &name, AbstractClock &base_clock, entry_point_t &&entry_point):
			owner(&owner),
			name(name),
			clock(base_clock, name + " clock"),
			entry_point(std::move(entry_point)){
		this->init();
	}
	Pimpl(Coroutine &owner, const std::string &name, entry_point_t &&entry_point):
		Pimpl(owner, name, get_current_coroutine().get_clock(), std::move(entry_point)){}
	bool resume(){
		this->resume_thread_id = std::this_thread::get_id();
		if (this->active)
			throw std::runtime_error("Attempting to resume a running coroutine!");
		this->active = true;
		this->push();
		//Logger() << this->name << " RESUMES\n";
		this->clock.resume();
		auto ret = !!(*this->coroutine)();
		//Logger() << this->name << " PAUSES\n";
		this->pop();
		this->active = false;
		if (!ret)
			this->init();
		return ret;
	}
	void yield(){
		if (coroutine_stack != this)
			throw std::runtime_error("Coroutine::yield() was used incorrectly!");
		if (std::this_thread::get_id() != this->resume_thread_id)
			throw std::runtime_error("Coroutine::yield() must be called from the thread that resumed the coroutine!");
		if (!this->yielder)
			throw std::runtime_error("Coroutine::yield() must be called while the coroutine is active!");
		auto yielder = this->yielder;
		this->yielder = nullptr;
		(*yielder)();
		this->yielder = yielder;
		if (this->on_yield)
			this->on_yield();
	}
	void wait(double s){
		auto target = this->clock.get() + s + this->wait_remainder;
		while (true){
			this->yield();
			auto now = this->clock.get();
			if (now >= target){
				this->wait_remainder = target - now;
				return;
			}
		}
	}
	void wait_frames(int frames){
		this->wait(frames * Engine::logical_refresh_period);
	}
	void set_on_yield(on_yield_t &&on_yield){
		this->on_yield = std::move(on_yield);
	}
	void clear_on_yield(){
		this->on_yield = on_yield_t();
	}
	static Coroutine *get_current_coroutine_ptr(){
		return coroutine_stack->owner;
	}
	static Coroutine &get_current_coroutine(){
		auto p = get_current_coroutine_ptr();
		if (!p)
			throw std::runtime_error("No coroutine is running!");
		return *p;
	}
	PausableClock &get_clock(){
		return this->clock;
	}
	DEFINE_GETTER(name)
	DEFINE_GETTER(active)
};

thread_local Coroutine::Pimpl *Coroutine::Pimpl::coroutine_stack = nullptr;

Coroutine::Coroutine(const std::string &name, entry_point_t &&entry_point){
	this->pimpl.reset(new Pimpl(*this, name, std::move(entry_point)));
}

Coroutine::Coroutine(const std::string &name, AbstractClock &base_clock, entry_point_t &&entry_point){
	this->pimpl.reset(new Pimpl(*this, name, base_clock, std::move(entry_point)));
}

Coroutine::~Coroutine(){}

void Coroutine::yield(){
	this->pimpl->yield();
}

bool Coroutine::resume(){
	return this->pimpl->resume();
}

void Coroutine::wait(double s){
	this->pimpl->wait(s);
}

void Coroutine::wait_frames(int frames){
	this->wait(frames * Engine::logical_refresh_period);
}

void Coroutine::set_on_yield(on_yield_t &&on_yield){
	this->pimpl->set_on_yield(std::move(on_yield));
}

void Coroutine::clear_on_yield(){
	this->pimpl->clear_on_yield();
}

Coroutine *Coroutine::get_current_coroutine_ptr(){
	return Pimpl::get_current_coroutine_ptr();
}

Coroutine &Coroutine::get_current_coroutine(){
	return Pimpl::get_current_coroutine();
}

PausableClock &Coroutine::get_clock(){
	return this->pimpl->get_clock();
}
