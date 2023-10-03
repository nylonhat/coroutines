#ifndef CHAINED_TASK_H
#define CHAINED_TASK_H

#include <coroutine>
#include <variant>

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
struct ChainedTask {
	struct promise_type {
		T value;
		std::variant<std::monostate, std::coroutine_handle<>> waiting_handle;
		S& scheduler;

		template<template<typename>typename AWAITABLE>
		promise_type(S& scheduler, AWAITABLE<T>& awaitable)
			:scheduler{scheduler}{
		}

		ChainedTask get_return_object() { 
			return {std::coroutine_handle<promise_type>::from_promise(*this)}; 
		}

   	 	std::suspend_always initial_suspend() noexcept { return {}; }

		struct ResultAwaiter {
			promise_type& promise;

			bool await_ready() noexcept {return false;}

			std::coroutine_handle<> await_suspend (std::coroutine_handle<> handle) noexcept {

				bool was_scheduled = promise.scheduler.schedule([this](){
					std::get<1>(promise.waiting_handle).resume();
				});
				
				if(was_scheduled){
					return std::noop_coroutine();
				}
				
				//If can't schedule, default to symmetric transfer
				return std::get<1>(promise.waiting_handle).resume();


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
	ChainedTask(std::coroutine_handle<promise_type> handle) 
		:my_handle(handle){
	} 

	//Copy constructor
	ChainedTask(ChainedTask& t) = delete;

	//Move constructor
	ChainedTask(ChainedTask&& rhs) : my_handle(rhs.my_handle){
		rhs.my_handle = nullptr;
	}

	//Copy assignment
	ChainedTask& operator=(const ChainedTask& t) = delete;
	
	//Move assignment
	ChainedTask& operator=(ChainedTask&& rhs) = delete;

	//Destructor
	~ChainedTask(){
		if (my_handle){
			my_handle.destroy();
		}		
	}

	//No heap allocations - Heap Elision Optimisation 
	void* operator new(std::size_t size);
	void operator delete(void* ptr, std::size_t size);


	std::coroutine_handle<promise_type> my_handle;


	//Awaiter
	bool await_ready(){
		return my_handle.done();
	}

	std::coroutine_handle<> await_suspend(std::coroutine_handle<> caller_handle) noexcept{
		my_handle.promise().waiting_handle = caller_handle;
		
		//Try to schedule using scheduler
		bool was_scheduled = my_handle.promise().scheduler.schedule([this](){
			my_handle.resume();
		});

		
		if(was_scheduled){
			return std::noop_coroutine();
		}

		//If can't schedule, default to running symmetric transfer
		return my_handle;
		
	}

	T await_resume(){
		return my_handle.promise().value;
	}

};


//Chaining Implementation
template<Scheduler S, template<typename>typename AWAITABLE, typename T>
ChainedTask<T, S> chain_by_value_on(S& scheduler, AWAITABLE<T> awaitable){
	co_return co_await awaitable;
};

template<Scheduler S, template<typename>typename AWAITABLE, typename T>
ChainedTask<T, S> chain_by_reference_on(S& scheduler, AWAITABLE<T>& awaitable){
	co_return co_await awaitable;
};



#endif
