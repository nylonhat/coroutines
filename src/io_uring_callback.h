#ifndef IO_URING_CALLBACK_H
#define IO_URING_CALLBACK_H

#include <functional>
#include <coroutine>

namespace networking {

struct IOUringData {
	std::coroutine_handle<> waiting_handle = std::noop_coroutine();
	bool callback_completed = false;

	void callback(){
		waiting_handle.resume();
		callback_completed = true;
	}
};

}

#endif
