#ifndef FORK_H
#define FORK_H

#include "forkcount.h"
#include "scheduler.h"


template<typename T>
struct [[nodiscard]] Fork {
	struct promise_type {
		T value;
		SchedulerHandle scheduler;
		Forkcount& forkcount;
		std::coroutine_handle<> waiting_handle = std::noop_coroutine();

		template<Scheduler S, typename... A>
		promise_type(S& scheduler, Forkcount& forkcount, A&...)
			:scheduler{scheduler}
			,forkcount{forkcount}
		{}
		

		Fork get_return_object() { 
			return {std::coroutine_handle<promise_type>::from_promise(*this)}; 
		}

		std::suspend_always initial_suspend() noexcept { 
			return {}; 
		}

		struct ResultAwaiter {
			promise_type& promise;

			bool await_ready() noexcept {return false;}

			auto await_suspend (std::coroutine_handle<> handle) noexcept {
				auto maybe_waiting = promise.waiting_handle;
				promise.forkcount.release_and_notify(promise.scheduler).resume();
				return maybe_waiting;
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

		//void* operator new(std::size_t size) noexcept{
		//}

		//void operator delete(void* ptr, std::size_t size) noexcept{
		//}

	};

	//Constructor
	Fork(std::coroutine_handle<promise_type> handle) noexcept 
		:my_handle(handle)
	{} 

	//Copy constructor
	Fork(Fork& t) = delete;

	//Move constructor
	Fork(Fork&& rhs) noexcept 
		:my_handle(rhs.my_handle)
	{ 
		rhs.my_handle = nullptr;
	}

	//Copy assignment
	Fork& operator=(const Fork& t) = delete;
	
	//Move assignment
	Fork& operator=(Fork&& rhs) = delete;

	//Destructor
	~Fork(){
		if (my_handle){
			my_handle.destroy(); 
		}		
	}

	std::coroutine_handle<promise_type> my_handle;

	//Awaiter
	bool await_ready() const noexcept{
		return true;
	}

	void await_suspend(std::coroutine_handle<> caller_handle){
	}

	T await_resume() const noexcept{
		return my_handle.promise().value;
	}

	

};



template<typename T>
using ValueTypeOf = std::remove_reference<T>::type::value_type;

//Chaining Implementation
template<Scheduler S, typename A>
Fork<ValueTypeOf<A>> fork_on_impl(S& scheduler, Forkcount& forkcount, A awaitable){
	co_return co_await awaitable;
};

template<Scheduler S, typename A>
auto create_fork_on(S& scheduler, Forkcount& forkcount, A&& awaitable){
	return fork_on_impl<S, A>(scheduler, forkcount, std::forward<A>(awaitable));
};



template<typename T>
struct [[nodiscard]] ForkAwaiter {
	SchedulerHandle scheduler;
	Fork<T> fork;

	template<Scheduler S, typename A>
	ForkAwaiter(S& scheduler, Forkcount& forkcount, A&& awaitable)
		:scheduler{scheduler}
		,fork{create_fork_on(scheduler, forkcount, std::forward<A>(awaitable))}
	{}
	

	//Awaiter
	bool await_ready() noexcept{
		return false;
	}

	std::coroutine_handle<> await_suspend(std::coroutine_handle<> caller_handle) noexcept{
		auto fork_handle_copy = fork.my_handle;
		//up forkcount before scheduling continuation
		fork_handle_copy.promise().forkcount.up();
		auto waiting_handle = scheduler.schedule(caller_handle);
		//fork variable now invalid;
		fork_handle_copy.promise().waiting_handle = waiting_handle;
		return fork_handle_copy;

	}

	Fork<T> await_resume() noexcept{
		return std::move(fork);
	}

};


template<Scheduler S, typename A>
auto fork_on(S& scheduler, Forkcount& forkcount, A&& awaitable){
	return ForkAwaiter<ValueTypeOf<A>>(scheduler, forkcount, std::forward<A>(awaitable));
};

#endif
