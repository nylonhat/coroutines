#ifdef _WIN32

#include "win32_io_event_loop.h"
#include "win32_io_request_data.h"

namespace win32::io {

EventLoop::EventLoop()
{
	iocp_handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 1);

	auto work = [this](){
		
		DWORD bytes_transferred;
		ULONG_PTR completion_key;
		OVERLAPPED *overlapped;

		while(running){
			GetQueuedCompletionStatus(iocp_handle, &bytes_transferred, &completion_key, &overlapped, INFINITE);
			
			auto* completion_data = static_cast<RequestData*>(overlapped);
		
			completion_data->res = bytes_transferred;
			completion_data->callback();
		}		

	};

	thread = std::jthread(work);

}


EventLoop::~EventLoop(){
	running.store(false);
	PostQueuedCompletionStatus(iocp_handle, 0, 999, NULL);
	
	thread.join();
	CloseHandle(iocp_handle);
}


bool EventLoop::addSocket(net::udp::Socket& socket){
	
	HANDLE result = CreateIoCompletionPort((HANDLE)socket.socket_handle, iocp_handle, 0, 0);
	if(result == NULL){
		return false;
	}

	return true;

}


}


#endif
