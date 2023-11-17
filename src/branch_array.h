#ifndef BRANCH_ARRAY_H
#define BRANCH_ARRAY_H

#include "scheduler.h"
#include "branch.h"



template<typename T, Scheduler S, size_t SIZE>
std::variant<std::monostate, Branch<T, S>> emplace_in(std::array<std::variant<std::monostate, Branch<T,S>>, SIZE>& branches, Branch<T,S>&& branch, size_t& index){
	for(;;){
		std::variant<std::monostate, Branch<T, S>>& slot = branches.at(index%SIZE); 
		index++;
		if(slot.index() == 0){
			slot.template emplace<1>(std::move(branch));
			return std::monostate();
		}

		auto& running_branch = std::get<1>(slot);

		if(running_branch.done()){
			Branch<T,S> done_branch = std::move(running_branch);
			slot.template emplace<1>(std::move(branch));
			return std::move(done_branch);
		}

	}
}	

#endif

