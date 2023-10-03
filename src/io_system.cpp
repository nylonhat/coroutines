#include <iostream>

#include "udp_socket_wsa.h"
#include "io_system.h"
#include "task.h"
#include "threadpool.h"
#include "iocp.h"
#include "blocking_task.h"

IOSystem::IOSystem()
	: threadpool(2)
	, iocp()
{}

BlockingTask<int> IOSystem::entry(){
	networking::udp::Socket socket;

	socket.connect(NULL, "21212", "10.0.0.109", "5555");

	iocp.addSocket(socket.socket_handle);
	
	std::string message = "hello string\n";

	//co_await socket.send(message.c_str(), message.size()+1);
	//std::cout << co_await threadpool.chain(socket.send("test",5)) << "\n"; 		
	while (true){
		char read_buffer[16] = {};
		
		co_await socket.recv(read_buffer, sizeof(read_buffer));
		
		read_buffer[15] = '\0';
		std::cout << read_buffer;
	}
	
	co_return 0;
}


