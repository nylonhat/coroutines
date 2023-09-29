#ifndef OVERLAP_CALLBACK_H
#define OVERLAP_CALLBACK_H

#include <winsock2.h>
#include <functional>


struct OverlapWithCallback : WSAOVERLAPPED {
	std::function<void()> callback = nullptr;
};

#endif
