#ifndef IO_URING_CALLBACK_H
#define IO_URING_CALLBACK_H

#include <functional>

namespace networking {

struct IOUringData {
	std::function<void()> callback;
};

}

#endif
