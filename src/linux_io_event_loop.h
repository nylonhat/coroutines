#ifndef LINUX_IO_EVENT_LOOP_H
#define LINUX_IO_EVENT_LOOP_H

#include <thread>
#include <atomic>

#include "liburing/liburing.h"
#include "linux_io_request_data.h"
#include "linux_io_net_udp_socket.h"

namespace linux::io {

struct EventLoop {
	io_uring ring;
	std::jthread thread;
	std::atomic<bool> running = true;

	EventLoop();

	~EventLoop();

	void submitNoop(RequestData& data);

	bool addSocket(net::udp::Socket& socket);

};

}

#endif




