#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <atomic>
#include <thread>
#include <functional>
#include <utility>

#include "chained_task.h"
#include "branched_task.h"
#include "bounded_mpmc_queue.h"
#include "throttle_chain.h"

/**
 * A threadpool is a type of scheduler that has a queue
 * of work that needs to be done. A fixed pool of worker
 * threads try to pull work from the queue in a first in 
 * first out (FIFO) fashion and execute them. This 
 * implementation uses exponential backoff with jitter to
 * try and reduce contention between the worker threads, 
 * and also allow threads to go idle if the work load is
 * low. 
 *
 * This threadpool supports the chaining and branching of 
 * coroutines onto it.  
 */

struct Threadpool {
private:
	bounded_mpmc_queue<std::function<void()>, 128> queue{};
	std::atomic<bool> running{true};
	std::vector<std::jthread> threads;
	ThrottleChain throttle_chain{};

public:
	//Constructor
	Threadpool(int num_threads);

	//Destructor
	~Threadpool();
	
private:	
	bool try_dequeue(std::function<void()>& task);

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
