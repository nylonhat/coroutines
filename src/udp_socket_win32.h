#ifndef UDP_SOCKET_WIN32_H
#define UDP_SOCKET_WIN32_H

#include <winsock2.h>
#include <ws2tcpip.h>

#include "wsa_send_task.h"

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

	SendingTask send(char* send_buffer, size_t send_buffer_size);


};


} //End UDP namespace
} //End Networking namespace 

#endif 
