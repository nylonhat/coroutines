#ifndef LINUX_IO_NET_UDP_SOCKET_H
#define LINUX_IO_NET_UDP_SOCKET_H

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <iostream>

#include <liburing/liburing.h>
#include "linux_io_awaiter.h"


namespace linux::io::net::udp {

struct Socket{
	const static int INVALID_SOCKET = -1;
	const static int SOCKET_ERROR = -1;


	int sockfd = INVALID_SOCKET;

	io_uring* ring;

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

	auto send(const char* buf, size_t len){
		auto lambda = [=, this](RequestData& data){
			io_uring_sqe *sqe = io_uring_get_sqe(ring);
			io_uring_prep_send(sqe, sockfd, buf, len, 0);
			io_uring_sqe_set_data(sqe, &data);
			io_uring_submit(ring);
		};

		return Awaiter(lambda);
	}

	auto recv(char* buf, size_t len){
		auto lambda = [=, this](RequestData& data){
			io_uring_sqe *sqe = io_uring_get_sqe(ring);
			io_uring_prep_recv(sqe, sockfd, buf, len, 0);
			io_uring_sqe_set_data(sqe, &data);
			io_uring_submit(ring);
		};

		return Awaiter(lambda);
	}


};


}

#endif 
