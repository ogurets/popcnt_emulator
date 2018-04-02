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

std::ostream * out = &cerr;

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
    *out << "Emulate popcnt" << endl;
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

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID *v)
{
	if (
		//INS_Category(ins) == 
		INS_Opcode(ins) == XED_ICLASS_POPCNT
		) {
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
