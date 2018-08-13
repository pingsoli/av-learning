// this program tests for loading ffmepg library.

#include <iostream>
extern "C" {
#include "libavcodec/avcodec.h"
}

// 2 ways to load external library
// 1) use macro, like following code
#pragma comment(lib, "avcodec.lib")
// 2) specify the library name in VS project.
// Property -> Linker -> Input -> Extra Dependencies, adds you need.

int main(int argc, char* argv[])
{
#ifdef WIN32
	std::cout << "32 bit program" << std::endl;
#else
	std::cout << "64 bit program" << std::endl;
#endif

	std::cout << avcodec_configuration() << std::endl;
	return 0;
}