#ifndef BRANCH_H
#define BRANCH_H

#include "coroutine_flag.h"
#include "scheduler.h"

/**
 * Continuation stealing  
 *
 *         ╭───────── cont' ──────────────┯── caller
 * caller ─┷─ branch ─ task ─ branch ╼ ╌╌╌╯
 *
 *
 *         ╭──── cont' ──╼ ╌╌╌╌╌╌╌╌╌╌╌╮
 * caller ─┷─ branch ─ task ─ branch ─┷────── caller 
 * 
 */

template<typename T>
struct [[nodiscard]] Branch {
	struct promise_type {
		T value{};
		SchedulerHandle scheduler;
		CoroFlag flag;
		std::coroutine_handle<> waiting_handle = std::noop_coroutine();

		template<Scheduler S, typename... A>
		promise_type(S& scheduler, A&...)
			:scheduler{scheduler}
		{
		}
		
		template<typename C, Scheduler S, typename... A>
		promise_type(C& c, S& scheduler, A&...)
			:scheduler{scheduler}
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

			auto await_suspend (std::coroutine_handle<> handle) noexcept {
				auto maybe_waiting = promise.waiting_handle;
				promise.flag.signal_and_notify(promise.scheduler);
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

	std::coroutine_handle<promise_type> my_handle;

	struct FlagAwaiter : CoroFlag::Awaiter{
		T& value;

		T& await_resume(){
			return value;
		}
	}; 

	auto operator co_await() const noexcept{
  		//return my_handle.promise().flag.operator co_await();
		auto& promise = my_handle.promise();
		return FlagAwaiter{{promise.flag}, {promise.value}};
	}

	bool done() noexcept{
		return my_handle.promise().flag.is_signalled();
	}

};



template<typename T>
using ValueTypeOf = std::remove_reference<T>::type::value_type;

//Chaining Implementation
template<Scheduler S, typename A>
Branch<ValueTypeOf<A>> branch_on_impl(S& scheduler, A awaitable){
	co_return co_await awaitable;
};

template<Scheduler S, typename A>
auto create_branch_on(S& scheduler, A&& awaitable){
	return branch_on_impl<S, A>(scheduler, std::forward<A>(awaitable));
};



template<typename T>
struct [[nodiscard]] BranchAwaiter {
	SchedulerHandle scheduler;
	Branch<T> branch;

	template<Scheduler S, typename A>
	BranchAwaiter(S& scheduler, A&& awaitable)
		:scheduler{scheduler}
		,branch{create_branch_on(scheduler, std::forward<A>(awaitable))}
	{}
	
	template<Scheduler S>
	BranchAwaiter(S& scheduler, Branch<T>&& branch)
		:scheduler{scheduler}
		,branch{std::forward<Branch<T>>(branch)}
	{}


	//Awaiter
	bool await_ready() noexcept{
		return false;
	}

	//noinline attribute as workaround for miscompilation bug in clang.
	//https://github.com/llvm/llvm-project/issues/56301
	//results in correct but slower code! (still faster than gcc)
	//__attribute__((noinline))
	std::coroutine_handle<> await_suspend(std::coroutine_handle<> caller_handle) noexcept{
		auto branch_handle_copy = branch.my_handle;
		auto waiting_handle = scheduler.schedule(caller_handle);
		//branch variable now invalid;
		branch_handle_copy.promise().waiting_handle = waiting_handle;;
		return branch_handle_copy;

	}

	Branch<T> await_resume() noexcept{
		return std::move(branch);
	}

};


template<Scheduler S, typename A>
auto branch_on(S& scheduler, A&& awaitable){
	return BranchAwaiter<ValueTypeOf<A>>(scheduler, std::forward<A>(awaitable));
};

template<Scheduler S, typename T>
auto branch_on(S& scheduler, Branch<T>&& branch){
	return BranchAwaiter<T>(scheduler, std::forward<Branch<T>>(branch));
};
#endif
