#ifndef LINUX_IO_SYSTEM_H
#define LINUX_IO_SYSTEM_H

#include <print>

#include "sync.h"
#include "linux_io_event_loop.h"
#include "threadpool.h"

struct IOSystem {
	linux::io::EventLoop event_loop;
	Threadpool threadpool;

	IOSystem()
		: event_loop()
		, threadpool(0)
	{}

	Sync<int> entry(){
		linux::io::net::udp::Socket socket;
		
		event_loop.addSocket(socket);
		
		bool success = co_await socket.connect(NULL, "21212", "127.0.0.1", "27000");
		if(!success){
			co_return 0;
		}

		std::println("Starting echo server");

		std::string message = "You have connected to an Echo server\n";

		int bytes_sent = co_await socket.send(message.c_str(), message.size()+1);
		std::println("{:2} send", bytes_sent);
	
		//std::cout << co_await threadpool.chain(socket.send("test",5)) << "\n"; 		
	
		
		while(true){
			//Echo server
			char read_buffer[32] = {};
			int bytes_recv = co_await socket.recv(read_buffer, sizeof(read_buffer));
			read_buffer[31] = '\0';
			std::print("{:2} recv: {}", bytes_recv, read_buffer);

			auto send_task = socket.send(read_buffer, bytes_recv);
			auto bytes_sent = co_await send_task;
			//int bytes_sent = co_await co_await threadpool.branch(std::move(send_task));
			std::println("{:2} send", bytes_sent);

		}
		
		co_return 0;
	}

};

#endif
