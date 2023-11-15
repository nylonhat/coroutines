#include "backoff.h"
#include "work_steal_pool.h"
#include <random>
#include <iostream>

thread_local int WorkStealPool::worker_id = 0;
thread_local WorkStealPool* WorkStealPool::my_pool = nullptr;

WorkStealPool::WorkStealPool(int num_threads) {

	//define worker loop
	auto work = [this](){
		Backoff backoff;

		//assign each worker a unique id representing queue index
		my_pool = this;
		worker_id = worker_id_ticket.fetch_add(1);
		
		std::minstd_rand random_generator{std::random_device{}()};

		while(running.load()){

			std::coroutine_handle<> task;
			//dequeue from threads own queue first 
			if(queues.at(worker_id).try_local_pop(task)){
				task();
				continue;
			}
			
			if(master_queue.try_dequeue(task)){
				task();
				continue;
			}

			//try to steal from other random queues
			std::uniform_int_distribution<int> distribution(0,worker_id_ticket.load() -1);
			for(int i=0; i<worker_id_ticket.load()-1; i++){
				int random_index = distribution(random_generator);
			
				if(queues.at(random_index).try_steal(task)){
					task();
					backoff.reset();
					break;
				}
			}
			
			if(worker_id == 0){
				continue;
			}
			
			backoff.backoff();

		}

	};

	//start each worker
	for (int i=0; i<num_threads; i++){
		threads.emplace_back(work);
	}
}

WorkStealPool::~WorkStealPool(){
	running.store(false);
}

std::coroutine_handle<> WorkStealPool::schedule(std::coroutine_handle<> task){
	//thread does not belong to this pool
	if(my_pool != this){
		return master_queue.try_enqueue(task) ? std::noop_coroutine() : task;
	}

	//enqueue task into threads own queue
	return queues.at(worker_id).try_local_push(task) ? std::noop_coroutine() : task;
}




