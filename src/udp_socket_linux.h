#ifndef UDP_SOCKET_LINUX_H
#define UDP_SOCKET_LINUX_H

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
//#include "sending_task_linux.h"
//#include "recving_task_linux.h"

namespace networking::udp {

struct Socket{
	const static int INVALID_SOCKET = -1;
	const static int SOCKET_ERROR = -1;


	int file_descriptor = INVALID_SOCKET;

	//Constructor
	Socket();

	//Destructor
	~Socket();
	
	addrinfo getSourceHints();
	addrinfo getDestinationHints();
	
	addrinfo* resolveAddressInfo(const char* address, const char* port, addrinfo hints);
	
	bool createSocket(addrinfo* address_info);

	bool bindSocket(addrinfo* source_address_info);

	bool connectSocket(addrinfo* address_info);

	

	bool connect(const char* s_address, const char* s_port, const char* d_address, const char* d_port);

	void disconnect();

	//SendingTask<bool> send(const char* send_buffer, size_t send_buffer_size);

	//RecvingTask<bool> recv(char* recv_buffer, size_t recv_buffer_size);


};


}

#endif 
