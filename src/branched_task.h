#ifndef BRANCHED_TASK_H
#define BRANCHED_TASK_H

#include "coroutine_flag.h"
#include "scheduler.h"

template<typename T, Scheduler S>
struct [[nodiscard]] BranchedTask {
	struct promise_type {
		T value;
		S& scheduler;
		CoroFlag<T> flag;

		template<template<typename>typename AWAITABLE>
		promise_type(S& tp, AWAITABLE<T> &awaitable)
			:scheduler{tp}, 
			flag{[this](){return value;}}{
		}

		BranchedTask get_return_object() { 
			return {std::coroutine_handle<promise_type>::from_promise(*this)}; 
		}

		struct InitialAwaiter {
			promise_type& promise;

			bool await_ready() noexcept {return false;}

			void await_suspend (std::coroutine_handle<> handle) noexcept {

				promise.scheduler.schedule([handle](){
					handle.resume();
				});
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
				promise.flag.signal_and_notify([this](std::coroutine_handle<> resuming_handle){
					promise.scheduler.schedule([resuming_handle](){
						resuming_handle.resume();
					});

				});
				
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
	BranchedTask(std::coroutine_handle<promise_type> handle) 
		:my_handle(handle)
	{} 

	//Copy constructor
	BranchedTask(BranchedTask& t) = delete;

	//Move constructor
	BranchedTask(BranchedTask&& rhs) : my_handle(rhs.my_handle) { 
		rhs.my_handle = nullptr;
	}

	//Copy assignment
	BranchedTask& operator=(const BranchedTask& t) = delete;
	
	//Move assignment
	BranchedTask& operator=(BranchedTask&& rhs) = delete;

	//Destructor
	~BranchedTask(){
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

};

//Branching Implementation
template<Scheduler S, template<typename>typename AWAITABLE, typename T>
BranchedTask<T, S> branch_by_value_on(S& scheduler, AWAITABLE<T> awaitable){
	co_return co_await awaitable;
};

template<Scheduler S, template<typename>typename AWAITABLE, typename T>
BranchedTask<T, S> branch_by_reference_on(S& scheduler, AWAITABLE<T>& awaitable){
	co_return co_await awaitable;
};


#endif
