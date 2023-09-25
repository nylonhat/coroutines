#ifndef SCHEDULER_H
#define SCHEDULER_H

#include<concepts>
#include<functional>

template<typename S>
concept Scheduler = requires(S s, std::function<void()> func){
	s.schedule(func);
};

#endif
