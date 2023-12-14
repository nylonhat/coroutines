#ifndef IO_SYSTEM_LINUX_H
#define IO_SYSTEM_LINUX_H

#include "sync.h"
#include "async_io_linux.h"
#include <iostream>

struct IOSystem {
	net::AsyncIO async_io;

	IOSystem()
		: async_io()
	{}

	Sync<int> entry(){
		net::udp::Socket socket;
		
		async_io.addSocket(socket);
		
		socket.connect(NULL, "21212", "127.0.0.1", "5555");

		std::string message = "hello string\n";

		int bytes_sent = co_await socket.send(message.c_str(), message.size()+1);
		std::cout << "bytes sent: " << bytes_sent << std::endl;
	
		//std::cout << co_await threadpool.chain(socket.send("test",5)) << "\n"; 		
		
		while(true){
			char read_buffer[32] = {};
			int bytes_recv = co_await socket.recv(read_buffer, sizeof(read_buffer));
			read_buffer[31] = '\0';
			std::cout << read_buffer;

			//echo
			int bytes_sent = co_await socket.send(read_buffer, bytes_recv);


		}

		co_return 0;
	}

};

#endif
