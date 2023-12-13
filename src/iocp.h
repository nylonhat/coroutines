#ifndef IOCP_H
#define IOCP_H

#include <windows.h>
#include <thread>
#include <atomic>

#include "udp_socket_wsa.h" 

namespace networking {

struct IOCP {
	HANDLE iocp_handle;
	std::jthread thread;
	std::atomic<bool> running = true;
	
	IOCP();

	~IOCP();

	bool addSocket(udp::Socket& socket);


};

}

#endif
