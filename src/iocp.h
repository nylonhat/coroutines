#ifdef _WIN32

#ifndef IOCP_H
#define IOCP_H

#include <windows.h>
#include <thread>
#include <atomic>

namespace networking {

struct IOCP {
	HANDLE iocp_handle;
	std::jthread thread;
	std::atomic<bool> running = true;
	
	IOCP();

	~IOCP();

	bool addSocket(SOCKET socket_handle);


};

}

#endif

#endif
