#ifndef BOUNDED_WORKSTEALING_DEQUE_H
#define BOUNDED_WORKSTEALING_DEQUE_H

#include <limits>

template<typename T, size_t buffer_size>
class bounded_workstealing_deque{

public:
	//constructor
	bounded_workstealing_deque()
		: buffer_mask(buffer_size - 1)
	{

		static_assert(buffer_size >= 2);
		static_assert((buffer_size & (buffer_size - 1)) == 0);
  static_assert(buffer_size < std::numerics_limit<size_t>::max()/2)

		for(size_t i = 0; i < buffer_size; i++){
			buffer[i].sequence.store(i, std::memory_order_relaxed);
		}

	}
	
	//destructor
	~bounded_workstealing_deque(){
	}

	
	bool try_local_push(T const& data){
		//Check potential cell to push onto local stack end
		cell_t* cell = &buffer[stack_position & buffer_mask];
		size_t seq = cell->sequence.load(std::memory_order_acquire);
		
		//Check if deque is full
		if(seq != stack_position){
			//Deque is full
			return false;
		}

		cell->data = data;

		//Release info to other dequeuers that cell has new data
		cell->sequence.store(stack_position + 1, std::memory_order_release);
		
		stack_position++;
		return true;
	}

	bool try_local_pop(T& data){
		//Check potential cell to pop off stack end
		cell_t* cell = &buffer[(stack_position-1) & buffer_mask];

		//Preemptively reverse the cell sequence if deque is not empty
		//Will signal any lagging stealers that queue is empty now
		size_t expected_seq = stack_position;
		if(cell->sequence.compare_exchange_strong(expected_seq, stack_position-1, std::memory_order_acq_rel)){
			
			//Attempt to race with any potential stealers
			size_t expected_steal = stack_position-1;
			if(steal_position.compare_exchange_strong(expected_steal, stack_position, std::memory_order_relaxed)){
				//Beat stealers, meaning we stole the last item from the deque
				data = cell->data;

				//Modify cell sequence to be as if we stole like a stealer
				cell->sequence.store(expected_steal+buffer_mask+1, std::memory_order_release);
				return true;
			}
			
			//Check if we failed to get last item in deque
			//Note: expected_steal is now loaded with value from previous CAS
			if(expected_steal == stack_position){
				//Lost race to stealer for the last item
				//Deque must now be empty
				return false;
			}

			//Item was not the last item
			//Free to take because lagging stealers think the deque is empty
			data = cell->data;
			stack_position--;
			return true;
		}
		
		//Deque is empty
		return false;
	}

	bool try_steal(T& data){
		cell_t* cell;
		size_t pos = steal_position.load(std::memory_order_relaxed);

		for(;;){
			//Check potential cell to steal from
			cell = &buffer[pos & buffer_mask];
			size_t seq = cell->sequence.load(std::memory_order_acquire);
			intptr_t dif = (intptr_t)seq - (intptr_t)(pos + 1);
			
			//We can potentially claim the steal position
			if(dif == 0){
				//Race for the steal position
				if(steal_position.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)){
					//Success: we are free to steal item from deque
					break;
				}
				//We lost the race; try all over again
			}
			
			//Deque is empty
			else if(dif < 0){
				return false;
			}
			
			//We are lagging behind the other stealers; try again
			else{
				pos = steal_position.load(std::memory_order_relaxed);
			}
		}
		
		data = cell->data;

		//Release info to others that cell has been stolen
		cell->sequence.store(pos + buffer_mask + 1, std::memory_order_release);
		return true;
			

	}

	
private:

	struct alignas(64) cell_t{
		std::atomic<size_t> sequence;
		T data;
	};

	
	//Member variables
	cell_t buffer[buffer_size];
	size_t const buffer_mask;
	alignas(64) size_t stack_position = 0;
	alignas(64) std::atomic<size_t> steal_position = 0;

	//Copy constructor
	bounded_workstealing_deque(bounded_workstealing_deque const&) = delete;
	
	//Copy assigment
	void operator= (bounded_workstealing_deque const&) = delete;

}; 


#endif
