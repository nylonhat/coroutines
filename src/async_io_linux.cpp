#ifdef __linux__

#include <iostream>
#include "async_io_linux.h"
#include "uring_data.h"

namespace net {

AsyncIO::AsyncIO(){
	io_uring_queue_init(16, &ring, 0);
	
	auto work = [this](){
		
		while(running){
			io_uring_cqe* cqe;
			int error_code = io_uring_wait_cqe(&ring, &cqe);

			if(error_code != 0){
				continue;
			}

			auto* data = static_cast<UringData*>(io_uring_cqe_get_data(cqe));
			data->res = cqe->res;
			data->callback();
			io_uring_cqe_seen(&ring, cqe);
		}
	};

	thread = std::jthread(work);
}

AsyncIO::~AsyncIO(){
	running.store(false);

	//Submit last blank IO to wakeup thread last time 
	UringData data;
	submitNoop(data);

	thread.join();
	io_uring_queue_exit(&ring);

}

void AsyncIO::submitNoop(UringData& data){
	io_uring_sqe *sqe = io_uring_get_sqe(&ring);
	io_uring_prep_nop(sqe);
	io_uring_sqe_set_data(sqe, &data);
	io_uring_submit(&ring);

}

bool AsyncIO::addSocket(udp::Socket& socket){
	socket.ring = &ring;
	return true;
}

}

#endif

