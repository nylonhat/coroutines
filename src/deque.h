#ifndef DEQUE_H
#define DEQUE_H

#include <limits>
#include <bit>
#include <array>
#include <atomic>

template<typename T, size_t buffer_size>
requires
	(buffer_size > 1) &&
	(buffer_size < std::numeric_limits<size_t>::max()/2) && //circular dif
	(std::has_single_bit(buffer_size)) //power of 2

class Deque{

public:
	Deque(){
		for(size_t i = 0; i < buffer_size; i++){
			buffer[i].sequence.store(i, relaxed);
		}
	}
	
	~Deque() = default;


	bool try_local_push(T const& data){
		Cell& cell = buffer[stack_id & buffer_mask];
		size_t seq = cell.sequence.load(acquire);
		
		if(seq != stack_id){
			return false;
		}

		cell.data = data;
		cell.sequence.store(stack_id + 1, release);
		stack_id++;
		return true;
	}

	bool try_local_pop(T& data){
		Cell& cell = buffer[(stack_id - 1) & buffer_mask];

		size_t old_seq = stack_id;
		if(!cell.sequence.compare_exchange_strong(old_seq, stack_id - 1, acq_rel)){
			return false;
		}

		size_t old_steal = stack_id - 1;
		if(steal_id.compare_exchange_strong(old_steal, stack_id, relaxed)){
			data = cell.data;
			cell.sequence.store(old_steal + buffer_size, release);
			return true;
		}

		if(old_steal == stack_id){
			return false;
		}

		data = cell.data;
		stack_id--;
		return true;
	}

	bool try_steal(T& data){
		Cell* cell;
		size_t old_steal = steal_id.load(relaxed);

		for(;;){
			cell = &buffer[old_steal & buffer_mask];
			size_t seq = cell->sequence.load(acquire);

			intptr_t dif = (intptr_t)seq - (intptr_t)(old_steal + 1);

			if(dif < 0){
				return false;
			}
			
			if(dif > 0){
				old_steal = steal_id.load(relaxed);
				continue;
			}

			if(steal_id.compare_exchange_weak(old_steal, old_steal + 1, relaxed)){
				break;
			}
		}

		data = cell->data;
		cell->sequence.store(old_steal + buffer_size, release);
		return true;
	}

private:

	struct alignas(64) Cell{
		std::atomic<size_t> sequence;
		T data;
	};
	
	std::array<Cell, buffer_size> buffer;
	size_t const buffer_mask = buffer_size - 1;
	alignas(64) size_t stack_id = 0;
	alignas(64) std::atomic<size_t> steal_id = 0;

	Deque(Deque const&) = delete;
	void operator= (Deque const&) = delete;

	static constexpr auto acquire = std::memory_order::acquire;
	static constexpr auto release = std::memory_order::release;
	static constexpr auto acq_rel = std::memory_order::acq_rel;
	static constexpr auto relaxed = std::memory_order::relaxed;
};

#endif
