#include "dag_system.h"
//#include "io_system.h"
#include "io_system_linux.h"


int main() {
/*
	WSADATA wsa_data;
	WSAStartup(MAKEWORD(2,2), &wsa_data);
	
	IOSystem io_system;
	
	io_system.entry().await();

	WSACleanup();

*/
	
	IOSystem io_system{};
	io_system.entry().await();

	//DagSystem dag_system;
	//dag_system.entry().await();
	
	return 0;

}




