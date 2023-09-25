#include <iostream>
#include <coroutine>
#include <cassert>
#include <algorithm>
#include <chrono>
#include <concepts>
#include <vector>

#include "timer.h"
#include "blocking_task.h"
#include "task.h"
#include "threadpool.h"

Threadpool threadpool{8};

template<typename B>
concept Branchable = requires(B b){
	b;
};

int multiply_func(int a, int b){
	return a*b;
}

Task<int> multiply(int a, int b){
	//std::cout << "multiply on thread: " << std::this_thread::get_id() << "\n";
	//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	co_return a * b;
}

Task<int> permutation (){
	unsigned int count = 0;
	std::array<int, 8> array {0, 1, 2, 3, 4, 5, 6, 7};
	while(std::next_permutation(array.begin(), array.end())){
		count++;
	}

	co_return count;
}

Task<int> integerGenerator(){
	for (int i = 0; true; i++){
		co_yield i;
	}

	//return i;
}

Task<int> A(){
	//std::this_thread::sleep_for(std::chrono::milliseconds(300));
	//std::cout << "A finished\n";
	co_return 1;
}

Task<int> B(){
	//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	//std::cout << "B finished\n";
	co_return 2;
}

Task<int> C(){
	//std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	//std::cout << "C finished\n";
	co_return 3;
}

Task<int> D(auto& b){
	auto a = threadpool.branch(A());
	//auto a = A();
	
	co_return co_await a + co_await b;
}

Task<int> E(BranchedTask<int, Threadpool>& b){
	auto c = threadpool.branch(C());
	//auto c = C();
	co_return co_await b + co_await c;
}

Task<int> F(){
	std::this_thread::sleep_for(std::chrono::milliseconds(4000));
	co_return 44;
}

Task<int> dagTest(){
	unsigned int result = 0;	

	auto branchF = threadpool.branch(F());

	auto branchB = threadpool.branch(B());

	auto branchD = threadpool.branch(D(branchB));

	auto branchE = threadpool.branch(E(branchB));
	
	result += co_await branchD;
	result += co_await branchE;

	co_await branchB;
	co_await branchF;
	
	co_return result;
}

Task<int> branchesTest(int num_branches){
	unsigned int result = 0;	
	std::vector<BranchedTask<int, Threadpool>> branches;
	
	for(int i = 0; i < num_branches; i++){
		branches.emplace_back(threadpool.branch(permutation()));
	}

	for(auto& branch: branches){
		result += co_await branch;
	}

	co_return result;

}

Task<int> stressTest(int iterations){
	
	unsigned int result = 0;
	
	Timer timer;
	timer.start();

	for (int i=0; i< iterations; i++){
		result += co_await branchesTest(8); 
		
		//std::cout << "iternation: " << i << "\n";
	}

	timer.stop();
	std::cout << timer.count()/iterations << "\n";
	
	co_return result;
}

BlockingTask<int> mainCoroutine(){
	int iterations = 10000;

	auto simulation = stressTest(iterations);

	auto result = co_await simulation;

	std::cout << "Result: " << result << "\n";
	
	co_return 0;
}

int main() {
	return mainCoroutine();
}




