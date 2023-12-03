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
#include "recycler.h"

DagSystem::DagSystem()
	: threadpool(8)
{}

BlockingTask<int> DagSystem::entry(){
	size_t iterations = 1;

	auto result = co_await benchmark(iterations);

	std::cout << "Result: " << result << "\n";
	
	co_return 0;
}

Task<size_t> DagSystem::benchmark(int iterations){
	
	size_t result = 0;
	
	Timer timer;
	timer.start();

	for (size_t i=0; i< iterations; i++){
		//result = fib_f(50);
		//result = co_await fib(50);
		//result += co_await multiply(i, 1);
		//result += co_await threadpool.chain(multiply(i, 1));
		//result += co_await co_await threadpool.branch(multiply(i, 1));
		//result += co_await threadpool.spawn(multiply(i, 1));
		result += co_await recyclerTest(1'000'000);
		//result = co_await vectorTest(1'000'000);
	}

	timer.stop();
	std::cout << timer.count()/iterations << " ns\n";
	
	co_return result;
}


Task<int> DagSystem::multiply(int a, int b){
	//std::cout << "multiply on thread: " << std::this_thread::get_id() << "\n";
	//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	co_return a * b;
}

Task<int> DagSystem::permutation(){
	//std::cout << "perm on thread: " << std::this_thread::get_id() << "\n";
	unsigned int count = 0;
	std::array<int, 7> array {0, 1, 2, 3, 4, 5, 6};
	while(std::next_permutation(array.begin(), array.end())){
		count++;
	}

	co_return count;
}

size_t DagSystem::fib_f(int n){
	if(n < 2){
		return n;
	}
	return fib_f(n-2) + fib_f(n-1);
}

Task<size_t> DagSystem::fib(int n){
	if(n < 33){
		co_return fib_f(n);
	}

	auto branch = co_await threadpool.branch(fib(n-1));
	co_return co_await fib(n-2) + co_await branch; 
}

Task<int> DagSystem::vectorTest(size_t size){
	size_t result = 0;

	std::vector<Branch<int,WorkStealPool>> branches{};
	branches.reserve(size);

	for(size_t i = 0; i<size; i++){
		branches.emplace_back(co_await threadpool.branch(multiply(1, 1)));
	}

	for(auto& branch: branches){
		result += co_await branch;
	}

	co_return result;
}

Task<int> DagSystem::recyclerTest(size_t limit){
	using Branch = std::variant<std::monostate, Branch<int, WorkStealPool>>;
	
	int result = 0;
	size_t count = 0;

	std::array<Branch, 1000> branches{};
	Recycler recycler{branches};
	
	while(count < limit){
		auto recycled_slot = recycler.emplace(co_await threadpool.branch(multiply(1,1)));

		if(recycled_slot.index() == 1){
			result += co_await std::get<1>(recycled_slot);
		}

		count++;
	}

	for(auto& branch : branches){
		if(branch.index() == 1){
			result += co_await std::get<1>(branch);
		}
	}

	co_return result;

}


