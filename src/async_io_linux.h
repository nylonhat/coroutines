#ifndef ASYNC_IO_LINUX_H
#define ASYNC_IO_LINUX_H

#include <thread>
#include <atomic>

#include "liburing/liburing.h"
#include "uring_data.h"
#include "udp_socket_linux.h"

namespace net {

struct AsyncIO {
	io_uring ring;
	std::jthread thread;
	std::atomic<bool> running = true;

	AsyncIO();

	~AsyncIO();

	void submitNoop(UringData& data);

	bool addSocket(udp::Socket& socket);

};

}

#endif




