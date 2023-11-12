#ifndef SPAWN_H
#define SPAWN_H

#include "coroutine_flag.h"
#include "scheduler.h"

template<typename T, Scheduler S>
struct [[nodiscard]] Spawn {
	struct promise_type {
		T value{};
		S& scheduler;
		CoroFlag<T> flag;

		template<template<typename>typename AWAITABLE>
		promise_type(S& scheduler, AWAITABLE<T> &awaitable)
			:scheduler{scheduler}
			,flag(value)
		{
		}

		Spawn get_return_object() { 
			return {std::coroutine_handle<promise_type>::from_promise(*this)}; 
		}

		struct InitialAwaiter {
			promise_type& promise;

			bool await_ready() noexcept {return false;}

			std::coroutine_handle<> await_suspend (std::coroutine_handle<> handle) noexcept {
				return promise.scheduler.schedule(handle);
			}

			void await_resume() noexcept {}	
		};


   	 	InitialAwaiter initial_suspend() noexcept { 
			return InitialAwaiter{*this}; 
		}

		struct ResultAwaiter {
			promise_type& promise;

			bool await_ready() noexcept {return false;}

			void await_suspend (std::coroutine_handle<> handle) noexcept {
				promise.flag.signal_and_notify(promise.scheduler);
			}

			void await_resume() noexcept {}	

		};

    	
		ResultAwaiter yield_value(T yield_value){
			value = yield_value;
			return ResultAwaiter{*this};	
		}

		void return_value(T return_value){
			value = return_value;
		}
		
		ResultAwaiter final_suspend() noexcept {
			return ResultAwaiter{*this}; 
		}

		void unhandled_exception() {}

	};

	//Constructor
	Spawn(std::coroutine_handle<promise_type> handle) noexcept 
		:my_handle(handle)
	{} 

	//Copy constructor
	Spawn(Spawn& t) = delete;

	//Move constructor
	Spawn(Spawn&& rhs) noexcept 
		:my_handle(rhs.my_handle)
	{ 
		rhs.my_handle = nullptr;
	}

	//Copy assignment
	Spawn& operator=(const Spawn& t) = delete;
	
	//Move assignment
	Spawn& operator=(Spawn&& rhs) = delete;

	//Destructor
	~Spawn(){
		if (my_handle){
			assert(my_handle.promise().flag.is_signalled() == true);
			my_handle.destroy(); 
		}		
	}

	//No heap allocations - Heap Elision Optimisation 
	void* operator new(std::size_t size);
	void operator delete(void* ptr, std::size_t size);

	std::coroutine_handle<promise_type> my_handle;

	auto operator co_await() const noexcept{
  		return my_handle.promise().flag.operator co_await();
	}

	bool done() noexcept{
		return my_handle.promise().flag.is_signalled();
	}

};

//Branching Implementation
template<Scheduler S, template<typename>typename AWAITABLE, typename T>
Spawn<T, S> spawn_by_value_on(S& scheduler, AWAITABLE<T> awaitable){
	co_return co_await awaitable;
};

template<Scheduler S, template<typename>typename AWAITABLE, typename T>
Spawn<T, S> spawn_by_reference_on(S& scheduler, AWAITABLE<T>& awaitable){
	co_return co_await awaitable;
};

template<Scheduler S, template<typename>typename AWAITABLE, typename T>
Spawn<T, S> spawn_on(S& scheduler, AWAITABLE<T>&& awaitable){
	return spawn_by_value_on(scheduler, std::move(awaitable));
};

template<Scheduler S, template<typename>typename AWAITABLE, typename T>
Spawn<T, S> spawn_on(S& scheduler, AWAITABLE<T>& awaitable){
	return spawn_by_reference_on(scheduler, awaitable);
};


#endif
