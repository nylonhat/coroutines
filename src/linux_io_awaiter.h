#ifndef LINUX_IO_AWAITER_H
#define LINUX_IO_AWAITER_H

#include <coroutine>
#include "linux_io_request_data.h"

namespace linux::io {

template<typename F>
struct Awaiter {
	using value_type = int;

	F func;
	RequestData data;

	Awaiter(F func)
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

