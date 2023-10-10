#ifndef UDP_SOCKET_WSA_H
#define UDP_SOCKET_WSA_H

#include <winsock2.h>
#include <ws2tcpip.h>

#include "sending_task_wsa.h"
#include "recving_task_wsa.h"

namespace networking {
namespace udp{

struct Socket{
	SOCKET socket_handle = INVALID_SOCKET;

	//Constructor
	Socket();

	//Destructor
	~Socket();
	
	addrinfo getSourceHints();
	addrinfo getDestinationHints();
	
	addrinfo* resolveAddressInfo(PCSTR address, PCSTR port, addrinfo hints);
	
	bool createSocket(addrinfo* address_info);

	bool bindSocket(addrinfo* source_address_info);

	bool connectSocket(addrinfo* address_info);

	

	bool connect(PCSTR s_address, PCSTR s_port, PCSTR d_address, PCSTR d_port);

	void disconnect();

	SendingTask<bool> send(const char* send_buffer, size_t send_buffer_size);

	RecvingTask<bool> recv(char* recv_buffer, size_t recv_buffer_size);


};


} //End UDP namespace
} //End Networking namespace 

#endif
