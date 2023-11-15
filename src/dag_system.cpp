#include <algorithm>
#include <chrono>
#include <concepts>
#include <vector>
#include <array>
#include <variant>
#include <iostream>

#include "branch.h"
#include "dag_system.h"
#include "timer.h"
#include "branch_array.h"

DagSystem::DagSystem()
	: threadpool(8)
{}

BlockingTask<int> DagSystem::entry(){
	int iterations = 1;

	auto result = co_await benchmark(iterations);

	std::cout << "Result: " << result << "\n";
	
	co_return 0;
} 

Task<int> DagSystem::multiply(int a, int b){
	//std::cout << "multiply on thread: " << std::this_thread::get_id() << "\n";
	//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	co_return a * b;
}

Task<int> DagSystem::permutation (){
	//std::cout << "perm on thread: " << std::this_thread::get_id() << "\n";
	unsigned int count = 0;
	std::array<int, 7> array {0, 1, 2, 3, 4, 5, 6};
	while(std::next_permutation(array.begin(), array.end())){
		count++;
	}

	co_return count;
}

Task<int> DagSystem::fib(int n){
	if(n < 2){
		co_return n;
	}

	auto branch = co_await threadpool.branch(fib(n-1));
	co_return co_await fib(n-2) + co_await branch; 
}

Task<int> DagSystem::arrayTest(){
	using Branch = std::variant<std::monostate, Branch<int, WorkStealPool>>;
	
	int result = 0;
	size_t limit = 0;
	std::array<Branch, 8> branches{};
	
	size_t index = 0;
	while(limit < 1'000'000){
		auto last_slot = emplace_in(branches, co_await threadpool.branch(permutation()), index);	
		if(last_slot.index() == 1){
			result += co_await std::get<1>(last_slot);
		}
		limit++;
	}

	for(auto& branch : branches){
		if(branch.index() == 1){
			result += co_await std::get<1>(branch);
		}
	}


	co_return result;

}

Task<int> DagSystem::benchmark(int iterations){
	
	unsigned int result = 0;
	
	Timer timer;
	timer.start();

	for (int i=0; i< iterations; i++){
		//result = co_await fib(48);
		//result += co_await multiply(i, 1);
		//result += co_await threadpool.chain(multiply(i, 1));
		//result += co_await co_await threadpool.branch(multiply(i, 1));
		//result += co_await threadpool.spawn(multiply(i, 1));
		result += co_await arrayTest();
	}

	timer.stop();
	std::cout << timer.count()/iterations << " ns\n";
	
	co_return result;
}
