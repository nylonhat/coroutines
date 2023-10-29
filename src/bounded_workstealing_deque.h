#ifndef BOUNDED_WORKSTEALING_DEQUE_H
#define BOUNDED_WORKSTEALING_DEQUE_H


template<typename T, size_t buffer_size>
class bounded_workstealing_deque{

public:
	//constructor
	bounded_workstealing_deque()
		: buffer_mask(buffer_size - 1)
	{

		static_assert(buffer_size >= 2);
		static_assert((buffer_size & (buffer_size - 1)) == 0);

		for(size_t i = 0; i < buffer_size; i++){
			buffer[i].sequence.store(i, std::memory_order_relaxed);
		}

	}
	
	//destructor
	~bounded_workstealing_deque(){
	}

	
	bool try_local_push(T const& data){
		//check potential cell to push onto local stack end
		cell_t* cell = &buffer[stack_position & buffer_mask];
		size_t seq = cell->sequence.load(std::memory_order_acquire);
		
		//check if deque is full
		if(seq != stack_position){
			//deque is full
			return false;
		}

		//push data into cell
		cell->data = data;

		//update sequence and release info to other dequeuers
		cell->sequence.store(stack_position + 1, std::memory_order_release);
		
		stack_position++;
		return true;
	}

	bool try_local_pop(T& data){
		//check potential cell to pop off local stack end
		cell_t* cell = &buffer[(stack_position-1) & buffer_mask];
		
		//read data knowing it may be invalid
		T optimistic_data = cell->data;
		
		//check if deque is empty
		//might race for last item with stealers
		size_t expected = stack_position;	
		

		//ABA BUG! - stealers may see sequence has not changed
		//when infact it decremented and then incremented
		if(cell->sequence.compare_exchange_strong(expected, stack_position-1)){
			//won race 
			data = optimistic_data;
			stack_position--;
			return true;
		}
		
		//deque was already empty or
		//lost fight with stealer; deque must now be empty
		return false;


	}

	bool try_steal(T& data){
		cell_t* cell;
		T optimistic_data;
		size_t pos = steal_position.load(std::memory_order_seq_cst);

		for(;;){
			cell = &buffer[pos & buffer_mask];
			size_t seq = cell->sequence.load(std::memory_order_seq_cst);
			intptr_t dif = (intptr_t)seq - (intptr_t)(pos + 1);
			
			size_t expected = pos + 1;
			size_t desired = pos + buffer_mask + 1;

			//can we potentially steal
			if(dif == 0){
				optimistic_data = cell->data;

				//race for cell with other stealers and local pop
				if(cell->sequence.compare_exchange_weak(expected, desired)){
					//won race
					steal_position.fetch_add(1, std::memory_order_seq_cst);
					break;
				}

				//else try again
				pos = steal_position.load(std::memory_order_seq_cst);

			}
			
			//deque is empty
			else if(dif < 0){
				return false;
			}

			//we were beat out by another stealer; try again
			else{
				pos = steal_position.load(std::memory_order_seq_cst);
			}
		}

		data = optimistic_data;
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
	size_t stack_position = 0;
	alignas(64) std::atomic<size_t> steal_position = 0;

	//Copy constructor
	bounded_workstealing_deque(bounded_workstealing_deque const&) = delete;
	
	//Copy assigment
	void operator= (bounded_workstealing_deque const&) = delete;

}; 


#endif
