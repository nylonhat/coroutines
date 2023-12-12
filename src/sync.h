#ifndef SYNC_H
#define SYNC_H

#include <variant>
#include <atomic>
#include <coroutine>
#include <thread>

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
struct Sync {

	struct promise_type {
		T value;
		std::atomic_flag flag = ATOMIC_FLAG_INIT;

		Sync get_return_object() { 
			return {std::coroutine_handle<promise_type>::from_promise(*this)}; 
		}

   	 	std::suspend_always initial_suspend() noexcept { return {}; }

		struct ResultAwaiter {
			promise_type& promise;

			bool await_ready() noexcept {return false;}

			std::coroutine_handle<> await_suspend (std::coroutine_handle<> handle) noexcept {
				promise.flag.test_and_set();
				promise.flag.notify_all();
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
	Sync(std::coroutine_handle<promise_type> handle) 
		:my_handle(handle){
	}

	//Copy Constructor
	Sync(Sync& t) = delete;

	//Move Constructor
	Sync(Sync&& rhs) :my_handle(rhs.my_handle) {
		rhs.my_handle = nullptr;
	}

	//Copy Assignment
	Sync& operator=(const Sync& t) = delete;

	//Move Assigment
	Sync& operator=(Sync&& rhs) = delete;
	
	//Destructor
	~Sync(){
		if (my_handle){
			my_handle.destroy(); 
		}		
	}

	void* operator new(std::size_t size);
	void operator delete(void* ptr, std::size_t size);


	std::coroutine_handle<promise_type> my_handle;

	T await() {
		if (my_handle.done()){
			return my_handle.promise().value;		
		}

		my_handle.resume();
		my_handle.promise().flag.wait(false);
		my_handle.promise().flag.clear();
		
		return my_handle.promise().value;
	}

	T operator()(){
		return await();
	}

	operator T(){
		return await();
	}

	bool done(){
		return my_handle.done();
	}


};


template<typename T>
using ValueTypeOf = std::remove_reference<T>::type::value_type;

template<typename A>
Sync<ValueTypeOf<A>> sync_impl(A awaitable){
	co_return co_await awaitable;
};

template<typename A>
auto sync(A&& awaitable){
	return sync_impl<A>(std::forward<A>(awaitable));
};

template<typename A>
auto sync_run(A&& awaitable){
	return sync(std::forward<A>(awaitable)).await();
}


#endif
