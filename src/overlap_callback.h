#ifndef IO_COMPLETION_DATA_WSA_H
#define IO_COMPLETION_DATA_WSA_H

#include <winsock2.h>
#include <functional>
#include <coroutine>

/**
 * A data structure that extends Windows WSAOVERLAPPED
 * to contain a callback function. This struct is passed
 * into calls like WSASend or WSARecv, and allows an IO
 * Completion Port to eventually resume a coroutine that
 * is waiting for the IO operation to be completed.
 */

struct IOCompletionDataWSA : WSAOVERLAPPED {
	std::coroutine_handle<> waiting_handle = std::noop_coroutine();
	bool callback_completed = false;

	void callback(){
		callback_completed = true;
		coroutine_handle.resume();
	}
};

#endif
