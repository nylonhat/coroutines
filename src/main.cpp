#include <iostream>
#include <coroutine>
#include <cassert>
#include <algorithm>
#include <chrono>
#include "timer.h"
#include "blocking_task.h"
#include "task.h"
#include "threadpool.h"
#include <concepts>
#include "scheduler.h"

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

Task<int> stressTest(int iterations){
	
	unsigned int result = 0;
	
	Timer timer;
	timer.start();

	for (int i=0; i< iterations; i++){

		//auto branchF = threadpool.branch(F());

		//auto branchB = threadpool.branch(B());

		//auto branchD = threadpool.branch(D(branchB));

		//auto branchE = threadpool.branch(E(branchB));
		
		//result += co_await branchD;
		//result += co_await branchE;

		//co_await branchB;
		//co_await branchF;
		
		auto b1 = threadpool.chain(multiply(1,i));
		auto b2 = threadpool.chain(multiply(1,i));
		//auto b3 = threadpool.branch(permutation());
		//auto b4 = threadpool.branch(permutation());
		//auto b5 = threadpool.branch(permutation());
		//auto b6 = threadpool.branch(permutation());
		//auto b7 = threadpool.branch(permutation());
		//auto b8 = threadpool.branch(permutation());
		//auto b9 = threadpool.branch(permutation());
		//auto b10 = threadpool.branch(permutation());
		//auto b11 = threadpool.branch(permutation());
		//auto b12 = threadpool.branch(permutation());
		//auto b13 = threadpool.branch(permutation());
		//auto b14 = threadpool.branch(permutation());
		//auto b15 = threadpool.branch(permutation());
		//auto b16 = threadpool.branch(permutation());
		co_await b1;  
		co_await b2;  
		//co_await b3;  
		//co_await b4;  
		//co_await b5;  
		//co_await b6;  
		//co_await b7;  
		//co_await b8;
		//co_await b9;
		//co_await b10;
		//co_await b11;
		//co_await b12;
		//co_await b13;
		//co_await b14;
		//co_await b15;
		//co_await b16;
		

		//result += co_await threadpool.branch(multiply(1,i));
		
		
		
		//std::cout << "iternation: " << i << "\n";
	}

	timer.stop();
	std::cout << timer.count()/iterations << "\n";
	
	co_return result;
}

BlockingTask<int> mainCoroutine(){
	int iterations = 100000;

	auto my_sim = stressTest(iterations);

	auto result = co_await my_sim;

	std::cout << "Result: " << result << "\n";
	
	co_return 0;
}

int main() {
	return mainCoroutine();
}




