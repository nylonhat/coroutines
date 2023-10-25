#include "backoff.h"
#include "work_steal_pool.h"
#include <random>

WorkStealPool::WorkStealPool(int num_threads){

	//define worker loop
	auto work = [this](){
		Backoff backoff;

		//assign each worker a unique id representing queue index
		worker_id = worker_id_ticket.fetch_add(1);
		
		std::minstd_rand random_generator{std::random_device{}()};

		while(running.load()){

			std::function<void()> task;
			//dequeue from threads own queue first
			if(queues.at(worker_id).try_dequeue(task)){
				task();
				continue;
			}
				
			//try to steal from other another random queue
			std::uniform_int_distribution<int> distribution(0,worker_id_ticket.load() -1);
			int random_index = distribution(random_generator);
			
			if(queues.at(random_index).try_dequeue(task)){
				task();
				backoff.reset();
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

void WorkStealPool::schedule(std::function<void()> task){
	//enqueue task into threads own queue
	//default to queue 0 if from outside thread
	if(queues.at(worker_id).try_enqueue(task)){
		return;
	}
	
	//Default to running task inline if can't schedule
	//Tail call optimisation should work here
	return task();
}


thread_local int WorkStealPool::worker_id = 0;

