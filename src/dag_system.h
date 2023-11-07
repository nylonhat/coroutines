#ifndef DAG_SYSTEM_H
#define DAG_SYSTEM_H

#include "work_steal_pool.h"
#include "task.h"
#include "blocking_task.h"

template<typename B>
concept Branchable = requires(B b){
	b;
};

struct DAGSystem {
	WorkStealPool threadpool;

	DAGSystem();
	
	BlockingTask<int> entry();

	int multiply_func(int a, int b);
	Task<int> multiply(int a, int b);
	Task<int> permutation ();
	Task<int> integerGenerator();
	Task<int> A();
	Task<int> B();
	Task<int> C();
	Task<int> D(auto& b);
	Task<int> E(auto& b);
	Task<int> F();
	Task<int> dagTest();
	Task<int> branchesTest(int num_branches);
	Task<int> branchesVectorTest(int num_branches);
	Task<int> stressTest(int iterations);
	Task<int> fib(int n);

};

#endif
