#pragma once
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <iostream>
#include "queue/readerwriterqueue.h"

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
			std::lock_guard<std::mutex> lg(this->ready_mutex);
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
		std::lock_guard<std::mutex> lg(this->ready_mutex);
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
			std::lock_guard<std::mutex> lg(this->ready_mutex);
			this->ready.push_back(frame);
		}
	}
};

template <typename T>
class QueuedPublishingResource{
	std::vector<std::unique_ptr<T>> allocated;
	moodycamel::ReaderWriterQueue<T *> return_queue;
	//Invariant: private_resource is valid at all times.
	T *private_resource;
	moodycamel::ReaderWriterQueue<T *> queue;

	T *reuse_or_allocate(){
		T *ret;
		if (this->return_queue.try_dequeue(ret))
			return ret;
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
	QueuedPublishingResource(size_t max_capacity): return_queue(max_capacity), queue(max_capacity){
		this->private_resource = this->allocate();
	}
	void publish(){
		if (!this->queue.try_enqueue(this->private_resource))
			return;
		this->private_resource = this->reuse_or_allocate();
	}
	T *get_private_resource(){
		return this->private_resource;
	}
	T *get_public_resource(){
		T *ret = nullptr;
		this->queue.try_dequeue(ret);
		return ret;
	}
	void return_resource(T *r){
		this->return_queue.enqueue(r);
	}
	void clear_public_resource(){
		T *p;
		while (this->queue.try_dequeue(p))
			this->return_queue.enqueue(p);
	}
};
