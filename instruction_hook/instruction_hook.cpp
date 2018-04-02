// instruction_hook.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <intrin.h>

// The running count of instructions is kept here
// make it static to help the compiler optimize docount
static uint64_t icount = 0;
UINT64 insCount = 0;        //number of dynamically executed instructions
UINT64 bblCount = 0;        //number of dynamically executed basic blocks
UINT64 threadCount = 0;     //total number of threads, including main thread
UINT64 invokes = 0;

#ifdef _WIN64
	std::ofstream sout("inshook.log");
	std::ofstream * out = &sout;
#else
	std::ostream * out = &cerr;
#endif

// This function is called before every block
// Use the fast linkage for calls
void PIN_FAST_ANALYSIS_CALL docount(ADDRINT c) { icount += c; }

/* ===================================================================== */
// Command line switches
/* ===================================================================== */
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE,  "pintool",
    "o", "", "specify file name for MyPinTool output");

KNOB<BOOL>   KnobCount(KNOB_MODE_WRITEONCE,  "pintool",
    "count", "1", "count instructions, basic blocks and threads in the application");

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

int32_t Usage()
{
    cerr << "This tool counts the number of dynamic instructions executed" << endl;
	cerr << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/*!
 * Print out analysis results.
 * This function is called when the application exits.
 * @param[in]   code            exit code of the application
 * @param[in]   v               value specified by the tool in the 
 *                              PIN_AddFiniFunction function call
 */
VOID Fini(INT32 code, VOID *v)
{
    *out <<  "===============================================" << endl;
    *out <<  "MyPinTool analysis results: " << endl;
    *out <<  "Number of instructions: " << insCount  << endl;
    *out <<  "Number of basic blocks: " << bblCount  << endl;
    //*out <<  "Number of threads: " << threadCount  << endl;
	*out <<  "Number of invocations: " << invokes  << endl;
    *out <<  "===============================================" << endl;

	#ifdef _WIN64
		sout.close();
	#endif
}

/*uint64_t doinstruction()
{
    *out << "Emulate rdtsc " << endl;
	return 12345678LL;
}*/

// https://stackoverflow.com/questions/17518774/sse-4-popcount-for-16-8-bit-values
// http://kent-vandervelden.blogspot.ru/2009/10/counting-bits-population-count-and.html
uint64_t PIN_FAST_ANALYSIS_CALL dopopcnt(BOOL memop, ADDRINT * memaddr, UINT32 memsize, PIN_REGISTER* regvalue)
{
    //*out << "Emulate popcnt" << endl;
	++invokes;

	// Deal with the different operand types
	uint64_t x;
	if (memop) {
		// Get value from memory
		x = 0;
		PIN_SafeCopy(&x, memaddr, memsize);
	} else {
		switch(memsize) {
			case 8:
				x = regvalue->byte[0];
				break;
			case 16:
				x = regvalue->word[0];
				break;
			case 32:
				x = regvalue->dword[0];
				break;
			case 64:
				x = regvalue->qword[0];
				break;
			default:
				*out << "Unknown operand size: " << memsize << endl;
		}
	}

	// bit twiddling method
	uint64_t c;
	for(c = 0; x; c++) {
		x &= x - 1;
	}
	return c;
}

// https://software.intel.com/en-us/articles/intel-64-architecture-processor-topology-enumeration/
// This doesn't work as planned! The program thinks it runs on real Nehalem and tries to use lots of other non-implemented instructions
#define RETURN_REGS(a,b,d,c) rax->dword[0] = (a); rbx->dword[0] = (b); rdx->dword[0] = (d); rcx->dword[0] = (c);
void PIN_FAST_ANALYSIS_CALL docpuid_1(PIN_REGISTER* rax, PIN_REGISTER* rbx, PIN_REGISTER* rdx, PIN_REGISTER* rcx)
{
    // Nehalem tables
    /*
        00000000 ******** => 0000000b 756e6547 6c65746e 49656e69
        00000001 ******** => 000106a5 06100800 009ce3bd bfebfbff
        00000002 ******** => 55035a01 00f0b2e4 00000000 09ca212c
        00000004 00000000 => 1c004121 01c0003f 0000003f 00000000
        00000004 00000001 => 1c004122 00c0003f 0000007f 00000000
        00000004 00000002 => 1c004143 01c0003f 000001ff 00000000
        00000004 00000003 => 1c03c163 03c0003f 00001fff 00000002
        00000005 ******** => 00000040 00000040 00000003 00001120
        00000006 ******** => 00000003 00000002 00000001 00000000
        00000007 00000000 => 00000000 00000000 00000000 00000000
        0000000a ******** => 07300403 00000044 00000000 00000603
        0000000b 00000000 => 00000001 00000002 00000100 00000006
        0000000b 00000001 => 00000004 00000008 00000201 00000006
        0000000b 00000002 => 00000000 00000000 00000002 00000006
        0000000b 00000003 => 00000000 00000000 00000003 00000006
        0000000b 00000004 => 00000000 00000000 00000004 00000006
        0000000c ******** => 00000001 00000002 00000100 00000006
        0000000d 00000000 => 00000001 00000002 00000100 00000006
        0000000d 00000001 => 00000004 00000008 00000201 00000006
        0000000d 00000002 => 00000000 00000000 00000002 00000006
        0000000d 00000003 => 00000000 00000000 00000003 00000006
        0000000d 00000004 => 00000000 00000000 00000004 00000006
        0000000d 00000005 => 00000000 00000000 00000005 00000006
        0000000d 00000006 => 00000000 00000000 00000006 00000006
        0000000d 00000007 => 00000000 00000000 00000007 00000006
        0000000d 00000008 => 00000000 00000000 00000008 00000006
        80000000 ******** => 80000008 00000000 00000000 00000000
        80000001 ******** => 00000000 00000000 00000001 28100800
        80000002 ******** => 65746e49 2952286c 6f655820 2952286e
        80000003 ******** => 55504320 20202020 20202020 45202020
        80000004 ******** => 30323535 20402020 37322e32 007a4847
        80000006 ******** => 00000000 00000000 01006040 00000000
        80000007 ******** => 00000000 00000000 00000000 00000100
        80000008 ******** => 00003028 00000000 00000000 00000000
    */
    switch (rax->dword[0]) {
        case 0x00000000:
            RETURN_REGS(0x0000000b, 0x756e6547, 0x6c65746e, 0x49656e69);
            break;
        case 0x00000001:
            RETURN_REGS(0x000106a5, 0x06100800, 0x009ce3bd, 0xbfebfbff);
            break;
        case 0x00000002:
            RETURN_REGS(0x55035a01, 0x00f0b2e4, 0x00000000, 0x09ca212c);
            break;
        case 0x00000004:
            switch (rcx->dword[0]) {
                case 0x00000000:
                    RETURN_REGS(0x1c004121, 0x01c0003f, 0x0000003f, 0x00000000);
                    break;
                case 0x00000001:
                    RETURN_REGS(0x1c004122, 0x00c0003f, 0x0000007f, 0x00000000);
                    break;
                case 0x00000002:
                    RETURN_REGS(0x1c004143, 0x01c0003f, 0x000001ff, 0x00000000);
                    break;
                case 0x00000003:
                    RETURN_REGS(0x1c03c163, 0x03c0003f, 0x00001fff, 0x00000002);
                    break;
            }
            break;
        case 0x00000005:
            RETURN_REGS(0x00000040, 0x00000040, 0x00000003, 0x00001120);
            break;
        case 0x00000006:
            RETURN_REGS(0x00000003, 0x00000002, 0x00000001, 0x00000000);
            break;
        case 0x00000007:
            // FIXME: has only 1 subleaf == 0x00000000
            RETURN_REGS(0x00000000, 0x00000000, 0x00000000, 0x00000000);
            break;
        case 0x0000000a:
            RETURN_REGS(0x07300403, 0x00000044, 0x00000000, 0x00000603);
            break;
        case 0x0000000b:
            switch (rcx->dword[0]) {
                case 0x00000000:
                    RETURN_REGS(0x00000001, 0x00000002, 0x00000100, 0x00000006);
                    break;
                case 0x00000001:
                    RETURN_REGS(0x00000004, 0x00000008, 0x00000201, 0x00000006);
                    break;
                case 0x00000002:
                    RETURN_REGS(0x00000000, 0x00000000, 0x00000002, 0x00000006);
                    break;
                case 0x00000003:
                    RETURN_REGS(0x00000000, 0x00000000, 0x00000003, 0x00000006);
                    break;
                case 0x00000004:
                    RETURN_REGS(0x00000000, 0x00000000, 0x00000004, 0x00000006);
                    break;
            }
            break;
        case 0x0000000c:
            RETURN_REGS(0x00000001, 0x00000002, 0x00000100, 0x00000006);
            break;
        case 0x0000000d:
            switch (rcx->dword[0]) {
                case 0x00000000:
                    RETURN_REGS(0x00000001, 0x00000002, 0x00000100, 0x00000006);
                    break;
                case 0x00000001:
                    RETURN_REGS(0x00000004, 0x00000008, 0x00000201, 0x00000006);
                    break;
                case 0x00000002:
                    RETURN_REGS(0x00000000, 0x00000000, 0x00000002, 0x00000006);
                    break;
                case 0x00000003:
                    RETURN_REGS(0x00000000, 0x00000000, 0x00000003, 0x00000006);
                    break;
                case 0x00000004:
                    RETURN_REGS(0x00000000, 0x00000000, 0x00000004, 0x00000006);
                    break;
                case 0x00000005:
                    RETURN_REGS(0x00000000, 0x00000000, 0x00000005, 0x00000006);
                    break;
                case 0x00000006:
                    RETURN_REGS(0x00000000, 0x00000000, 0x00000006, 0x00000006);
                    break;
                case 0x00000007:
                    RETURN_REGS(0x00000000, 0x00000000, 0x00000007, 0x00000006);
                    break;
                case 0x00000008:
                    RETURN_REGS(0x00000000, 0x00000000, 0x00000008, 0x00000006);
                    break;
            }
            break;
        case 0x80000000:
            RETURN_REGS(0x80000008, 0x00000000, 0x00000000, 0x00000000);
            break;
        case 0x80000001:
            RETURN_REGS(0x00000000, 0x00000000, 0x00000001, 0x28100800);
            break;
        case 0x80000002:
            RETURN_REGS(0x65746e49, 0x2952286c, 0x6f655820, 0x2952286e);
            break;
        case 0x80000003:
            RETURN_REGS(0x55504320, 0x20202020, 0x20202020, 0x45202020);
            break;
        case 0x80000004:
            RETURN_REGS(0x30323535, 0x20402020, 0x37322e32, 0x007a4847);
            break;
        case 0x80000006:
            RETURN_REGS(0x00000000, 0x00000000, 0x01006040, 0x00000000);
            break;
        case 0x80000007:
            RETURN_REGS(0x00000000, 0x00000000, 0x00000000, 0x00000100);
            break;
        case 0x80000008:
            RETURN_REGS(0x00003028, 0x00000000, 0x00000000, 0x00000000);
            break;
    }
}

// https://msdn.microsoft.com/en-us/library/hskdteyh.aspx
void PIN_FAST_ANALYSIS_CALL docpuid(PIN_REGISTER* rax, PIN_REGISTER* rbx, PIN_REGISTER* rdx, PIN_REGISTER* rcx)
{
    int regs[4];
    __cpuidex(regs, rax->dword[0], rcx->dword[0]);
    // Apply patch here
    if (rax->dword[0] == 1) {
        regs[2] |= (1 << 23); // popcnt instruction support
    }
	rax->s_dword[0] = regs[0];
	rbx->s_dword[0] = regs[1];
	rcx->s_dword[0] = regs[2];
	rdx->s_dword[0] = regs[3];
	//PIN_Detach();
}

// Pin calls this function every time a new instruction is encountered
// TODO: http://gurmeet.net/puzzles/fast-bit-counting-routines/
VOID Instruction(INS ins, VOID *v)
{
	if (INS_Opcode(ins) == XED_ICLASS_POPCNT) {
		++insCount;
		bool ismem = INS_OperandIsMemory(ins, 1);
		if (ismem) {
			INS_InsertCall(ins, IPOINT_BEFORE,
								AFUNPTR(dopopcnt),
								IARG_FAST_ANALYSIS_CALL,
								IARG_RETURN_REGS, INS_OperandReg(ins, 0),
								//IARG_ADDRINT, INS_MemoryOperandCount(ins),  // https://stackoverflow.com/questions/48066707/how-to-get-memory-operation-values-using-intel-pin?rq=1
								IARG_BOOL, ismem,
								IARG_MEMORYOP_EA, 0,
								IARG_UINT32, ismem ? INS_MemoryOperandSize(ins, 0) : 0,
								IARG_REG_CONST_REFERENCE, INS_OperandReg(ins, 0),
								IARG_END);
		} else {
			INS_InsertCall(ins, IPOINT_BEFORE,
								AFUNPTR(dopopcnt),
								IARG_FAST_ANALYSIS_CALL,
								IARG_RETURN_REGS, INS_OperandReg(ins, 0),
								IARG_BOOL, ismem,
								IARG_ADDRINT, 0,
								IARG_UINT32, INS_OperandWidth(ins, 1),
								IARG_REG_CONST_REFERENCE, INS_OperandReg(ins, 1),
								IARG_END);
		}
		INS_Delete(ins);
		// TODO: PIN_Detach(); after certain number of patched instructions
	} else if (INS_Opcode(ins) == XED_ICLASS_CPUID) {
        INS_InsertCall(ins, IPOINT_BEFORE,
                            AFUNPTR(docpuid),
                            IARG_FAST_ANALYSIS_CALL,
                            IARG_REG_REFERENCE, REG_GAX,
                            IARG_REG_REFERENCE, REG_GBX,
                            IARG_REG_REFERENCE, REG_GDX,
                            IARG_REG_REFERENCE, REG_GCX,
                            IARG_END);
		INS_Delete(ins);
	}
}

VOID Trace(TRACE trace, VOID *v)
{
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
		++bblCount;
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
			if(INS_IsRDTSC(ins)) {
				++insCount;
				INS_InsertCall(ins, IPOINT_AFTER,
								   AFUNPTR(dopopcnt),
								   IARG_RETURN_REGS, REG_GAX,
								   IARG_END);
				INS_Delete(ins);
			}
		}
	}
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

// C:\bin\pin-2.14-71313-msvc10-windows\pin.exe -t instruction_hook.dll -- instruction_caller.exe

int main(int argc, char * argv[])
{
    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    // Register Instruction to be called to instrument instructions
    //TRACE_AddInstrumentFunction(Trace, 0);

	// Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);
	
    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}
