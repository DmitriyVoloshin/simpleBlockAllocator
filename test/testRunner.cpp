#include "CppUTest/CommandLineTestRunner.h"

int main(int argc, char** argv)
{
	MemoryLeakWarningPlugin::turnOnThreadSafeNewDeleteOverloads();
	RUN_ALL_TESTS(argc, argv);
}
