#ifndef SCHEDULER_H
#define SCHEDULER_H

#include<concepts>
#include<functional>
#include <coroutine>

template<typename S>
concept Scheduler = requires(S s, std::coroutine_handle<> func){
	s.schedule(func);
};

#endif
