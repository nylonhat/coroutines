#ifndef BACKOFF_H
#define BACKOFF_H

#include <immintrin.h>
#include <algorithm>
#include <random>

struct alignas(64) Backoff {
	const unsigned int min_backoff_count = 0;
	const unsigned int max_backoff_count = 26;
	unsigned int backoff_count = 0;

	std::minstd_rand random_generator{std::random_device{}()};

	void backoff(){
		if (backoff_count == max_backoff_count){
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			return;
		}

		std::uniform_int_distribution<unsigned int> distribution(0, (1<<backoff_count)-1 );
		unsigned int random_iterations = distribution(random_generator);
		
		for (unsigned int i = 0; i < random_iterations; i++){
			_mm_pause();
		}

		backoff_count = std::min(max_backoff_count, backoff_count + 1);
	}

	void reset(){
		backoff_count = min_backoff_count;
	}

	void easein(){
		backoff_count = std::max(min_backoff_count, backoff_count - 2);
	}



};

#endif
