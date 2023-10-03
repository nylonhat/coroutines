#ifndef WSA_RECV_TASK
#define WSA_RECV_TASK

#include <winsock2.h>
#include <variant>
#include <coroutine>
#include <iostream>
#include "overlap_callback.h"

namespace networking {
namespace udp {


template<typename T = bool>
struct RecvingTask {
	SOCKET socket_handle;
	std::variant<std::monostate, std::coroutine_handle<>> waiting_handle;
	OverlapWithCallback overlapped{};
	WSABUF data_buffer;
	bool was_recved = false;
	DWORD flags = 0;

	//constructor
	RecvingTask(SOCKET socket_handle, char* recv_buffer_ptr, ULONG send_buffer_size)
		: socket_handle{socket_handle}
		, data_buffer{send_buffer_size, recv_buffer_ptr}
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
			was_recved = true;
			std::get<1>(waiting_handle).resume();
		};
		
		//Call WSASend supplying access to callback
		int error_code = WSARecv(socket_handle, &data_buffer, 1, NULL, &flags, &overlapped, NULL);
	
		//WSASend successful. Stay suspended, callback will be called later.
		if ((error_code == 0) || (WSAGetLastError() == WSA_IO_PENDING)){
			return true;
		}
		
		std::cout << WSAGetLastError() << "\n";
		//WSASend failed, don't suspend because no callback will be called.
		return false;
	}

	bool await_resume(){
		return was_recved;
	}

};




}
}

#endif
