#ifndef RECYCLER_H
#define RECYCLER_H

#include "scheduler.h"
#include <array>
#include <variant>
#include <optional>

template<typename B, size_t SIZE>
struct Recycler {
	std::array<B, SIZE>& array;
	size_t index = 0;

	Recycler(std::array<B, SIZE>& array)
		:array{array}
	{}

	B recycle(B&& branch){
		for(;;){
			auto& slot = array.at(index%SIZE); 
			index++;
			if(!slot.done()){
				continue; 
			}
			return std::exchange(slot, std::move(branch));
		}
	}	

};



#endif

