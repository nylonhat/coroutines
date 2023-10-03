#ifndef BLOCKING_TASK_H
#define BLOCKING_TASK_H

#include <variant>
#include <atomic>
#include <coroutine>

#include "atomic_flag.h"

/**
 * A blocking task is a coroutine whose only purpose is
 * to interact with the main() function. Since main() is 
 * not a coroutine (its just a regular function), we can't
 * use the co_await keywork in it. A blocking task will
 * block the main thread until it is ready to return a value.
 *
 * The blocking task will essentially act as our main coroutine
 * function. Since it is a coroutine, we can now co_await other
 * coroutines inside its body. 
 */

template<typename T>
struct BlockingTask {

	struct promise_type {
		T value;
		AtomicFlag flag;

		BlockingTask get_return_object() { 
			return {std::coroutine_handle<promise_type>::from_promise(*this)}; 
		}

   	 	std::suspend_always initial_suspend() noexcept { return {}; }

		struct ResultAwaiter {
			promise_type& promise;

			bool await_ready() noexcept {return false;}

			std::coroutine_handle<> await_suspend (std::coroutine_handle<> handle) noexcept {
				promise.flag.signal();

				return std::noop_coroutine();
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
	BlockingTask(std::coroutine_handle<promise_type> handle) 
		:my_handle(handle){
	}

	//Copy Constructor
	BlockingTask(BlockingTask& t) = delete;

	//Move Constructor
	BlockingTask(BlockingTask&& rhs) :my_handle(rhs.my_handle) {
		rhs.my_handle = nullptr;
	}

	//Copy Assignment
	BlockingTask& operator=(const BlockingTask& t) = delete;

	//Move Assigment
	BlockingTask& operator=(BlockingTask&& rhs) = delete;

	~BlockingTask(){
		if (my_handle){
			my_handle.destroy(); 
		}		
	}

	void* operator new(std::size_t size);
	void operator delete(void* ptr, std::size_t size);


	std::coroutine_handle<promise_type> my_handle;

	T block_and_get() {
		if (my_handle.done()){
			return my_handle.promise().value;		
		}

		my_handle.resume();

		my_handle.promise().flag.wait();
		my_handle.promise().flag.reset();
		
		return my_handle.promise().value;
	}

	T operator()(){
		return block_and_get();
	}

	operator T(){
		return block_and_get();
	}

	bool done(){
		return my_handle.done();
	}


};

#endif
