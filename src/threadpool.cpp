#include "threadpool.h"
#include "backoff.h"

Threadpool::Threadpool(int num_threads){
	auto work = [this](){
		ThrottleNode my_throttle_node{};
		Backoff backoff{};

		throttle_chain.add(my_throttle_node);
		
		while(running.load()){
			my_throttle_node.throttle();

			std::function<void()> task;
			if(try_dequeue(task)){
				my_throttle_node.setThrottle(false);
				task();
				backoff.easein();
			}else{
				backoff.backoff();
				if(backoff.isMaxBackoff()){
					my_throttle_node.setThrottle(true);
				}
			}
		}
		
		throttle_chain.remove(my_throttle_node);

	};

	for (int i=0; i<num_threads; i++){
		threads.emplace_back(work);
	}
}

Threadpool::~Threadpool(){
	running.store(false);
}

void Threadpool::schedule(std::function<void()> task){
	if(queue.try_enqueue(task)){
		return;
	}
	
	//Default to running task inline if can't schedule
	//Tail call optimisation should work here
	return task();
}


bool Threadpool::try_dequeue(std::function<void()>& task){
	return queue.try_dequeue(task);
}


