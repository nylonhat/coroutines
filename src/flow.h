#ifndef FLOW_H
#define FLOW_H

#include "semaphore.h"
#include "scheduler.h"


template<typename T>
struct [[nodiscard]] Flow {
	struct promise_type {
		T value{};
		SchedulerHandle scheduler;
		Semaphore& semaphore;
		std::coroutine_handle<> waiting_handle = std::noop_coroutine();
		std::atomic<bool> done = false;

		template<Scheduler S, typename... A>
		promise_type(S& scheduler, Semaphore& semaphore, A&...)
			:scheduler{scheduler}
			,semaphore{semaphore}
		{}
		

		Flow get_return_object() { 
			return {std::coroutine_handle<promise_type>::from_promise(*this)}; 
		}

		std::suspend_always initial_suspend() noexcept { 
			return {}; 
		}

		struct ResultAwaiter {
			promise_type& promise;

			bool await_ready() noexcept {return false;}

			auto await_suspend (std::coroutine_handle<> handle) noexcept {
				///??? TODO
				promise.done.store(true);
				promise.semaphore.release_and_notify(promise.scheduler).resume();
				return promise.waiting_handle;
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
	Flow(std::coroutine_handle<promise_type> handle) noexcept 
		:my_handle(handle)
	{} 

	//Copy constructor
	Flow(Flow& t) = delete;

	//Move constructor
	Flow(Flow&& rhs) noexcept 
		:my_handle(rhs.my_handle)
	{ 
		rhs.my_handle = nullptr;
	}

	//Copy assignment
	Flow& operator=(const Flow& t) = delete;
	
	//Move assignment
	Flow& operator=(Flow&& rhs) = delete;

	//Destructor
	~Flow(){
		if (my_handle){
			my_handle.destroy(); 
		}		
	}

	std::coroutine_handle<promise_type> my_handle;

	bool done(){
		return my_handle.promise().done.load();
	}

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
Flow<ValueTypeOf<A>> flow_on_impl(S& scheduler, Semaphore& semaphore, A awaitable){
	co_return co_await awaitable;
};

template<Scheduler S, typename A>
auto create_flow_on(S& scheduler, Semaphore& semaphore, A&& awaitable){
	return flow_on_impl<S, A>(scheduler, semaphore, std::forward<A>(awaitable));
};



template<typename T>
struct [[nodiscard]] FlowAwaiter {
	SchedulerHandle scheduler;
	Flow<T> flow;

	template<Scheduler S, typename A>
	FlowAwaiter(S& scheduler, Semaphore& semaphore, A&& awaitable)
		:scheduler{scheduler}
		,flow{create_flow_on(scheduler, semaphore, std::forward<A>(awaitable))}
	{}
	

	//Awaiter
	bool await_ready() noexcept{
		return false;
	}

	std::coroutine_handle<> await_suspend(std::coroutine_handle<> caller_handle) noexcept{
		auto flow_handle_copy = flow.my_handle;
		//up semaphore before scheduling continuation
		auto waiting_handle = scheduler.schedule(caller_handle);
		//flow variable now invalid;
		flow_handle_copy.promise().waiting_handle = waiting_handle;
		return flow_handle_copy;

	}

	Flow<T> await_resume() noexcept{
		return std::move(flow);
	}

};


template<Scheduler S, typename A>
auto flow_on(S& scheduler, Semaphore& semaphore, A&& awaitable){
	return FlowAwaiter<ValueTypeOf<A>>(scheduler, semaphore, std::forward<A>(awaitable));
};

#endif
