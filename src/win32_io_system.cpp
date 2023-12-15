#ifdef _WIN32

#include <iostream>

#include "win32_io_net_udp_socket.h"
#include "win32_io_system.h"

IOSystem::IOSystem()
	: iocp()
	, threadpool(2)
{}

Sync<int> IOSystem::entry(){
	win32::io::net::udp::Socket socket;

	
	socket.connect(NULL, "21212", "127.0.0.1", "5555");

	iocp.addSocket(socket);
	
	std::string message = "hello string\n";

	co_await socket.send(message.c_str(), message.size()+1);
	co_await threadpool.chain(socket.send("test",5)); 		
	
	while (true){
		char read_buffer[16] = {};
		
		co_await socket.recv(read_buffer, sizeof(read_buffer));
		
		read_buffer[15] = '\0';
		std::cout << read_buffer;
	}
	
	co_return 0;
}

#endif
