#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <concepts>
#include <functional>
#include <coroutine>
#include <memory>

template<typename S>
concept Scheduler = requires(S scheduler, std::coroutine_handle<> coro){
	{scheduler.schedule(coro)} -> std::same_as<std::coroutine_handle<>>;
};

template<Scheduler S>
auto type_erased_schedule(void* type_ptr, std::coroutine_handle<> coro){
	S& scheduler = *static_cast<S*>(type_ptr);
	return scheduler.schedule(coro);
}

struct SchedulerHandle {
	void* type_ptr = nullptr;
	std::coroutine_handle<> (*schedule_ptr)(void*, std::coroutine_handle<>) = nullptr;
	
	template<Scheduler S>
	SchedulerHandle(S& scheduler)
		: type_ptr{std::addressof(scheduler)}
		, schedule_ptr{type_erased_schedule<S>} 
	{}

	SchedulerHandle(SchedulerHandle& rhs){
		type_ptr = rhs.type_ptr;
		schedule_ptr = rhs.schedule_ptr;
	}


	std::coroutine_handle<> schedule(std::coroutine_handle<> coro){
		return schedule_ptr(type_ptr, coro);
	}
};

#endif
