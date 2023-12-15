#ifndef LINUX_IO_SYSTEM_H
#define LINUX_IO_SYSTEM_H

#include <iostream>

#include "sync.h"
#include "linux_io_event_loop.h"
#include "threadpool.h"

struct IOSystem {
	linux::io::EventLoop event_loop;
	Threadpool threadpool;

	IOSystem()
		: event_loop()
		, threadpool(2)
	{}

	Sync<int> entry(){
		linux::io::net::udp::Socket socket;
		
		event_loop.addSocket(socket);
		
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
			auto send_task = socket.send(read_buffer, bytes_recv);
			int bytes_sent = co_await co_await threadpool.branch(std::move(send_task));


		}

		co_return 0;
	}

};

#endif
