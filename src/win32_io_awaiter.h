#ifndef WIN32_IO_AWAITER_H
#define WIN32_IO_AWAITER_H

#include <coroutine>
#include <functional>
#include <iostream>

namespace win32::io {

template<typename F>
struct Awaiter {
	using value_type = int;

	F func;
	RequestData data{};

	Awaiter(F func)
		:func{func}
	{}

	bool await_ready(){return false;}

	bool await_suspend(std::coroutine_handle<> caller_handle){
		
		data.waiting_handle = caller_handle;

		int error_code = std::invoke(func, data);

		//std::cout << "error code: " << error_code << "\n"; 
		//std::cout << "last error: " << WSAGetLastError() << "\n";		

		if(error_code == 0){
			return true;
		}

		if(WSAGetLastError() == WSA_IO_PENDING){
			return true;
		}

		data.res = error_code;
		return false;
	}

	unsigned int await_resume(){
		return data.res; 
	}

};


}

#endif
