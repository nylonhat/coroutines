#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <atomic>
#include <thread>
#include <functional>
#include <utility>

//#include "bounded_mpmc_queue.h"
#include "chained_task.h"
#include "branched_task.h"
#include "static_bounded_queue.h"

struct Threadpool {
	bounded_mpmc_queue<std::function<void()>, 128> queue{};
	std::atomic<bool> running{true};
	std::vector<std::jthread> threads;
	
	//Constructor
	Threadpool(int num_threads);

	//Destructor
	~Threadpool();
	
	
	void schedule(std::function<void()> task);

	bool dequeue(std::function<void()>& task);


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
