#ifndef RECYCLER_H
#define RECYCLER_H

#include "scheduler.h"
#include <array>
#include <variant>
#include <optional>

template<typename B, size_t SIZE>
struct Recycler {
	std::array<std::optional<B>, SIZE>& array;
	size_t index = 0;

	Recycler(std::array<std::optional<B>, SIZE>& array)
		:array{array}
	{}

	std::optional<B> emplace(B&& branch){
		for(;;){
			auto& slot = array.at(index%SIZE); 
			index++;
			if(!slot){
				slot.template emplace<B>(std::move(branch));
				return {};
			}

			auto& running_branch = *slot;

			if(running_branch.done()){
				B done_branch = std::move(running_branch);
				slot.template emplace<B>(std::move(branch));
				return std::move(done_branch);
			}

		}
	}	

};



#endif

