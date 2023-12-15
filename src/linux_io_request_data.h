#ifndef LINUX_IO_REQUEST_DATA_H
#define LINUX_IO_REQUEST_DATA_H

#include <functional>
#include <coroutine>

namespace linux::io {

struct RequestData {
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
