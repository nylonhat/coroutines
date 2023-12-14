#ifndef URING_DATA_H
#define URING_DATA_H

#include <functional>
#include <coroutine>

namespace net {

struct UringData {
	std::coroutine_handle<> waiting_handle = std::noop_coroutine();
	bool callback_completed = false;
	int res = 0;

	void callback(){
		waiting_handle.resume();
		callback_completed = true;
	}
};

}

#endif
