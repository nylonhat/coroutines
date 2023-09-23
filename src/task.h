#ifndef TASK_H
#define TASK_H

#include <variant>

template<typename T>
struct Task {

	struct promise_type {
		T value;
		std::variant<std::monostate, std::coroutine_handle<>> waiting_handle;

		Task get_return_object() { 
			return {std::coroutine_handle<promise_type>::from_promise(*this)}; 
		}

   	 	std::suspend_always initial_suspend() noexcept { return {}; }

		struct ResultAwaiter {
			promise_type& promise;

			bool await_ready() noexcept {return false;}

			std::coroutine_handle<> await_suspend (std::coroutine_handle<> handle) noexcept {
				return std::get<1>(promise.waiting_handle);
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
	Task(std::coroutine_handle<promise_type> handle) :my_handle(handle){}

	//Copy Constructor
	Task(Task& t) = delete;

	//Move Constructor
	Task(Task&& rhs) : my_handle(rhs.my_handle) { 
		rhs.my_handle = nullptr; 
	}

	//Copy Assignment
	Task& operator=(const Task& t) = delete;

	//Move Assigment
	Task& operator=(Task&& rhs) = delete;

	//Destructor
	~Task(){
		if (my_handle){
			my_handle.destroy(); 
		}		
	}

	void* operator new(std::size_t size);
	void operator delete(void* ptr, std::size_t size);

	std::coroutine_handle<promise_type> my_handle;

	bool done(){
		return my_handle.done();
	}
	
	//Awaiter
	bool await_ready(){
		return my_handle.done();
	}

	std::coroutine_handle<> await_suspend(std::coroutine_handle<> caller_handle) noexcept{
		my_handle.promise().waiting_handle = caller_handle;
		return my_handle;
	}

	T await_resume(){
		return my_handle.promise().value;
	}

};

#endif