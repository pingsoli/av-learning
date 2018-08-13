// this program tests for loading ffmepg library.

#include <iostream>
extern "C" {
#include "libavcodec/avcodec.h"
}

// 2 ways to load external library
// 1) using macro, like following code.
#pragma comment(lib, "avcodec.lib")

// 2) specify the library name in VS project.
// Property -> Linker -> Input -> Extra Dependencies, adds you need.


int main(int argc, char* argv[])
{
// windows platform and non-windows platform macro definition.
#ifdef _WIN32
	#ifdef _WIN64
        std::cout << "Platform: Windows X64 platform" << std::endl;
	#else
		std::cout << "Platform: Windows X86 plarform" << std::endl;
	#endif
#else
	std::cout << "Unknown platform" << std::endl;
#endif

	std::cout << avcodec_configuration() << std::endl;
	return 0;
}
