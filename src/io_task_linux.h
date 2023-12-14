#ifndef IO_TASK_LINUX_H
#define IO_TASK_LINUX_H

#include <coroutine>
#include "uring_data.h"

namespace net::udp {

template<typename F>
struct IOTask {
	using value_type = int;

	F func;
	UringData data;

	IOTask(F func)
		:func{func}
	{}

	bool await_ready(){return false;}

	bool await_suspend(std::coroutine_handle<> caller_handle){
		data.waiting_handle = caller_handle;
		std::invoke(func, data);
		return true;
	}

	int await_resume(){
		return data.res;
	}


};


}

#endif

