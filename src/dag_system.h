#ifndef DAG_SYSTEM_H
#define DAG_SYSTEM_H

#include "threadpool.h"
#include "task.h"
#include "sync.h"

template<typename B>
concept Branchable = requires(B b){
	b;
};

struct DagSystem {
	Threadpool threadpool;

	DagSystem();
	
	Sync<int> entry();

	Task<int> multiply(int a, int b);
	Task<int> permutation();
	Task<size_t> benchmark(size_t iterations);
	Task<size_t> fib(int n);
	Task<int> vectorTest(size_t elements);
	//Task<int> recyclerTest(size_t limit);

	Task<int> forkTest(size_t size);
	size_t fib_f(int n);
};

#endif
