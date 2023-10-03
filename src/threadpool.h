#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <atomic>
#include <thread>
#include <functional>
#include <utility>

#include "chained_task.h"
#include "branched_task.h"
#include "bounded_mpmc_queue.h"

/**
 * A threadpool is a type of scheduler that has a queue
 * of work that needs to be done. A fixed pool of worker
 * threads try to pull work from the queue in a first in 
 * first out (FIFO) fashion and execute them. This 
 * implementation uses exponential backoff with jitter to
 * try and reduce contention between the worker threads, 
 * and also allow threads to go idle if the work load is
 * low. 
 *
 * This threadpool supports the chaining and branching of 
 * coroutines onto it.  
 */

struct Threadpool {
private:
	bounded_mpmc_queue<std::function<void()>, 128> queue{};
	std::atomic<bool> running{true};
	std::vector<std::jthread> threads;

public:
	//Constructor
	Threadpool(int num_threads);

	//Destructor
	~Threadpool();
	
private:	
	bool dequeue(std::function<void()>& task);

public:
	bool schedule(std::function<void()> task);
	
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
