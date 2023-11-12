#ifndef FORKED_TASK_H
#define FORKED_TASK_H

#include "coroutine_flag.h"
#include "scheduler.h"

template<typename T, Scheduler S>
struct [[nodiscard]] ForkedTask {
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

		ForkedTask get_return_object() { 
			return {std::coroutine_handle<promise_type>::from_promise(*this)}; 
		}

		std::suspend_always initial_suspend() noexcept { 
			return {}; 
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
	ForkedTask(std::coroutine_handle<promise_type> handle) noexcept 
		:my_handle(handle)
	{} 

	//Copy constructor
	ForkedTask(ForkedTask& t) = delete;

	//Move constructor
	ForkedTask(ForkedTask&& rhs) noexcept 
		:my_handle(rhs.my_handle)
	{ 
		rhs.my_handle = nullptr;
	}

	//Copy assignment
	ForkedTask& operator=(const ForkedTask& t) = delete;
	
	//Move assignment
	ForkedTask& operator=(ForkedTask&& rhs) = delete;

	//Destructor
	~ForkedTask(){
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
ForkedTask<T, S> create_fork_by_value_on(S& scheduler, AWAITABLE<T> awaitable){
	co_return co_await awaitable;
};

template<Scheduler S, template<typename>typename AWAITABLE, typename T>
ForkedTask<T, S> create_fork_by_reference_on(S& scheduler, AWAITABLE<T>& awaitable){
	co_return co_await awaitable;
};

template<Scheduler S, template<typename>typename AWAITABLE, typename T>
ForkedTask<T, S> create_fork_on(S& scheduler, AWAITABLE<T>&& awaitable){
	return create_fork_by_value_on(scheduler, std::move(awaitable));
};

template<Scheduler S, template<typename>typename AWAITABLE, typename T>
ForkedTask<T, S> create_fork_on(S& scheduler, AWAITABLE<T>& awaitable){
	return create_fork_by_reference_on(scheduler, awaitable);
};


#endif
