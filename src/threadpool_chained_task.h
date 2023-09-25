#ifndef THREADPOOL_CHAINED_TASK_H
#define THREADPOOL_CHAINED_TASK_H

struct Threadpool;

template<typename T>
struct ChainedTask {
	struct promise_type {

		T value;
		std::variant<std::monostate, std::coroutine_handle<>> waiting_handle;
		Threadpool& threadpool;

		template<template<typename>typename AWAITABLE>
		promise_type(Threadpool& tp, AWAITABLE<T>& awaitable)
			:threadpool{tp}{}

		ChainedTask get_return_object() { 
			return {std::coroutine_handle<promise_type>::from_promise(*this)}; 
		}

   	 	std::suspend_always initial_suspend() noexcept { return {}; }

		struct ResultAwaiter {
			promise_type& promise;

			bool await_ready() noexcept {return false;}

			void await_suspend (std::coroutine_handle<> handle) noexcept {

				promise.threadpool.schedule([this](){
					std::get<1>(promise.waiting_handle).resume();
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
	ChainedTask(std::coroutine_handle<promise_type> handle) 
		:my_handle(handle){
	} 

	//Copy constructor
	ChainedTask(ChainedTask& t) = delete;

	//Move constructor
	ChainedTask(ChainedTask&& rhs) : my_handle(rhs.my_handle){
		rhs.my_handle = nullptr;
	}

	//Copy assignment
	ChainedTask& operator=(const ChainedTask& t) = delete;
	
	//Move assignment
	ChainedTask& operator=(ChainedTask&& rhs) = delete;

	//Destructor
	~ChainedTask(){
		if (my_handle){
			my_handle.destroy();
		}		
	}

	//No heap allocations - Heap Elision Optimisation 
	void* operator new(std::size_t size);
	void operator delete(void* ptr, std::size_t size);


	std::coroutine_handle<promise_type> my_handle;


	//Awaiter
	bool await_ready(){
		return my_handle.done();
	}

	void await_suspend(std::coroutine_handle<> caller_handle) noexcept{
		my_handle.promise().waiting_handle = caller_handle;
	
		my_handle.promise().threadpool.schedule([this](){
			my_handle.resume();
		});
		
	}

	T await_resume(){
		return my_handle.promise().value;
	}

};




#endif
