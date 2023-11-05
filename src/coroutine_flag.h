#ifndef CORO_FLAG_H
#define CORO_FLAG_H

#include <atomic>
#include <coroutine>
#include <functional>
#include "scheduler.h"

template<typename T>
struct CoroFlag {
	mutable std::atomic<void*> head_of_awaiter_list = nullptr;
	T& result_reference;

	CoroFlag(T& result) 
		: result_reference{result}	{
	}

	~CoroFlag(){
		assert(head_of_awaiter_list.load() == nullptr || head_of_awaiter_list.load() == static_cast<void*>(this));
	}
	

	bool is_signalled() const {
		return head_of_awaiter_list.load() == static_cast<const void*>(this);
	}

	template<Scheduler S>
	void signal_and_notify(S& scheduler){
		//Signal set by exchanging the head of list with 'set' state
		auto* current_awaiter = static_cast<Awaiter*> (head_of_awaiter_list.exchange(static_cast<void*>(this)) );
		
		//Notify by walking through linked list
		//Scheduling all awaiters in the list
		while (current_awaiter != nullptr) {
			//keep a temporary copy of next awaiter since it may be destroyed after awaiter is resumed
			auto* temp_next_awaiter = current_awaiter->next_awaiter;
			
			//Provide a custom scheduling function
			scheduler.schedule(current_awaiter->waiting_handle);

			current_awaiter = temp_next_awaiter;
		}
	}

	struct Awaiter {
		const CoroFlag& flag;
		Awaiter* next_awaiter = nullptr;
		std::coroutine_handle<> waiting_handle;

		bool await_ready(){
			return flag.is_signalled();
		}

		bool await_suspend(std::coroutine_handle<> awaiting_coroutine){
			//Suspend the current coroutine

			// Set state means that result is ready
			const void* const set_state = static_cast<const void*>(&flag);

			// Remember the handle of the awaiting coroutine.
			waiting_handle = awaiting_coroutine;

			// Try to atomically push this awaiter onto the front of the list.
			void* current_head = flag.head_of_awaiter_list.load();

			do {
				// Resume immediately if already in set  state.
				if (current_head == set_state) {
					return false;
				} 

				// Update linked list to point at current head.
				next_awaiter = static_cast<Awaiter*>(current_head);

				// Try to swap the old list head, inserting this awaiter
				// as the new list head.
			} while (!flag.head_of_awaiter_list.compare_exchange_weak(current_head, static_cast<void*>(this)));

			// Successfully enqueued. Remain suspended.
			return true;
		}

		T await_resume(){
			return flag.result_reference;
		}

	};

	Awaiter operator co_await() const noexcept{
  		return Awaiter{*this};
	}

};

#endif
