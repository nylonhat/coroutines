#ifdef _WIN32

#include "io_completion_data_wsa.h"
#include "iocp.h"

namespace networking {

IOCP::IOCP()
{
	iocp_handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 1);

	auto work = [this](){
		
		DWORD bytes_transferred;
		ULONG_PTR completion_key;
		OVERLAPPED *overlapped;

		while(running){
			GetQueuedCompletionStatus(iocp_handle, &bytes_transferred, &completion_key, &overlapped, INFINITE);
			
			auto* completion_data = static_cast<IOCompletionDataWSA*>(overlapped);
			
			completion_data->callback();
		}		

	};

	thread = std::jthread(work);

}


IOCP::~IOCP(){
	running.store(false);
	PostQueuedCompletionStatus(iocp_handle, 0, 999, NULL);
	
	thread.join();
	CloseHandle(iocp_handle);
}


bool IOCP::addSocket(udp::Socket socket){
	
	HANDLE result = CreateIoCompletionPort((HANDLE)socket.socket_handle, iocp_handle, 0, 0);
	if(result == NULL){
		return false;
	}

	return true;

}


}


#endif
