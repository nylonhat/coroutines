//#include "dag_system.h"
//#include "io_system.h"
#include "udp_socket_linux.h"

int main() {
/*
	WSADATA wsa_data;
	WSAStartup(MAKEWORD(2,2), &wsa_data);
	
	IOSystem io_system;
	
	io_system.entry().block_and_get();

	WSACleanup();

*/
	networking::udp::Socket socket;
	socket.connect(0, "21212", "10.0.0.109", "5555");

	return 0;
}




