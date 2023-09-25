#ifndef BOUNDED_MPMC_QUEUE_H
#define BOUNDED_MPMC_QUEUE_H

template<typename T>
class bounded_mpmc_queue{

public:
	//constructor
	bounded_mpmc_queue(size_t buffer_size)
		: buffer_(new cell_t [buffer_size])
		, buffer_mask_(buffer_size - 1)
	{

		assert((buffer_size >= 2) && ((buffer_size & (buffer_size - 1)) == 0));

		for(size_t i = 0; i != buffer_size; i += 1){
			buffer_[i].sequence_.store(i, std::memory_order_relaxed);
		}

		enqueue_pos_.store(0, std::memory_order_relaxed);
		dequeue_pos_.store(0, std::memory_order_relaxed);

	}
	
	//destructor
	~bounded_mpmc_queue(){
		delete [] buffer_;
	}


	bool try_enqueue(T const& data){
		cell_t* cell;
		size_t pos = enqueue_pos_.load(std::memory_order_relaxed);

		for(;;){
			cell = &buffer_[pos & buffer_mask_];
			size_t seq = cell->sequence_.load(std::memory_order_acquire);
			intptr_t dif = (intptr_t)seq - (intptr_t)pos;
			
			//We can potentially claim the front position
			if(dif == 0){
				//Try to claim position
				if(enqueue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)){
					//Successfully claimed position
					break;
				}
				//Retry all over
			}
			
			//Fail if queue is full
			else if(dif < 0){
				return false;
			}
			
			//We were beat to the front; try again
			else{
				pos = enqueue_pos_.load(std::memory_order_relaxed);
			}
		}
		
		//Store data into the queue
		cell->data_ = data;
		cell->sequence_.store(pos + 1, std::memory_order_release);
		return true;

	}

	bool try_dequeue(T& data){
		cell_t* cell;
		size_t pos = dequeue_pos_.load(std::memory_order_relaxed);

		for(;;){
			cell = &buffer_[pos & buffer_mask_];
			size_t seq = cell->sequence_.load(std::memory_order_acquire);
			intptr_t dif = (intptr_t)seq - (intptr_t)(pos + 1);
			
			//We can potentially claim the front dequeue position
			if(dif == 0){
				//Try to claim position
				if(dequeue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)){
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
				pos = dequeue_pos_.load(std::memory_order_relaxed);
			}
		}
		
		//Read data out from the queue
		data = cell->data_;
		cell->sequence_.store(pos + buffer_mask_ + 1, std::memory_order_release);
		return true;
	}

private:

	struct alignas(64) cell_t{
		std::atomic<size_t> sequence_;
		T data_;

	};

	static size_t const cacheline_size = 64;
	typedef char cacheline_pad_t [cacheline_size];
	
	//Member variables
	cacheline_pad_t pad0_;
	cell_t* const buffer_;
	size_t const buffer_mask_;
	cacheline_pad_t  pad1_;
	std::atomic<size_t>  enqueue_pos_;
	cacheline_pad_t pad2_;
	std::atomic<size_t> dequeue_pos_;
	cacheline_pad_t pad3_;

	//Copy constructor
	bounded_mpmc_queue(bounded_mpmc_queue const&);
	
	//Copy assigment
	void operator= (bounded_mpmc_queue const&);

}; 


#endif
