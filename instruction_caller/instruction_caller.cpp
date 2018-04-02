// instruction_caller.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

int _tmain(int argc, _TCHAR* argv[])
{
	/*unsigned long long res;
	res = __rdtsc();
	printf("Instruction result: %I64u\n", res);*/

	uint64_t *x = new uint64_t;
	*x = 0xFFFFFFFFFFFFFFFFLL;
	uint64_t res = __popcnt64(*x);  // Will crash on non-SSE4 cpus. Good!
	printf("Instruction result, memory: %I64u\n", res);
	for (int i = 0; i < 10; ++i)
		res = __popcnt64(0xFFFFFFFFFFFFFFFFLL);  // Test multiple invocations
	printf("Instruction result, register: %I64u\n", res);
	delete x;
	return 0;
}

