#ifndef IO_TASK_LINUX_H
#define IO_TASK_LINUX_H

#include <coroutine>
#include "io_uring_callback.h"

namespace networking::udp {

template<typename F>
struct IOTask {
	using value_type = bool;

	F func;
	IOUringData data;

	IOTask(F func)
		:func{func}
	{}

	bool await_ready(){return false;}

	bool await_suspend(std::coroutine_handle<> caller_handle){
		data.waiting_handle = caller_handle;
		std::invoke(func, data);
		return true;
	}

	bool await_resume(){
		return data.callback_completed;
	}


};


}

#endif

