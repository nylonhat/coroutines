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
	size_t iterations = 1;

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

Task<int> DagSystem::arrayTest(){
	using Spawn = std::variant<std::monostate, Branch<int, WorkStealPool>>;
	
	int result = 0;
	size_t limit = 0;
	std::array<Spawn, 8> branches{};
	Recycler recycler{branches};
	
	while(limit < 1'000'000){
		auto recycled_slot = recycler.emplace(co_await threadpool.branch(permutation()));	
		if(recycled_slot.index() == 1){
			//std::cout << "b: " << &last_slot << " c: " << std::get<1>(last_slot).my_handle.address() << "\n";
			result += co_await std::get<1>(recycled_slot);
		}
		limit++;
	}

	for(auto& branch : branches){
		if(branch.index() == 1){
			//std::cout << "b: " << &branch << " c: " << std::get<1>(branch).my_handle.address() << "\n";
			result += co_await std::get<1>(branch);
		}
	}


	co_return result;

}

Task<int> DagSystem::manyBranch(){
	int result = 0;
	auto b1 = co_await threadpool.branch(multiply(1,1));
	auto b2 = co_await threadpool.branch(multiply(1,1));
	auto b3 = co_await threadpool.branch(multiply(1,1));
	auto b4 = co_await threadpool.branch(multiply(1,1));
	auto b5 = co_await threadpool.branch(multiply(1,1));
	auto b6 = co_await threadpool.branch(multiply(1,1));
	auto b7 = co_await threadpool.branch(multiply(1,1));

	result += co_await b1;
	result += co_await b2;
	result += co_await b3;
	result += co_await b4;
	result += co_await b5;
	result += co_await b6;
	result += co_await b7;

	co_return result;
	
}

Task<int> DagSystem::normalArrayTest(){
	int result = 0;

	std::array<Branch<int, WorkStealPool>, 8> array {
		co_await threadpool.branch(multiply(1,1)),
		co_await threadpool.branch(multiply(1,1)),
		co_await threadpool.branch(multiply(1,1)),
		co_await threadpool.branch(multiply(1,1)),
		co_await threadpool.branch(multiply(1,1)),
		co_await threadpool.branch(multiply(1,1)),
		co_await threadpool.branch(multiply(1,1)),
		co_await threadpool.branch(multiply(1,1))
	};

	for(auto& branch: array){
		result += co_await branch;
	}

	co_return result;
		
}

Task<int> DagSystem::variantArrayTest(){
	using BranchV = std::variant<std::monostate, Branch<int, WorkStealPool>>;
	int result =0;

	std::array<BranchV, 2> branches{};

	for(auto& branchv : branches){
		branchv.emplace<1>( co_await threadpool.branch(multiply(1,1)) );
	}

	for(auto& branchv : branches){
		result += co_await std::get<1>(branchv);
	}

	co_return result;


}

Task<int> DagSystem::benchmark(int iterations){
	
	unsigned int result = 0;
	
	Timer timer;
	timer.start();

	for (int i=0; i< iterations; i++){
		//result = co_await fib(33);
		//result += co_await multiply(i, 1);
		//result += co_await threadpool.chain(multiply(i, 1));
		//result += co_await co_await threadpool.branch(multiply(i, 1));
		//result += co_await threadpool.spawn(multiply(i, 1));
		result += co_await arrayTest();
		//result = co_await vectorTest(2);
		//result += co_await manyBranch();
		//result += co_await normalArrayTest();
		//result += co_await variantArrayTest();
	}

	timer.stop();
	std::cout << timer.count()/iterations << " ns\n";
	
	co_return result;
}
