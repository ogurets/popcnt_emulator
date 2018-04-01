// instruction_caller.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <intrin.h>

int _tmain(int argc, _TCHAR* argv[])
{
	unsigned long long res;
	res = __rdtsc();
	printf("Instruction result: %I64u\n", res);
	return 0;
}

