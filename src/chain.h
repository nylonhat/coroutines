#ifndef CHAIN_H
#define CHAIN_H

#include <coroutine>
#include "scheduler.h"

/**
 * A chained task represents a task responsible for executing another
 * task asynchronously on a different execution context, dictated 
 * by a scheduler.
 *
 * Chaining is an example of asynchronous execution: One coroutine will
 * susupend itself first before transferring control to another coroutine.
 * Once that coroutine has finished, it will suspend itself and return
 * execution back to the original coroutine. At no point are these 
 * coroutines running at the same time; hence there is no need for thread
 * synchronisation. 
 *
 * Caller Coroutine -> Chained Task -> Scheduler -> Chained Task -> Task 
 * Caller Coroutine <- Chained Task <- Scheduler <- Chained Task <- Task 
 */

template<typename T, Scheduler S>
struct Chain {
	struct promise_type {
		T value;
		std::coroutine_handle<> waiting_handle = std::noop_coroutine();
		S& scheduler;

		template<template<typename>typename AWAITABLE>
		promise_type(S& scheduler, AWAITABLE<T>& awaitable)
			:scheduler{scheduler}{
		}

		Chain get_return_object() { 
			return {std::coroutine_handle<promise_type>::from_promise(*this)}; 
		}

   	 	std::suspend_always initial_suspend() noexcept { return {}; }

		struct ResultAwaiter {
			promise_type& promise;

			bool await_ready() noexcept {return false;}

			std::coroutine_handle<> await_suspend (std::coroutine_handle<> handle) noexcept {
				return promise.scheduler.schedule(promise.waiting_handle);
			}

			void await_resume() noexcept {}	
		};

		ResultAwaiter yield_value(T yield_value) noexcept{
			value = yield_value;
			return ResultAwaiter{*this};	
		}

		void return_value(T return_value) noexcept{
			value = return_value;
			
		}
		
		ResultAwaiter final_suspend() noexcept {
			return ResultAwaiter{*this}; 
		}

		void unhandled_exception() {}

	};

	//Constructor
	Chain(std::coroutine_handle<promise_type> handle) noexcept 
		:my_handle(handle){
	} 

	//Copy constructor
	Chain(Chain& t) = delete;

	//Move constructor
	Chain(Chain&& rhs) noexcept 
		:my_handle(rhs.my_handle)
	{
		rhs.my_handle = nullptr;
	}

	//Copy assignment
	Chain& operator=(const Chain& t) = delete;
	
	//Move assignment
	Chain& operator=(Chain&& rhs) = delete;

	//Destructor
	~Chain(){
		if (my_handle){
			my_handle.destroy();
		}		
	}

	//No heap allocations - Heap Elision Optimisation 
	void* operator new(std::size_t size);
	void operator delete(void* ptr, std::size_t size);


	std::coroutine_handle<promise_type> my_handle;


	//Awaiter
	bool await_ready() noexcept{
		return my_handle.done();
	}

	std::coroutine_handle<> await_suspend(std::coroutine_handle<> caller_handle) noexcept{
		my_handle.promise().waiting_handle = caller_handle;
		return my_handle.promise().scheduler.schedule(my_handle);
	}

	T await_resume() noexcept{
		return my_handle.promise().value;
	}

};


//Chaining Implementation
template<Scheduler S, template<typename>typename AWAITABLE, typename T>
Chain<T, S> chain_by_value_on(S& scheduler, AWAITABLE<T> awaitable){
	co_return co_await awaitable;
};

template<Scheduler S, template<typename>typename AWAITABLE, typename T>
Chain<T, S> chain_by_reference_on(S& scheduler, AWAITABLE<T>& awaitable){
	co_return co_await awaitable;
};

template<Scheduler S, template<typename>typename AWAITABLE, typename T>
Chain<T, S> chain_on(S& scheduler, AWAITABLE<T>&& awaitable){
	return chain_by_value_on(scheduler, std::move(awaitable));
};

template<Scheduler S, template<typename>typename AWAITABLE, typename T>
Chain<T, S> chain_on(S& scheduler, AWAITABLE<T>& awaitable){
	return chain_by_reference_on(scheduler, awaitable);
};


#endif
