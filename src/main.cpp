#include "dag_system.h"
//#include "win32_io_system.h"
#include "linux_io_system.h"
#include "monadic.h"
#include <tuple>

Task<int> mult(int a, int b){
	co_return a*b;
}

Task<float> multf(float a, float b){
	co_return a*b;
}

int sub(int a, int b){
	return a-b;
}



Sync<int> maincoro(){
	auto make_tup = [](auto&&... args){
		return std::make_tuple(args...);	
	};

	auto [a, b] = co_await fmap(make_tup, mult(7,5), multf(3.41,1.44));
	std::cout << a << " " << b <<  "\n";
	co_return 0;
}

int main() {
	
/*
	WSADATA wsa_data;
	WSAStartup(MAKEWORD(2,2), &wsa_data);
	
	IOSystem io_system;
	io_system.entry().await();

	WSACleanup();
*/

	
//	IOSystem io_system{};
//	io_system.entry().await();

//	DagSystem dag_system;
//	dag_system.entry().await();

	maincoro().await();

	return 0;

}




