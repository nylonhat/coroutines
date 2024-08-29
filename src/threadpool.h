#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <atomic>
#include <thread>
#include <utility>
#include <coroutine>
#include <vector>
#include <array>

#include "chain.h"
#include "branch.h"
#include "queue.h"
#include "deque.h"

struct Threadpool {
private:
	std::atomic<bool> running{true};
	std::vector<std::jthread> threads;
	thread_local static size_t worker_id;
	thread_local static Threadpool* my_pool;

	std::atomic<size_t> worker_id_ticket = 0;
	std::array<Deque<std::coroutine_handle<>,4>, 16> queues{};
	Queue<std::coroutine_handle<>, 16> master_queue{};

public:
	//Constructor
	Threadpool(int num_threads);

	//Destructor
	~Threadpool();
	
private:	
	void work();

public:
	std::coroutine_handle<> schedule(std::coroutine_handle<> task);
	
	template<typename A>
	auto chain(A&& awaitable){
		return chain_on(*this, std::forward<A>(awaitable));
	}

	template<typename A>
	auto branch(A&& awaitable){
		return branch_on(*this, std::forward<A>(awaitable));
	}


};

#endif
