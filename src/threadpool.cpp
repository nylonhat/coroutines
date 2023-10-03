#include "threadpool.h"
#include "backoff.h"

Threadpool::Threadpool(int num_threads){
	auto work = [this](){
		Backoff backoff;
		
		while(running){
			std::function<void()> task;
			if(dequeue(task)){
				task();
				backoff.easein();
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

bool Threadpool::schedule(std::function<void()> task){
	return queue.try_enqueue(task);
}


bool Threadpool::dequeue(std::function<void()>& task){
	return queue.try_dequeue(task);
}


