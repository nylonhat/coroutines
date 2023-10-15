#include <algorithm>
#include <chrono>
#include <concepts>
#include <vector>
#include <array>
#include <variant>
#include <iostream>

#include "dag_system.h"
#include "timer.h"

DAGSystem::DAGSystem()
	: threadpool{3}
{}

BlockingTask<int> DAGSystem::entry(){
	int iterations = 1000000;

	auto simulation = stressTest(iterations);

	auto result = co_await simulation;

	std::cout << "Result: " << result << "\n";
	
	co_return 0;
} 

int DAGSystem::multiply_func(int a, int b){
	return a*b;
}

Task<int> DAGSystem::multiply(int a, int b){
	//std::cout << "multiply on thread: " << std::this_thread::get_id() << "\n";
	//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	co_return a * b;
}

Task<int> DAGSystem::permutation (){
	unsigned int count = 0;
	std::array<int, 8> array {0, 1, 2, 3, 4, 5, 6, 7};
	while(std::next_permutation(array.begin(), array.end())){
		count++;
	}

	co_return count;
}

Task<int> DAGSystem::integerGenerator(){
	for (int i = 0; true; i++){
		co_yield i;
	}

	//return i;
}

Task<int> DAGSystem::A(){
	//std::this_thread::sleep_for(std::chrono::milliseconds(300));
	//std::cout << "A finished\n";
	co_return 1;
}

Task<int> DAGSystem::B(){
	//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	//std::cout << "B finished\n";
	co_return 2;
}

Task<int> DAGSystem::C(){
	//std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	//std::cout << "C finished\n";
	co_return 3;
}

Task<int> DAGSystem::D(auto& b){
	auto a = threadpool.branch(A());
	//auto a = A();
	
	co_return co_await a + co_await b;
}

Task<int> DAGSystem::E(BranchedTask<int, Threadpool>& b){
	auto c = threadpool.branch(C());
	//auto c = C();
	co_return co_await b + co_await c;
}

Task<int> DAGSystem::F(){
	std::this_thread::sleep_for(std::chrono::milliseconds(4000));
	co_return 44;
}

Task<int> DAGSystem::dagTest(){
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

Task<int> DAGSystem::branchesTest(int num_branches){
	unsigned int result = 0;
	using Branch = std::variant<std::monostate, BranchedTask<int, Threadpool>>;
	
	std::array<Branch, 8> branches;
	
	for(auto& branch : branches){
		branch.emplace<1> (threadpool.branch(permutation()));
	}

	for(auto& branch: branches){
		result += co_await std::get<1>(branch);
	}

	co_return result;

}

Task<int> DAGSystem::branchesVectorTest(int num_branches){
	unsigned int result = 0;	
	std::vector<BranchedTask<int, Threadpool>> branches;
	branches.reserve(num_branches); 	
	
	for(int i = 0; i < num_branches; i++){
		branches.emplace_back(threadpool.branch(permutation()));
	}

	for(auto& branch: branches){
		result += co_await branch;
	}

	co_return result;

}


Task<int> DAGSystem::stressTest(int iterations){
	
	unsigned int result = 0;
	
	Timer timer;
	timer.start();

	for (int i=0; i< iterations; i++){
		//result += co_await branchesTest(8);
		result += co_await threadpool.branch(multiply(i, 1));
		//std::cout << "iternation: " << i << "\n";
	}

	timer.stop();
	std::cout << timer.count()/iterations << "\n";
	
	co_return result;
}
