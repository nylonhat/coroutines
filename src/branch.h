#ifndef BRANCH_H
#define BRANCH_H

#include "coroutine_flag.h"
#include "scheduler.h"
#include <iostream>

template<typename T, Scheduler S>
struct [[nodiscard]] Branch {
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

		Branch get_return_object() { 
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
	Branch(std::coroutine_handle<promise_type> handle) noexcept 
		:my_handle(handle)
	{} 

	//Copy constructor
	Branch(Branch& t) = delete;

	//Move constructor
	Branch(Branch&& rhs) noexcept 
		:my_handle(rhs.my_handle)
	{ 
		rhs.my_handle = nullptr;
	}

	//Copy assignment
	Branch& operator=(const Branch& t) = delete;
	
	//Move assignment
	Branch& operator=(Branch&& rhs) = delete;

	//Destructor
	~Branch(){
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
Branch<T, S> branch_by_value_on(S& scheduler, AWAITABLE<T> awaitable){
	co_return co_await awaitable;
};

template<Scheduler S, template<typename>typename AWAITABLE, typename T>
Branch<T, S> branch_by_reference_on(S& scheduler, AWAITABLE<T>& awaitable){
	co_return co_await awaitable;
};

template<Scheduler S, template<typename>typename AWAITABLE, typename T>
Branch<T, S> create_branch_on(S& scheduler, AWAITABLE<T>&& awaitable){
	return branch_by_value_on(scheduler, std::move(awaitable));
};

template<Scheduler S, template<typename>typename AWAITABLE, typename T>
Branch<T, S> create_branch_on(S& scheduler, AWAITABLE<T>& awaitable){
	return branch_by_reference_on(scheduler, awaitable);
};



template<typename T, Scheduler S, template<typename>typename A>
struct [[nodiscard]] BranchAwaiter {
	S& scheduler;
	Branch<T, S> branch;

	BranchAwaiter(S& scheduler, A<T>&& awaitable)
		:scheduler{scheduler}
		,branch{create_branch_on(scheduler, std::move(awaitable))}
	{
	}
	
	BranchAwaiter(S& scheduler, A<T>& awaitable)
		:scheduler{scheduler}
		,branch{create_branch_on(scheduler, awaitable)}
	{}

	//Awaiter
	bool await_ready() noexcept{
		return false;
	}

	std::coroutine_handle<> await_suspend(std::coroutine_handle<> caller_handle) noexcept{
		std::coroutine_handle<> branch_handle_copy = branch.my_handle;
		if(scheduler.schedule(caller_handle) != caller_handle){
			return branch_handle_copy;
		}

		branch_handle_copy.resume();
		return caller_handle;
	}

	Branch<T,S> await_resume() noexcept{
		return std::move(branch);
	}

};

//Forking Implementation
template<Scheduler S, typename A>
auto branch_on(S& scheduler, A&& awaitable){
	return BranchAwaiter(scheduler, std::forward<A>(awaitable));
};

#endif
