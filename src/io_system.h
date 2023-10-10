#ifdef _WIN32

#ifndef IO_SYSTEM_H
#define IO_SYSTEM_H

#include "blocking_task.h"
#include "threadpool.h"
#include "iocp.h"

struct IOSystem {
	Threadpool threadpool;
	networking::IOCP iocp;

	IOSystem();

	BlockingTask<int> entry();

};


#endif

#endif
