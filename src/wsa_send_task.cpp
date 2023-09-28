#include "wsa_send_task.h"
namespace networking{
namespace udp{

void sendingTaskCompletionRoutine(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags){
	auto* my_sending_task = static_cast<SendingTask*>(lpOverlapped->hEvent);
	std::get<1>(my_sending_task->waiting_handle).resume();

}



}
}

