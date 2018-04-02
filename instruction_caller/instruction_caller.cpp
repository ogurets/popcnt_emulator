// instruction_caller.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

int _tmain(int argc, _TCHAR* argv[])
{
	/*unsigned long long res;
	res = __rdtsc();
	printf("Instruction result: %I64u\n", res);*/

	if (0) {
		uint64_t *x = new uint64_t;
		*x = 0x0FFFFFFFFFFFFFFFLL;
		uint64_t res;
		/*res = __popcnt64(*x);  // Will crash on non-SSE4 cpus. Good!
		printf("Instruction result, memory: %I64u\n", res);*/
		res = __popcnt(((uint32_t *)x)[1]);
		printf("Instruction result, smaller memory: %I64u\n", res);
		/*for (int i = 0; i < 10; ++i)
			res = __popcnt64(0xFFFFFFFFFFFFFFFFLL);  // Test multiple invocations
		printf("Instruction result, register: %I64u\n", res);*/
		res = __popcnt(0xFFFFFF);
		printf("Instruction result, smaller register: %I64u\n", res);
		res = __popcnt16(0xFFF);
		printf("Instruction result, even smaller register: %I64u\n", res);
		delete x;
	}

	char cpid[4*4+1];
	__cpuidex((int *)&cpid, 0, 0);
	cpid[4*4] = '\0';
	printf("Cpuid: %s\n", cpid+4);
	return 0;
}

