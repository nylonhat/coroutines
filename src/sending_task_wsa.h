#ifndef SENDING_TASK_WSA_H
#define SENDING_TASK_WSA_H

#include <winsock2.h>
#include <variant>
#include <coroutine>

#include "overlap_callback.h"

namespace networking {
namespace udp {


template<typename T = bool>
struct SendingTask {
	SOCKET socket_handle;
	std::variant<std::monostate, std::coroutine_handle<>> waiting_handle;
	OverlapWithCallback overlapped{};
	WSABUF data_buffer;
	bool was_sent = false;

	//constructor
	SendingTask(SOCKET socket_handle, const char* send_buffer_ptr, ULONG send_buffer_size)
		: socket_handle{socket_handle}
		, data_buffer{send_buffer_size, const_cast<char*>(send_buffer_ptr)}
	{}


	//Awaiter
	bool await_ready(){
		return false;
	}

	bool await_suspend(std::coroutine_handle<> caller_handle){
		//Suspend the current coroutine and store handle to resume later
		waiting_handle = caller_handle;
		
		//Custom OVERLAPPED structure allows us to store a callback
		//Use callback to resume waiting coroutine
		overlapped.callback = [this](){
			was_sent = true;
			std::get<1>(waiting_handle).resume();
		};
		
		//Call WSASend supplying access to callback
		int error_code = WSASend(socket_handle, &data_buffer, 1, NULL, 0, &overlapped, NULL);
	
		//WSASend successful. Stay suspended, callback will be called later.
		if ((error_code == 0) || (WSAGetLastError() == WSA_IO_PENDING)){
			return true;
		}

		//WSASend failed, don't suspend because no callback will be called.
		return false;
	}

	bool await_resume(){
		return was_sent;
	}

};




}
}

#endif
