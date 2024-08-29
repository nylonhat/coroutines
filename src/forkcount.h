#ifndef FORKCOUNT_H
#define FORKCOUNT_H

#include <atomic>
#include <coroutine>
#include "scheduler.h"

struct Forkcount {
	std::atomic<int> count = 0;
	std::coroutine_handle<> waiting_handle = std::noop_coroutine();

	void up(){
		count.fetch_add(1);
	}

	std::coroutine_handle<> release_and_notify(Scheduler auto scheduler){
		auto old_count = count.fetch_sub(1);
		if(old_count == 0){
			return scheduler.schedule(waiting_handle);
		}

		return std::noop_coroutine();
	}

	bool all_done(){
		return count.load() == 0;
	}

	void reset(){
		waiting_handle = std::noop_coroutine();
		count.store(0);
	}

	bool join_and_should_suspend(std::coroutine_handle<> maybe_waiting_handle){
		waiting_handle = maybe_waiting_handle;
		auto old_count = count.fetch_sub(1);
		if(old_count == 0){
			waiting_handle = std::noop_coroutine();
			return false;
		}

		return true;
	}

	struct JoinAwaiter {
		Forkcount& forkcount;

		bool await_ready(){
			return forkcount.all_done(); 
		}

		bool await_suspend(std::coroutine_handle<> handle){
			return forkcount.join_and_should_suspend(handle);
		}

		void await_resume(){
			forkcount.reset();		
		}
	};

	auto join(){
		return JoinAwaiter{*this};
	}
	

};

#endif//FORKCOUNT_H
