#ifndef FORKED_AWAITER_H
#define FORKED_AWAITER_H

#include "scheduler.h"
#include "forked_task.h"

template<typename T, Scheduler S, template<typename>typename A>
struct [[nodiscard]] ForkedAwaiter {
	S& scheduler;
	ForkedTask<T, S> forked_task;

	ForkedAwaiter(S& scheduler, A<T>&& awaitable)
		:scheduler{scheduler}
		,forked_task{create_fork_on(scheduler, std::move(awaitable))}
	{
	}
	
	ForkedAwaiter(S& scheduler, A<T>& awaitable)
		:scheduler{scheduler}
		,forked_task{branch_on(scheduler, awaitable)}
	{
	}

	//Awaiter
	bool await_ready() noexcept{
		return false;
	}

	std::coroutine_handle<> await_suspend(std::coroutine_handle<> caller_handle) noexcept{
		scheduler.schedule(caller_handle);
		return forked_task.my_handle;
	}

	auto await_resume() noexcept{
		return std::move(forked_task);
	}

};

//Forking Implementation
template<Scheduler S, typename A>
auto fork_on(S& scheduler, A&& awaitable){
	return ForkedAwaiter(scheduler, std::forward<A>(awaitable));
};

#endif
