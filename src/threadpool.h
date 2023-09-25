#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <atomic>
#include <thread>
#include <functional>
#include <utility>

#include "bounded_mpmc_queue.h"
#include "chained_task.h"
#include "branched_task.h"
#include "backoff.h"
#include "scheduler.h"

struct Threadpool {
	bounded_mpmc_queue<std::function<void()>> queue{128};
	std::atomic<bool> running{true};
	std::vector<std::jthread> threads;
	
	//Constructor
	Threadpool(int num_threads){
		auto work = [this](){
			Backoff backoff;
			
			while(running){
				std::function<void()> task;
				if(dequeue(task)){
					task();
					backoff.reset();
				}else{
					backoff.backoff();
				}
			}

		};

		for (int i=0; i<num_threads; i++){
			threads.emplace_back(work);
		}
	}

	//Destructor
	~Threadpool(){
		running.store(false);
	}
	
	
	void schedule(std::function<void()> task){
		while(!queue.try_enqueue(task)){
			//Keep retrying to enqueue	
		}
	}

	bool dequeue(std::function<void()>& task){
		return queue.try_dequeue(task);
	}


	//Chaining Implementation
	template<template<typename>typename AWAITABLE, typename T>
	auto chain(AWAITABLE<T>&& awaitable){
		return chain_by_value_on<Threadpool, AWAITABLE, T>(*this, std::forward<AWAITABLE<T>>(awaitable));
	};

	template<template<typename>typename AWAITABLE, typename T>
	auto chain(AWAITABLE<T>& awaitable){
		return chain_by_reference_on<Threadpool, AWAITABLE, T>(*this, awaitable);
	};

	
	//Branching Implementation	
	template<template<typename>typename AWAITABLE, typename T>
	auto branch(AWAITABLE<T>&& awaitable){
		return branch_by_value_on<Threadpool, AWAITABLE, T>(*this, std::forward<AWAITABLE<T>>(awaitable));
	};

};

#endif
