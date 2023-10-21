#ifndef THROTTLE_CHAIN_H
#define THROTTLE_CHAIN_H

#include <atomic>
#include <immintrin.h>

struct alignas(64) ThrottleNode {
	std::atomic<bool> chained = false;
	std::atomic_flag contended = ATOMIC_FLAG_INIT;
	std::atomic<ThrottleNode*> next_node_ptr = nullptr;

	void setThrottle(bool is_contended){
		if(next_node_ptr == nullptr){
			return;
		}

		if(next_node_ptr.load()->contended.test() == is_contended){
			return;
		}

		if(is_contended){
			next_node_ptr.load()->contended.test_and_set();
			return;
		}

		next_node_ptr.load()->contended.clear();
		next_node_ptr.load()->contended.notify_one();
	}

	void throttle(){
		if(contended.test()){
			contended.wait(true);
		}
	}
};


struct ThrottleChain {
	std::atomic<ThrottleNode*> tail_ptr = nullptr;

	void add(ThrottleNode& new_node){
		auto* prev_node_ptr = tail_ptr.exchange(&new_node);

		if(prev_node_ptr != nullptr){
			new_node.chained = true;
			prev_node_ptr->next_node_ptr = &new_node;
		}

	}

	void remove(ThrottleNode& node_to_remove){

		//wait for previous node to let us be unchained
		node_to_remove.chained.wait(true);

		if(node_to_remove.next_node_ptr == nullptr){
			ThrottleNode* expected = &node_to_remove;
			
			if(tail_ptr.compare_exchange_strong(expected, nullptr)){
				return;
			}

			while(node_to_remove.next_node_ptr == nullptr){
				_mm_pause(); //bust spin wait
			}

		}

		node_to_remove.setThrottle(false);
		node_to_remove.next_node_ptr.load()->chained.store(false);
		node_to_remove.next_node_ptr.load()->chained.notify_one();
		
		node_to_remove.next_node_ptr = nullptr;

	}

};

#endif
