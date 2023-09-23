#ifndef CORO_FLAG_H
#define CORO_FLAG_H

#include <atomic>
#include <coroutine>
#include <functional>

template<typename T>
struct CoroFlag {
	mutable std::atomic<void*> head_of_awaiter_list = nullptr;
	std::function<T(void)> result_callback = nullptr;

	CoroFlag(std::function<T(void)> callback) 
		: result_callback{callback}
	{
		
	}

	~CoroFlag(){
		assert(head_of_awaiter_list.load() == nullptr || head_of_awaiter_list.load() == static_cast<void*>(this));
	}
	

	bool is_signalled() const {
		return head_of_awaiter_list.load() == static_cast<const void*>(this);
	}

	void signal_and_notify(std::function<void(std::coroutine_handle<>)> notify_function){
		//signal set by exchanging the head of list with 'set' state
		auto* current_awaiter = static_cast<Awaiter*> (head_of_awaiter_list.exchange(static_cast<void*>(this)) );

		while (current_awaiter != nullptr) {
			// Read m_next before resuming the coroutine as resuming
			// the coroutine will likely destroy the awaiter object.
			auto* temp_next_awaiter = current_awaiter->next_awaiter;

			notify_function(current_awaiter->waiting_handle);

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
			//suspend the current coroutine and wait for value
			//register waiting handle into a linked list of waiting handles

			// Special m_state value that indicates the event is in the 'set' state.
			const void* const set_state = static_cast<const void*>(&flag);

			// Remember the handle of the awaiting coroutine.
			waiting_handle = awaiting_coroutine;

			// Try to atomically push this awaiter onto the front of the list.
			void* current_head = flag.head_of_awaiter_list.load();

			do {
				// Resume immediately if already in 'set' state.
				if (current_head == set_state) {
					return false;
				} 

				// Update linked list to point at current head.
				next_awaiter = static_cast<Awaiter*>(current_head);

				// Finally, try to swap the old list head, inserting this awaiter
				// as the new list head.
			} while (!flag.head_of_awaiter_list.compare_exchange_weak(
						current_head,
						static_cast<void*>(this)
					));

			// Successfully enqueued. Remain suspended.
			return true;
		}

		T await_resume(){
			assert(flag.result_callback != nullptr);
			T result = flag.result_callback();
			return result;
		}

	};

	Awaiter operator co_await() const noexcept{
  		return Awaiter{ *this };
	}

};

#endif