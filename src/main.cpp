#include "dag_system.h"
#include "io_system.h"

int main() {
	WSADATA wsa_data;
	WSAStartup(MAKEWORD(2,2), &wsa_data);
	
	IOSystem io_system;
	
	io_system.entry().block_and_get();

	WSACleanup();

	return 0;
}




