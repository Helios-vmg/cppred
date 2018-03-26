#pragma once
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include "queue/readerwriterqueue.h"
#include "utility.h"

template <typename T>
class PublishingResource{
	std::vector<std::unique_ptr<T>> allocated;
	std::vector<T *> ready;
	std::mutex ready_mutex;
	//Invariant: private_resource is valid at all times.
	T *private_resource;
	std::atomic<T *> public_resource;

	T *reuse_or_allocate(){
		{
			LOCK_MUTEX(this->ready_mutex);
			if (this->ready.size()){
				auto ret = this->ready.back();
				this->ready.pop_back();
				return ret;
			}
		}
		return this->allocate();
	}
	T *allocate(){
		this->allocated.push_back(std::make_unique<T>());
		return this->allocated.back().get();
	}
public:
	PublishingResource(){
		this->private_resource = this->allocate();
		this->public_resource = nullptr;
	}
	void publish(){
		this->private_resource = (T *)std::atomic_exchange(&this->public_resource, this->private_resource);
		if (!this->private_resource)
			this->private_resource = this->reuse_or_allocate();
	}
	T *get_private_resource(){
		return this->private_resource;
	}
	T *get_public_resource(){
		return (T *)std::atomic_exchange(&this->public_resource, (T *)nullptr);
	}
	void return_resource_as_ready(T *r){
		LOCK_MUTEX(this->ready_mutex);
		this->ready.push_back(r);
	}
	void return_resource_as_private(T *r){
		T *null_T = nullptr;
		if (std::atomic_compare_exchange_strong(&this->public_resource, &null_T, r))
			return;
		this->return_resource_as_ready(r);
	}
	void clear_public_resource(){
		auto frame = (T *)std::atomic_exchange(&this->public_resource, (T *)nullptr);
		if (frame){
			LOCK_MUTEX(this->ready_mutex);
			this->ready.push_back(frame);
		}
	}
};

//#define QueuedPublishingResource_USE_DEQUE
#ifdef QueuedPublishingResource_USE_DEQUE
#include <deque>
#endif

template <typename T>
class QueuedPublishingResource{
	std::vector<std::unique_ptr<T>> allocated;
#ifndef  QueuedPublishingResource_USE_DEQUE
	moodycamel::ReaderWriterQueue<T *> return_queue, queue;
#else
	std::deque<T *> return_queue, queue;
	std::mutex return_queue_mutex, queue_mutex;
#endif
	//Invariant: private_resource is valid at all times.
	T *private_resource;

	T *reuse_or_allocate(){
		T *ret;
#ifndef QueuedPublishingResource_USE_DEQUE
		if (this->return_queue.try_dequeue(ret))
			return ret;
#else
		{
			LOCK_MUTEX(this->return_queue_mutex);
			if (this->return_queue.size()){
				ret = this->return_queue.front();
				this->return_queue.pop_front();
				return ret;
			}
		}
#endif
		return this->allocate();
	}
	T *allocate(){
		this->allocated.push_back(std::make_unique<T>());
		return this->allocated.back().get();
	}
public:
	QueuedPublishingResource(){
		this->private_resource = this->allocate();
	}
#ifndef QueuedPublishingResource_USE_DEQUE
	QueuedPublishingResource(size_t max_capacity): return_queue(max_capacity), queue(max_capacity){
#else
	QueuedPublishingResource(size_t max_capacity){
#endif
		this->private_resource = this->allocate();
	}
	void publish(){
#ifndef QueuedPublishingResource_USE_DEQUE
		if (!this->queue.try_enqueue(this->private_resource))
			return;
#else
		{
			LOCK_MUTEX(this->queue_mutex);
			this->queue.push_back(this->private_resource);
		}
#endif
		this->private_resource = this->reuse_or_allocate();
	}
	T *get_private_resource(){
		return this->private_resource;
	}
	T *get_public_resource(){
		T *ret = nullptr;
#ifndef QueuedPublishingResource_USE_DEQUE
		this->queue.try_dequeue(ret);
#else
		LOCK_MUTEX(this->queue_mutex);
		if (this->queue.size()){
			ret = this->queue.front();
			this->queue.pop_front();
		}
#endif
		return ret;
	}
	void return_resource(T *r){
#ifndef QueuedPublishingResource_USE_DEQUE
		this->return_queue.enqueue(r);
#else
		LOCK_MUTEX(this->return_queue_mutex);
		auto it = std::find(this->return_queue.begin(), this->return_queue.end(), r);
		assert(it == this->return_queue.end());
		this->return_queue.push_back(r);
#endif
	}
	void clear_public_resource(){
#ifndef QueuedPublishingResource_USE_DEQUE
		T *p;
		while (this->queue.try_dequeue(p))
			this->return_queue.enqueue(p);
#else
		LOCK_MUTEX(this->queue_mutex);
		LOCK_MUTEX(this->return_queue_mutex);
		while (this->queue.size()){
			this->return_queue.push_back(this->queue.front());
			this->queue.pop_front();
		}
#endif
	}
};
