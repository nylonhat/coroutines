#ifdef __linux__

#include "async_io_linux.h"
#include "io_uring_callback.h"
namespace networking {

AsyncIO::AsyncIO(){
	io_uring_queue_init(16, &ring, 0);
	
	auto work = [this](){
		
		while(running){
			io_uring_cqe* cqe;
			int error_code = io_uring_wait_cqe(&ring, &cqe);

			if(error_code < 0){
				continue;
			}

			auto* data = static_cast<IOUringData*>(io_uring_cqe_get_data(cqe));
			
			if(data->callback == nullptr){
				continue;
			}

			data->callback();
		}
	};

	thread = std::jthread(work);
}

AsyncIO::~AsyncIO(){
	running.store(false);

	//Submit last blank IO to wakeup thread last time 
	submitNoop();

	thread.join();
	io_uring_queue_exit(&ring);

}

void AsyncIO::submitNoop(){
	io_uring_sqe *sqe = io_uring_get_sqe(&ring);
	io_uring_prep_nop(sqe);
	IOUringData data;
	data.callback = nullptr;
	io_uring_sqe_set_data(sqe, &data);
	io_uring_submit(&ring);

}

}


#endif

