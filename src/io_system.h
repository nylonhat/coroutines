#ifndef IO_SYSTEM_H
#define IO_SYSTEM_H

#include "blocking_task.h"
#include "iocp.h"

struct IOSystem {
	networking::IOCP iocp;

	IOSystem();

	BlockingTask<int> entry();

};

#endif
