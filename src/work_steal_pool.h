#ifndef WORK_STEAL_POOL_H
#define WORK_STEAL_POOL_H

#include <atomic>
#include <thread>
#include <functional>
#include <utility>
#include <coroutine>

#include "chain.h"
#include "branch.h"
#include "spawn.h"
#include "bounded_mpmc_queue.h"
#include "bounded_workstealing_deque.h"
/**
 */
struct WorkStealPool {
private:
	std::atomic<bool> running{true};
	std::vector<std::jthread> threads;
	thread_local static int worker_id;
	thread_local static WorkStealPool* my_pool;

	std::atomic<int> worker_id_ticket = 0;
	std::array< bounded_workstealing_deque<std::coroutine_handle<>,4>, 16> queues{};
	bounded_mpmc_queue<std::coroutine_handle<>, 16> master_queue{};

public:
	//Constructor
	WorkStealPool(int num_threads);

	//Destructor
	~WorkStealPool();
	
private:	

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

	template<typename A>
	auto spawn(A&& awaitable){
		return spawn_on(*this, std::forward<A>(awaitable));
	}


};

#endif
