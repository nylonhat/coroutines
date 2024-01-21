#ifndef SPAWN_H
#define SPAWN_H

#include "coroutine_flag.h"
#include "scheduler.h"


/**
 * Child stealing  
 *
 *         ╭─ spawn ─ task ─ spawn ─╼ ╌╌╌╌╮
 * caller ─┷──────── cont' ───────────────┷── caller 
 *
 *         ╭─ spawn ─ task ─ spawn ───┯────── caller
 * caller ─┷────── cont' ─────╼ ╌╌╌╌╌╌╯
 *
 *
 */


template<typename T>
struct [[nodiscard]] Spawn {
	struct promise_type {
		T value{};
		SchedulerHandle scheduler;
		CoroFlag<T> flag;

		template<Scheduler S, typename A>
		promise_type(S& scheduler, A &awaitable)
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



template<typename T>
using ValueTypeOf = std::remove_reference<T>::type::value_type;

template<Scheduler S, typename A>
Spawn<ValueTypeOf<A>> spawn_on_impl(S& scheduler, A awaitable){
	co_return co_await awaitable;
};

template<Scheduler S, typename A>
auto spawn_on(S& scheduler, A&& awaitable){
	return spawn_on_impl<S, A>(scheduler, std::forward<A>(awaitable));
};



#endif
