#ifndef RECYCLER_H
#define RECYCLER_H

#include "scheduler.h"
#include <array>
#include <variant>

template<typename B, size_t SIZE>
struct Recycler {
	std::array<std::variant<std::monostate, B>, SIZE>& array;
	size_t index = 0;

	Recycler(std::array<std::variant<std::monostate, B>, SIZE>& array)
		:array{array}
	{}

	std::variant<std::monostate, B> emplace(B&& branch){
		for(;;){
			std::variant<std::monostate, B>& slot = array.at(index%SIZE); 
			index++;
			if(slot.index() == 0){
				slot.template emplace<1>(std::move(branch));
				return std::monostate();
			}

			auto& running_branch = std::get<1>(slot);

			if(running_branch.done()){
				B done_branch = std::move(running_branch);
				slot.template emplace<1>(std::move(branch));
				return std::move(done_branch);
			}

		}
	}	

};



#endif

