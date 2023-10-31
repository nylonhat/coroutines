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
		//check cell to pop off stack end
		cell_t* cell = &buffer[(stack_position-1) & buffer_mask];

		//preemptively modify the cell sequence if deque not empty
		//will signal any far away stealers that queue is empty now
		size_t expected_seq = stack_position;
		if(cell->sequence.compare_exchange_strong(expected_seq, stack_position-1)){
			
			//attempt to race with potential stealers
			size_t expected_steal = stack_position-1;
			if(steal_position.compare_exchange_strong(expected_steal, stack_position, std::memory_order_relaxed)){
				//stole last item in deque; beating stealer
				data = cell->data;
				steal_position.store(expected_steal+buffer_mask+1, std::memory_order_release);
				stack_position--;
				return true;
			}
			
			//note: expected_steal is now loaded with value from CAS
			if(expected_steal == stack_position){
				//lost race to stealer for the last item
				return false;
			}

			//item is not the last item
			//free to take because lagging stealers think deque is empty
			data = cell->data;
			stack_position--;
			return true;
		}
		
		//deque is empty
		return false;
	}

	bool try_steal(T& data){
		cell_t* cell;
		size_t pos = steal_position.load(std::memory_order_relaxed);

		for(;;){
			cell = &buffer[pos & buffer_mask];
			size_t seq = cell->sequence.load(std::memory_order_acquire);
			intptr_t dif = (intptr_t)seq - (intptr_t)(pos + 1);
			
			//We can potentially claim the front dequeue position
			if(dif == 0){
				//Try to claim position
				if(steal_position.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)){
					//Successfully claimed position
					break;
				}
				//Else try all over again
			}
			
			//Fail if queue is empty
			else if(dif < 0){
				return false;
			}
			
			//We were beat out to the front; try again
			else{
				pos = steal_position.load(std::memory_order_relaxed);
			}
		}
		
		//Read data out from the queue
		data = cell->data;
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
	size_t stack_position = 0;
	alignas(64) std::atomic<size_t> steal_position = 0;

	//Copy constructor
	bounded_workstealing_deque(bounded_workstealing_deque const&) = delete;
	
	//Copy assigment
	void operator= (bounded_workstealing_deque const&) = delete;

}; 


#endif
