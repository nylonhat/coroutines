#include <iostream>
#include <thread>
#include "udp_socket_win32.h"
#include "task.h"
#include "threadpool.h"
#include "test.h"
#include "iocp.h"
#include "blocking_task.h"

Threadpool threadpool{1};

struct System {
	Threadpool threadpool;
	networking::IOCP iocp;

	System()
		: threadpool(8)
		, iocp()
	{
		
	}

	~System(){
	}
	
	BlockingTask<int> entry(){
		networking::udp::Socket socket;

		socket.connect(NULL, "21212", "127.0.10.5", "34343");

		iocp.addSocket(socket.socket_handle);

		char send_string[] = "Hello world";

		co_await socket.send(send_string, strlen(send_string));

		co_return 0;
	}

};



BlockingTask<int> mainCoroutine(){
	int iterations = 10000;

	auto simulation = stressTest(iterations);

	auto result = co_await simulation;

	std::cout << "Result: " << result << "\n";
	
	co_return 0;
}


int main() {
	WSADATA wsa_data;
	WSAStartup(MAKEWORD(2,2), &wsa_data);
	
	System system;
	
	system.entry().block_and_get();

	WSACleanup();

	return 0;
}




