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
    *out <<  "Number of threads: " << threadCount  << endl;
    *out <<  "===============================================" << endl;
}

uint64_t doinstruction()
{
    *out << "Emulate rdtsc " << endl;
	return 12345678LL;
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
				INS_IsRDTSC(ins);
				INS_InsertCall(ins, IPOINT_AFTER,
								   AFUNPTR(doinstruction),
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
    TRACE_AddInstrumentFunction(Trace, 0);
	
    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}
