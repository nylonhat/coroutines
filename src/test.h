#ifndef TEST_H
#define TEST_H

#include "task.h"
#include "threadpool.h"
#include "branched_task.h"

template<typename B>
concept Branchable = requires(B b){
	b;
};

int multiply_func(int a, int b);
Task<int> multiply(int a, int b);
Task<int> permutation ();
Task<int> integerGenerator();
Task<int> A();
Task<int> B();
Task<int> C();
Task<int> D(auto& b);
Task<int> E(BranchedTask<int, Threadpool>& b);
Task<int> F();
Task<int> dagTest();
Task<int> branchesTest(int num_branches);
Task<int> branchesVectorTest(int num_branches);
Task<int> stressTest(int iterations);


#endif
