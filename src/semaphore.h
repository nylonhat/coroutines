#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <atomic>
#include <coroutine>
#include "scheduler.h"

struct Semaphore{
	std::atomic<int> count = 0;
	std::coroutine_handle<> waiting_handle = std::noop_coroutine();
	const int max = 0;

	Semaphore(int max)
		: count{max}
		, max{max}
	{}

	std::coroutine_handle<> release_and_notify(Scheduler auto scheduler){
		auto old_count = count.fetch_add(1);
		if(old_count == -1 || old_count == max){
			return scheduler.schedule(waiting_handle);
		}
		return std::noop_coroutine();
	}

	bool try_acquire(){
		if (count.load() > 0){
			count--;
			return true;
		}
		return false;
	}

	bool acquire_and_should_suspend(std::coroutine_handle<> may_wait_handle){
		waiting_handle = may_wait_handle;
		auto old_count = count.fetch_sub(1);
		if(old_count > 0){
			waiting_handle = std::noop_coroutine();
			return false;
		}
		return true;
	}

	void reset(){
		waiting_handle = std::noop_coroutine();
		count.store(max);
	}

	struct AcquireAwaiter {
		Semaphore& semaphore;

		bool await_ready(){
			return semaphore.try_acquire(); 
		}

		bool await_suspend(std::coroutine_handle<> handle){
			return semaphore.acquire_and_should_suspend(handle);
		}

		void await_resume(){}
	};

	auto acquire(){
		return AcquireAwaiter{*this};
	}

	bool all_done(){
		return count.load() == max;
	}

	bool join_and_should_suspend(std::coroutine_handle<> may_wait_handle){
		auto old_count = count.fetch_add(1);
		waiting_handle = may_wait_handle;
		if(old_count == max){
			waiting_handle = std::noop_coroutine();
			return false;	
		}

		return true;
		
	} 

	struct JoinAwaiter {
		Semaphore& semaphore;

		bool await_ready(){
			return semaphore.all_done(); 
		}

		bool await_suspend(std::coroutine_handle<> handle){
			return semaphore.join_and_should_suspend(handle);
		}

		void await_resume(){
			semaphore.reset();		
		}
	}; 

	auto join(){
		return JoinAwaiter{*this};
	} 

};

#endif//SEMAPHORE_H
