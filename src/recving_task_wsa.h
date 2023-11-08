#ifndef WSA_RECV_TASK
#define WSA_RECV_TASK

#include <winsock2.h>
#include <variant>
#include <coroutine>
#include <iostream>
#include "io_completion_data_wsa.h"

namespace networking {
namespace udp {


template<typename T = bool>
struct RecvingTask {
	SOCKET socket_handle;
	IOCompletionDataWSA completion_data{};
	WSABUF data_buffer;
	DWORD flags = 0;

	//constructor
	RecvingTask(SOCKET socket_handle, char* buffer_ptr, ULONG buffer_size)
		: socket_handle{socket_handle}
		, data_buffer{buffer_size, buffer_ptr}
	{}


	//Awaiter
	bool await_ready(){
		return false;
	}

	bool await_suspend(std::coroutine_handle<> caller_handle){
		
		//Custom OVERLAPPED structure allows us to store a callback
		//Use callback to resume waiting coroutine
		completion_data.waiting_handle = caller_handle;
		
		//Call WSASend supplying access to callback
		int error_code = WSARecv(socket_handle, &data_buffer, 
				1, NULL, &flags, &completion_data, NULL);
	
		//WSASend successful. Stay suspended, callback will be called later.
		if ((error_code == 0) || (WSAGetLastError() == WSA_IO_PENDING)){
			return true;
		}
		
		//WSASend failed, don't suspend because no callback will be called.
		return false;
	}

	bool await_resume(){
		return completion_data.callback_completed;
	}

};




}
}

#endif
