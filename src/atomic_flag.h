#ifndef ATOMIC_FLAG_H
#define ATOMIC_FLAG_H

#ifdef _WIN32
 	#include "windows.h"
#else
	#include <linux/futex.h>
	#include <sys/syscall.h>
	#include <unistd.h>
#endif


#include <atomic>

struct AtomicFlag {
	#ifdef _WIN32
	std::atomic<bool> boolean_flag {false};
	#else
	std::atomic<unsigned int> boolean_flag{0};
	#endif

	void wait(){
		#ifdef _WIN32
		bool undesired_value = false;
		#else
		unsigned int undesired_value = 0;
		#endif
		
		
		while(boolean_flag.load() == undesired_value){
			#ifdef _WIN32
			WaitOnAddress(&boolean_flag, &undesired_value, sizeof(boolean_flag), INFINITE);
			#else
			syscall(SYS_futex, &boolean_flag, FUTEX_WAIT, undesired_value, NULL, NULL, 0);	
			#endif
			
		}
	}

	void signal(){
		if (!boolean_flag.exchange(true)){
			#ifdef _WIN32
			WakeByAddressAll(&boolean_flag);
			#else
			syscall(SYS_futex, &boolean_flag, FUTEX_WAKE, INT_MAX, NULL, NULL, 0);
			#endif
		}
		
	}

	void reset(){
		boolean_flag.store(false);
	}
 

	bool load(){
		return boolean_flag.load();
	}
};

#endif