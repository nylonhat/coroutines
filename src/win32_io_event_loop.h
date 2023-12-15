#ifndef WIN32_IO_EVENT_LOOP_H
#define WIN32_IO_EVENT_LOOP_H

#include "win32_io_net_udp_socket.h" 

#include <windows.h>
#include <thread>
#include <atomic>


namespace win32::io {

struct EventLoop {
	HANDLE iocp_handle;
	std::jthread thread;
	std::atomic<bool> running = true;
	
	EventLoop();

	~EventLoop();

	bool addSocket(net::udp::Socket& socket);


};

}

#endif
