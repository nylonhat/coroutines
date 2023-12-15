#ifndef WIN32_IO_REQUEST_DATA_H
#define WIN32_IO_REQUEST_DATA_H

#include <winsock2.h>
#include <functional>
#include <coroutine>

namespace win32::io {

/**
 * A data structure that extends Windows WSAOVERLAPPED
 * to contain a callback function. This struct is passed
 * into calls like WSASend or WSARecv, and allows an IO
 * Completion Port to eventually resume a coroutine that
 * is waiting for the IO operation to be completed.
 */

struct RequestData : WSAOVERLAPPED {
	std::coroutine_handle<> waiting_handle = std::noop_coroutine();
	bool callback_completed = false;
	int res = 0;

	void callback(){
		callback_completed = true;
		waiting_handle.resume();
	}
};

}

#endif
