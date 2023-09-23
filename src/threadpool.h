#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <atomic>
#include <thread>
#include <iostream>
#include <functional>
#include <utility>
#include "mpmc.h"
#include "threadpool_chained_task.h"
#include "threadpool_branched_task.h"
#include "concurrentqueue.h"
#include "backoff.h"


struct Threadpool {
	mpmc_bounded_queue<std::function<void()>> queue{128};
	//moodycamel::ConcurrentQueue<std::function<void()>> queue{128};
	std::atomic<bool> running{true};
	std::vector<std::jthread> threads;

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

			//std::cout << "thread exit:" << std::this_thread::get_id() << "\n";
		};

		for (int i=0; i<num_threads; i++){
			threads.emplace_back(work);
		}
	}

	~Threadpool(){
		running.store(false);
	}

	bool enqueue(std::function<void()> task){
		
		while(!queue.enqueue(task)){
			;
		}
		
		return true;
	}

	bool dequeue(std::function<void()>& task){
		return queue.try_dequeue(task);
	}


	//Chaining Implementation
	template<template<typename>typename AWAITABLE, typename T>
	ChainedTask<T> chain_by_value(AWAITABLE<T> awaitable){
		co_return co_await awaitable;
	};

	template<template<typename>typename AWAITABLE, typename T>
	ChainedTask<T> chain_by_reference(AWAITABLE<T>& awaitable){
		co_return co_await awaitable;
	};

	template<template<typename>typename AWAITABLE, typename T>
	auto chain(AWAITABLE<T>&& awaitable){
		return chain_by_value<AWAITABLE, T>(std::forward<AWAITABLE<T>>(awaitable));
	};

	template<template<typename>typename AWAITABLE, typename T>
	auto chain(AWAITABLE<T>& awaitable){
		return chain_by_reference<AWAITABLE, int>(awaitable);
	};



	
	//Branching Implementation	
	template<template<typename>typename AWAITABLE, typename T>
	BranchedTask<T> branch_by_value(AWAITABLE<T> awaitable){
		co_return co_await awaitable;
	};

	template<template<typename>typename AWAITABLE, typename T>
	BranchedTask<T> branch_by_reference(AWAITABLE<T>& awaitable){
		co_return co_await awaitable;
	};

	template<template<typename>typename AWAITABLE, typename T>
	auto branch(AWAITABLE<T>&& awaitable){
		return branch_by_value<AWAITABLE, T>(std::forward<AWAITABLE<T>>(awaitable));
	};

	//template<template<typename>typename AWAITABLE, typename T>
	//auto branch(AWAITABLE<T>& awaitable){
	//	return branch_by_reference<AWAITABLE, T>(awaitable);
	//};


};

#endif