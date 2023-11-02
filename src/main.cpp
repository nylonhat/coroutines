#include "dag_system.h"
//#include "io_system.h"
//#include "udp_socket_linux.h"
//#include "async_io_linux.h"

#include <thread>
#include <iostream>
using namespace std::literals;

int main() {
/*
	WSADATA wsa_data;
	WSAStartup(MAKEWORD(2,2), &wsa_data);
	
	IOSystem io_system;
	
	io_system.entry().block_and_get();

	WSACleanup();

*
*/
//	networking::AsyncIO async_io;

//	networking::udp::Socket socket;
//	socket.connect(0, "21212", "10.0.0.109", "5555");
	DAGSystem dag_system;
	dag_system.entry().await();
	return 0;

}




