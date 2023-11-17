#ifndef DAG_SYSTEM_H
#define DAG_SYSTEM_H

#include "work_steal_pool.h"
#include "task.h"
#include "blocking_task.h"

template<typename B>
concept Branchable = requires(B b){
	b;
};

struct DagSystem {
	WorkStealPool threadpool;

	DagSystem();
	
	BlockingTask<int> entry();

	Task<int> multiply(int a, int b);
	Task<int> permutation();
	Task<int> benchmark(int iterations);
	Task<int> fib(int n);
	Task<int> vectorTest(size_t);
	Task<int> arrayTest();

};

#endif
