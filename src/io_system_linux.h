#ifndef IO_SYSTEM_LINUX_H
#define IO_SYSTEM_LINUX_H

#include "sync.h"
#include "async_io_linux.h"
#include <iostream>

struct IOSystem {
	networking::AsyncIO async_io;

	IOSystem()
		: async_io()
	{}

	Sync<int> entry(){
		networking::udp::Socket socket;
		
		async_io.addSocket(socket);
		
		socket.connect(NULL, "21212", "10.0.0.129", "5555");

		std::string message = "hello string\n";

		co_await socket.send(message.c_str(), message.size()+1);
		//std::cout << co_await threadpool.chain(socket.send("test",5)) << "\n"; 		
		//	char read_buffer[16] = {};
			
		//	co_await socket.recv(read_buffer, sizeof(read_buffer));
			
		//	read_buffer[15] = '\0';
		//	std::cout << read_buffer;
		
		co_return 0;
	}

};

#endif
