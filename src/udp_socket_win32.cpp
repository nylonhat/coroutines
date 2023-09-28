#include "udp_socket_win32.h"
#include <iostream>

namespace networking {
namespace udp {

Socket::Socket(){

}

Socket::~Socket(){

}

addrinfo Socket::getSourceHints(){
	addrinfo source_hints;
	ZeroMemory(&source_hints, sizeof(source_hints));
	
	source_hints.ai_family = AF_INET;
	source_hints.ai_socktype = SOCK_DGRAM;
	source_hints.ai_protocol = IPPROTO_UDP;
	source_hints.ai_flags = AI_PASSIVE;

	return source_hints;
}

addrinfo Socket::getDestinationHints(){
	addrinfo destination_hints;
	ZeroMemory(&destination_hints, sizeof(destination_hints));
	
	destination_hints.ai_family = AF_INET;
	destination_hints.ai_socktype = SOCK_DGRAM;
	destination_hints.ai_protocol = IPPROTO_UDP;

	return destination_hints;
}

addrinfo* Socket::resolveAddressInfo(PCSTR address, PCSTR port, addrinfo hints){
	addrinfo* addrinfo_ptr;
	int error_code = getaddrinfo(address, port, &hints, &addrinfo_ptr);
	if(error_code != 0){
		//TODO failed
	}

	return addrinfo_ptr;

}

bool Socket::createSocket(addrinfo* address){
	socket_handle = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
	
	if(socket_handle == INVALID_SOCKET){
		return false;
	}

	return true;
}

bool Socket::bindSocket(addrinfo* source){
	int error_code = bind(socket_handle, source->ai_addr, source->ai_addrlen);

	if(error_code == SOCKET_ERROR){
		closesocket(socket_handle);
		socket_handle = INVALID_SOCKET;
		return false;
	}

	return true;
}

bool Socket::connectSocket(addrinfo* address){
	int error_code = ::connect(socket_handle, address->ai_addr, (int)address->ai_addrlen);

	if(error_code == SOCKET_ERROR){
		closesocket(socket_handle);
		socket_handle = INVALID_SOCKET;
		return false;
	}

	//Print out connection
	char m_d_host[NI_MAXHOST];
	char m_d_service[NI_MAXSERV];
	getnameinfo(address->ai_addr, address->ai_addrlen, m_d_host, sizeof(m_d_host), m_d_service, sizeof(m_d_service), NI_NUMERICHOST);

	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	char m_s_host[INET_ADDRSTRLEN];

	if (getsockname(socket_handle, (struct sockaddr *)&sin, &len) == SOCKET_ERROR){
		std::cout << "Can't get source port\n";
	}else{
		inet_ntop(AF_INET, &(sin.sin_addr), m_s_host, INET_ADDRSTRLEN);
		std::cout << m_s_host << ":" << ntohs(sin.sin_port) << " -> ";
	}

	std::cout << m_d_host << ":" << m_d_service << "\n";
	
	return true;
} 

bool Socket::connect(PCSTR s_address, PCSTR s_port, PCSTR d_address, PCSTR d_port){
	closesocket(socket_handle);
	addrinfo* attempt = NULL;
	
	addrinfo* s_addrinfo = resolveAddressInfo(s_address, s_port, getSourceHints());
	addrinfo* d_addrinfo = resolveAddressInfo(d_address, d_port, getDestinationHints());

	for(attempt = d_addrinfo; attempt!=NULL; attempt = attempt->ai_next){
		if(!createSocket(attempt)){
			continue;
		}

		if(!bindSocket(s_addrinfo)){
			continue;
		}

		if(!connectSocket(attempt)){
			continue;
		}
		
		//Find first connection
		break;
	}

	//free resources
    freeaddrinfo(s_addrinfo);
	freeaddrinfo(d_addrinfo);

	if(socket_handle == INVALID_SOCKET){
		return false;
	}

	return true;

}

void Socket::disconnect(){
	closesocket(socket_handle);
	socket_handle = INVALID_SOCKET;
}



SendingTask Socket::send(char* send_buffer, size_t send_buffer_size){
	//TODO: check if socket is not nullptr
	return SendingTask(socket_handle, send_buffer, send_buffer_size);
}


}
}
