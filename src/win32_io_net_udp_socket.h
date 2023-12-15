#ifndef WIN32_IO_NET_UDP_SOCKET_H
#define WIN32_IO_NET_UDP_SOCKET_H

#include <winsock2.h>
#include <ws2tcpip.h>

#include "win32_io_request_data.h"
#include "win32_io_awaiter.h"

namespace win32::io::net::udp {

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

	auto send(const char* buf, ULONG len){
		WSABUF data_buffer{len, const_cast<char*>(buf)};

		auto lambda = [=, this](RequestData& data) mutable {
			return WSASend(socket_handle, &data_buffer, 1, NULL, 0, &data, NULL);
		};

		return Awaiter(lambda);
	}

	auto recv(char* buf, ULONG len){
		WSABUF data_buffer{len, buf};
		DWORD flags = 0;	

		auto lambda = [=, this](RequestData& data) mutable{
			return WSARecv(socket_handle, &data_buffer, 1, NULL, &flags, &data, NULL);
		};

		return Awaiter(lambda);

	}


};


} 

#endif
