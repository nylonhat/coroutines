#include "threadpool.h"
#include "backoff.h"

Threadpool::Threadpool(int num_threads){
	auto work = [this](){
		Backoff backoff;
		
		while(running.load()){
			std::function<void()> task;
			if(try_dequeue(task)){
				task();
				backoff.reset();
			}else{
				backoff.backoff();
			}
		}

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


