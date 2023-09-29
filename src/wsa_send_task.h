#ifndef WSA_SEND_TASK
#define WSA_SEND_TASK

#include <winsock2.h>
#include <variant>
#include <coroutine>
#include <iostream>
#include "overlap_callback.h"

namespace networking {
namespace udp {



struct SendingTask {
	SOCKET socket_handle;
	const char* send_buffer_ptr;
	size_t send_buffer_size;
	
	std::variant<std::monostate, std::coroutine_handle<>> waiting_handle;

	OverlapWithCallback overlapped{};

	WSABUF data_buffer;


	//constructor
	SendingTask(SOCKET socket_handle, char* send_buffer_ptr, size_t send_buffer_size)
		: socket_handle{socket_handle}
		, send_buffer_ptr{send_buffer_ptr}
		, send_buffer_size{send_buffer_size}
	{
		data_buffer.len = send_buffer_size;
		data_buffer.buf = send_buffer_ptr;
		
		//WSAOVERLAPPED allows us to store a void* as last member;
		//use it to store 'this' pointer
		overlapped.hEvent = 0;

		overlapped.callback = [this](){
			std::get<1>(waiting_handle).resume();
		};
		
	}

	//Awaiter
	bool await_ready(){
		return false;
	}

	void await_suspend(std::coroutine_handle<> caller_handle){
		//suspend the current coroutine
		waiting_handle = caller_handle;
		
		//call WSASend
		//TODO: check for error
		int error_code = WSASend(socket_handle, &data_buffer, 1, NULL, 0, 
				&overlapped, NULL);
		
		if (error_code == 0){
		}
	}

	bool await_resume(){
		return true;
	}

};




}
}

#endif
