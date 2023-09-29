#include "overlap_callback.h"
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
			
			if (completion_key == 999){
				break;
			}

			auto* io_task = static_cast<OverlapWithCallback*>(overlapped);
			io_task->callback();
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


bool IOCP::addSocket(SOCKET socket_handle){
	
	HANDLE result = CreateIoCompletionPort((HANDLE)socket_handle, iocp_handle, 0, 0);
	if(result == NULL){
		return false;
	}

	return true;

}


}
