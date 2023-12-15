#ifdef __linux__

#include <iostream>
#include "linux_io_event_loop.h"

namespace linux::io {

EventLoop::EventLoop(){
	io_uring_queue_init(16, &ring, 0);
	
	auto work = [this](){
		
		while(running){
			io_uring_cqe* cqe;
			int error_code = io_uring_wait_cqe(&ring, &cqe);

			if(error_code != 0){
				std::cout << "wait cqe error\n";
				continue;
			}

			auto* data = static_cast<RequestData*>(io_uring_cqe_get_data(cqe));
			data->res = cqe->res;
			data->callback();
			io_uring_cqe_seen(&ring, cqe);
		}
	};

	thread = std::jthread(work);
}

EventLoop::~EventLoop(){
	running.store(false);

	//Submit last blank IO to wakeup thread last time 
	RequestData data;
	submitNoop(data);

	thread.join();
	io_uring_queue_exit(&ring);

}

void EventLoop::submitNoop(RequestData& data){
	io_uring_sqe *sqe = io_uring_get_sqe(&ring);
	io_uring_prep_nop(sqe);
	io_uring_sqe_set_data(sqe, &data);
	io_uring_submit(&ring);

}

bool EventLoop::addSocket(net::udp::Socket& socket){
	socket.ring = &ring;
	return true;
}

}

#endif

