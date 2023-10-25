#ifndef WORK_STEAL_POOL_H
#define WORK_STEAL_POOL_H

#include <atomic>
#include <thread>
#include <functional>
#include <utility>

#include "chained_task.h"
#include "branched_task.h"
#include "bounded_mpmc_queue.h"

/**
 */
struct WorkStealPool {
private:
	std::atomic<bool> running{true};
	std::vector<std::jthread> threads;
	thread_local static int worker_id;
	std::atomic<int> worker_id_ticket = 0;
	std::array< bounded_mpmc_queue<std::function<void()>,8>, 16> queues{};

public:
	//Constructor
	WorkStealPool(int num_threads);

	//Destructor
	~WorkStealPool();
	
private:	

public:
	void schedule(std::function<void()> task);
	
	
	//Chaining Implementation
	template<typename A>
	auto chain(A&& awaitable){
		return chain_on(*this, std::forward<A>(awaitable));
	}

	//Branching Implementation	
	template<typename A>
	auto branch(A&& awaitable){
		return branch_on(*this, std::forward<A>(awaitable));
	}


};

#endif