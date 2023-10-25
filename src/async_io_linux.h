#ifndef ASYNC_IO_LINUX_H
#define ASYNC_IO_LINUX_H

#include <thread>
#include <atomic>

#include "liburing/liburing.h"

namespace networking {

struct AsyncIO {
	io_uring ring;
	std::jthread thread;
	std::atomic<bool> running = true;

	AsyncIO();

	~AsyncIO();

	void submitNoop();
};

}

#endif



