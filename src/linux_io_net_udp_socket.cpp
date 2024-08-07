#ifdef __linux__

#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <print>

#include "linux_io_net_udp_socket.h"
#include "liburing/liburing.h"

namespace linux::io::net::udp {

Socket::Socket(){

}

Socket::~Socket(){

}

addrinfo Socket::getSourceHints(){
	addrinfo source_hints;
	memset(&source_hints, 0, sizeof(source_hints));
	
	source_hints.ai_family = AF_INET;
	source_hints.ai_socktype = SOCK_DGRAM;
	source_hints.ai_protocol = IPPROTO_UDP;
	source_hints.ai_flags = AI_PASSIVE;

	return source_hints;
}

addrinfo Socket::getDestinationHints(){
	addrinfo destination_hints;
	memset(&destination_hints, 0, sizeof(destination_hints));
	
	destination_hints.ai_family = AF_INET;
	destination_hints.ai_socktype = SOCK_DGRAM;
	destination_hints.ai_protocol = IPPROTO_UDP;

	return destination_hints;
}

addrinfo* Socket::resolveAddressInfo(const char* address, const char* port, addrinfo hints){
	addrinfo* addrinfo_ptr;
	int error_code = getaddrinfo(address, port, &hints, &addrinfo_ptr);
	if(error_code != 0){
		//TODO failed
		std::println("getaddrinfo failed");
	}

	return addrinfo_ptr;

}

Task<bool> Socket::createSocket(addrinfo* address){
	//IORING_OP_SOCKET not supported correctly until linux kernel 6.3
	sockfd = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
	
	if(sockfd < 0){//== INVALID_SOCKET){
		std::println("sockfd error {}", sockfd);
		sockfd = INVALID_SOCKET;
		std::println("unable to create socket");
		co_return false;
	}

	co_return true;
}

bool Socket::bindSocket(addrinfo* source){
	int error_code = bind(sockfd, source->ai_addr, source->ai_addrlen);

	if(error_code == SOCKET_ERROR){
		std::println("unable to bind socket");
		perror("bind error:");
		close(sockfd);
		sockfd = INVALID_SOCKET;
		return false;
	}

	return true;
}

Task<bool> Socket::connectSocket(addrinfo* address){
	int error_code = co_await connect_async(sockfd, address->ai_addr, (int)address->ai_addrlen);

	if(error_code < 0){//== SOCKET_ERROR){
		std::println("socket connect failed");
		close(sockfd);
		sockfd = INVALID_SOCKET;
		co_return false;
	}

	//Print out connection
	char m_d_host[NI_MAXHOST];
	char m_d_service[NI_MAXSERV];
	getnameinfo(address->ai_addr, address->ai_addrlen, m_d_host, sizeof(m_d_host), m_d_service, sizeof(m_d_service), NI_NUMERICHOST);

	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	char m_s_host[INET_ADDRSTRLEN];

	if (getsockname(sockfd, (struct sockaddr *)&sin, &len) == SOCKET_ERROR){
		std::println("couldn't get source port");
	}else{
		inet_ntop(AF_INET, &(sin.sin_addr), m_s_host, INET_ADDRSTRLEN);
	}
	std::println("udp connection:");
	std::println("{}:{} -> {}:{}", m_s_host, ntohs(sin.sin_port), m_d_host, m_d_service);

	co_return true;
} 

Task<bool> Socket::connect(const char* s_address, const char* s_port, const char* d_address, const char* d_port){
	close(sockfd);
	addrinfo* attempt = NULL;
	
	addrinfo* s_addrinfo = resolveAddressInfo(s_address, s_port, getSourceHints());
	addrinfo* d_addrinfo = resolveAddressInfo(d_address, d_port, getDestinationHints());

	for(attempt = d_addrinfo; attempt!=NULL; attempt = attempt->ai_next){
		if(!co_await createSocket(attempt)){
			continue;
		}

		if(!bindSocket(s_addrinfo)){
			continue;
		}

		if(!co_await connectSocket(attempt)){
			continue;
		}
		
		//Find first connection
		break;
	}

	//free resources
    freeaddrinfo(s_addrinfo);
	freeaddrinfo(d_addrinfo);

	if(sockfd == INVALID_SOCKET){
		std::println("socket connection failed");
		co_return false;
	}

	co_return true;

}

void Socket::disconnect(){
	close(sockfd);
	sockfd = INVALID_SOCKET;
}



}

#endif
