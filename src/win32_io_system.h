#ifndef WIN32_IO_SYSTEM_H
#define WIN32_IO_SYSTEM_H

#include "sync.h"
#include "win32_io_event_loop.h"
#include "threadpool.h"

struct IOSystem {
	win32::io::EventLoop iocp;
	Threadpool threadpool;

	IOSystem();

	Sync<int> entry();

};

#endif
