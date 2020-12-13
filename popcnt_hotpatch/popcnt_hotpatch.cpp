// popcnt_hotpatch.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "popcnt_hotpatch.h"

// This is an example of an exported variable
POPCNT_HOTPATCH_API unsigned char icudt65_dat[] = {
	0x20, 0x00, 0xDA, 0x27, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x54, 0x6F, 0x43, 0x50,
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

uint64_t popcnt_reference(uint64_t x)
{
	// bit twiddling method
	uint64_t c;
	for(c = 0; x; c++) {
		x &= x - 1;
	}

	return c;
}

uint64_t GetModuleBase(const char *modname)
{
	HANDLE hModSnap;
	MODULEENTRY32 pe32;
	void *bs = NULL;

	// Take a snapshot of all processes in the system.
	hModSnap = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, GetCurrentProcessId() );
	if( hModSnap == INVALID_HANDLE_VALUE )
	{
		return NULL;
	}

	// Set the size of the structure before using it.
	pe32.dwSize = sizeof( MODULEENTRY32 );

	if( !Module32First( hModSnap, &pe32 ) )
	{
		CloseHandle( hModSnap );     // Must clean up the snapshot object!
		return NULL;
	}

	do
	{
		if (!StrCmpI(pe32.szModule, modname)) {
			bs = pe32.modBaseAddr;
			break;
		}
	} while( Module32Next( hModSnap, &pe32 ) );

	CloseHandle( hModSnap );

	return (uint64_t)bs;
}

uint64_t g_imageBase = 0;

#define HOTFIX(offset, dest, src, instr_size) \
	if (rip == g_imageBase + (offset)) { \
		(dest) = popcnt_reference((src)); \
		ctx->Rip += (instr_size); \
		return EXCEPTION_CONTINUE_EXECUTION; \
	}

LONG NTAPI vec_handler(EXCEPTION_POINTERS *ExceptionInfo)
{
	uint64_t rip = ExceptionInfo->ContextRecord->Rip;
	if (ExceptionInfo->ExceptionRecord->ExceptionCode != 0xc000001d)
		return EXCEPTION_CONTINUE_SEARCH;

	PCONTEXT ctx = ExceptionInfo->ContextRecord;
	
	// 0x2A81550 popcnt rax, qword ptr [rcx]
	HOTFIX(0x2A81550, ctx->Rax, *((uint64_t *)ctx->Rcx), 5);

	// Automatic (from PIN)
	HOTFIX(0x045AD8D, ctx->Rax, ctx->Rcx, 5);
	HOTFIX(0x048AF50, ctx->Rax, *((uint64_t *)ctx->Rcx), 5);
	HOTFIX(0x048AF62, ctx->R10, *((uint64_t *)(ctx->Rsp+0x18)), 7);
	HOTFIX(0x048AF71, ctx->Rax, *((uint64_t *)ctx->Rcx), 5);
	HOTFIX(0x048AF82, ctx->Rax, *((uint64_t *)(ctx->Rsp+0x38)), 7);
	HOTFIX(0x048D120, ctx->Rax, *((uint64_t *)ctx->Rdx), 5);
	HOTFIX(0x048D132, ctx->Rax, *((uint64_t *)(ctx->Rbp-0x41)), 6);
	HOTFIX(0x048D150, ctx->Rax, *((uint64_t *)ctx->Rdx), 5);
	HOTFIX(0x048D162, ctx->Rax, *((uint64_t *)(ctx->Rbp-0x21)), 6);
	HOTFIX(0x048D230, ctx->Rdx, *((uint64_t *)ctx->R8), 5);
	HOTFIX(0x048D242, ctx->Rdx, *((uint64_t *)(ctx->Rbp-0x1)), 6);
	HOTFIX(0x048D260, ctx->Rdx, *((uint64_t *)ctx->R8), 5);
	HOTFIX(0x048D271, ctx->Rdx, *((uint64_t *)(ctx->Rbp+0x1f)), 6);
	HOTFIX(0x048D420, ctx->Rax, *((uint64_t *)ctx->Rcx), 5);
	HOTFIX(0x048D431, ctx->R10, *((uint64_t *)(ctx->Rbp-0x49)), 6);
	HOTFIX(0x048D450, ctx->Rax, *((uint64_t *)ctx->Rcx), 5);
	HOTFIX(0x048D461, ctx->Rax, *((uint64_t *)(ctx->Rbp-0x29)), 6);
	HOTFIX(0x048D580, ctx->Rax, *((uint64_t *)ctx->Rcx), 5);
	HOTFIX(0x048D591, ctx->Rax, *((uint64_t *)(ctx->Rbp-0x9)), 6);
	HOTFIX(0x048D5B0, ctx->Rax, *((uint64_t *)ctx->Rcx), 5);
	HOTFIX(0x048D5C2, ctx->Rax, *((uint64_t *)(ctx->Rbp+0x17)), 6);
	HOTFIX(0x048D9C0, ctx->Rax, *((uint64_t *)ctx->Rdx), 5);
	HOTFIX(0x048D9D2, ctx->Rax, *((uint64_t *)(ctx->Rsp+0x48)), 7);
	HOTFIX(0x048D9F0, ctx->Rax, *((uint64_t *)ctx->Rdx), 5);
	HOTFIX(0x048DA02, ctx->Rax, *((uint64_t *)(ctx->Rsp+0x68)), 7);
	HOTFIX(0x048DAA0, ctx->Rax, *((uint64_t *)ctx->Rdx), 5);
	HOTFIX(0x048DAB2, ctx->Rax, *((uint64_t *)(ctx->Rbp-0x78)), 6);
	HOTFIX(0x048DAD0, ctx->Rax, *((uint64_t *)ctx->Rdx), 5);
	HOTFIX(0x048DAE2, ctx->Rax, *((uint64_t *)(ctx->Rbp-0x58)), 6);
	HOTFIX(0x048DBA0, ctx->Rax, *((uint64_t *)ctx->Rdx), 5);
	HOTFIX(0x048DBB2, ctx->Rax, *((uint64_t *)(ctx->Rbp-0x58)), 6);
	HOTFIX(0x048DBD0, ctx->Rax, *((uint64_t *)ctx->Rdx), 5);
	HOTFIX(0x048DBE2, ctx->Rax, *((uint64_t *)(ctx->Rbp-0x78)), 6);
	HOTFIX(0x048DC80, ctx->Rax, *((uint64_t *)ctx->Rdx), 5);
	HOTFIX(0x048DC92, ctx->Rax, *((uint64_t *)(ctx->Rsp+0x68)), 7);
	HOTFIX(0x048DCB0, ctx->Rax, *((uint64_t *)ctx->Rdx), 5);
	HOTFIX(0x048DCC2, ctx->Rax, *((uint64_t *)(ctx->Rsp+0x48)), 7);
	HOTFIX(0x048DD70, ctx->R8, *((uint64_t *)ctx->R9), 5);
	HOTFIX(0x048DD82, ctx->R8, *((uint64_t *)(ctx->Rbp-0x58)), 6);
	HOTFIX(0x048DDA0, ctx->R8, *((uint64_t *)ctx->R9), 5);
	HOTFIX(0x048DDB2, ctx->R8, *((uint64_t *)(ctx->Rbp-0x78)), 6);
	HOTFIX(0x048DE50, ctx->R8, *((uint64_t *)ctx->R9), 5);
	HOTFIX(0x048DE62, ctx->R8, *((uint64_t *)(ctx->Rsp+0x68)), 7);
	HOTFIX(0x048DE80, ctx->R8, *((uint64_t *)ctx->R9), 5);
	HOTFIX(0x048DE92, ctx->R8, *((uint64_t *)(ctx->Rsp+0x48)), 7);
	HOTFIX(0x048DF70, ctx->R8, *((uint64_t *)ctx->R9), 5);
	HOTFIX(0x048DF82, ctx->R8, *((uint64_t *)(ctx->Rbp-0x38)), 6);
	HOTFIX(0x048DFA0, ctx->R8, *((uint64_t *)ctx->R9), 5);
	HOTFIX(0x048DFB2, ctx->R8, *((uint64_t *)(ctx->Rbp-0x18)), 6);
	HOTFIX(0x048E040, ctx->R8, *((uint64_t *)ctx->R9), 5);
	HOTFIX(0x048E052, ctx->R8, *((uint64_t *)(ctx->Rbp+0x8)), 6);
	HOTFIX(0x048E070, ctx->R8, *((uint64_t *)ctx->R9), 5);
	HOTFIX(0x048E082, ctx->R8, *((uint64_t *)(ctx->Rbp+0x28)), 6);
	HOTFIX(0x048E380, ctx->Rax, *((uint64_t *)ctx->Rcx), 5);
	HOTFIX(0x048E391, ctx->R10, *((uint64_t *)(ctx->Rbp-0x28)), 6);
	HOTFIX(0x048E3B0, ctx->Rax, *((uint64_t *)ctx->Rcx), 5);
	HOTFIX(0x048E3C1, ctx->Rax, *((uint64_t *)(ctx->Rbp-0x8)), 6);
	HOTFIX(0x048E4F0, ctx->Rcx, *((uint64_t *)ctx->Rdx), 5);
	HOTFIX(0x048E502, ctx->Rcx, *((uint64_t *)(ctx->Rbp-0x8)), 6);
	HOTFIX(0x048E520, ctx->Rcx, *((uint64_t *)ctx->Rdx), 5);
	HOTFIX(0x048E532, ctx->Rcx, *((uint64_t *)(ctx->Rbp-0x28)), 6);
	HOTFIX(0x04907E0, ctx->Rax, *((uint64_t *)ctx->Rcx), 5);
	HOTFIX(0x04907F1, ctx->Rax, *((uint64_t *)(ctx->R13+0x30)), 6);
	HOTFIX(0x0492800, ctx->Rax, *((uint64_t *)ctx->Rcx), 5);
	HOTFIX(0x0492819, ctx->Rax, *((uint64_t *)(ctx->R11+ctx->R10*1+0x30)), 7);
	HOTFIX(0x0494FC0, ctx->Rax, *((uint64_t *)ctx->Rcx), 5);
	HOTFIX(0x0494FD1, ctx->R8, *((uint64_t *)(ctx->Rdi+0x30)), 6);
	HOTFIX(0x0496FB0, ctx->Rdx, *((uint64_t *)ctx->Rax), 5);
	HOTFIX(0x0496FC2, ctx->Rax, *((uint64_t *)(ctx->Rcx+0x18)), 6);
	HOTFIX(0x0498950, ctx->Rcx, *((uint64_t *)ctx->Rdx), 5);
	HOTFIX(0x0498961, ctx->Rax, *((uint64_t *)(ctx->Rbp+0x10)), 6);
	HOTFIX(0x0498A70, ctx->Rax, *((uint64_t *)ctx->Rcx), 5);
	HOTFIX(0x0498A80, ctx->Rax, *((uint64_t *)(ctx->Rsp+0x68)), 7);
	HOTFIX(0x0498FF0, ctx->Rax, *((uint64_t *)ctx->Rcx), 5);
	HOTFIX(0x0499001, ctx->Rax, *((uint64_t *)(ctx->R12+ctx->R9*1+0x30)), 7);
	HOTFIX(0x049A690, ctx->Rax, *((uint64_t *)ctx->Rcx), 5);
	HOTFIX(0x049A6A1, ctx->Rax, *((uint64_t *)(ctx->R12+0x30)), 7);
	HOTFIX(0x049DC20, ctx->Rax, *((uint64_t *)ctx->Rcx), 5);
	HOTFIX(0x049DC30, ctx->Rax, *((uint64_t *)(ctx->R12+0x18)), 7);
	HOTFIX(0x049E510, ctx->Rax, *((uint64_t *)ctx->Rcx), 5);
	HOTFIX(0x049E521, ctx->R8, *((uint64_t *)(ctx->Rbx+0x30)), 6);
	HOTFIX(0x04BDB0B, ctx->Rcx, ctx->Rax, 5);

	// HOTFIX(0x0B4945A, ctx->Ecx, *((uint32_t *)ctx->Rcx), 4);
	HOTFIX(0x0B4945A, ctx->Rcx, *((uint32_t *)ctx->Rcx), 4);

	HOTFIX(0x0BBB600, ctx->Rax, *((uint64_t *)ctx->Rcx), 5);
	HOTFIX(0x0BBB635, ctx->Rax, ctx->Rcx, 5);
	HOTFIX(0x0BBBF30, ctx->Rax, *((uint64_t *)ctx->Rdx), 5);
	HOTFIX(0x0BBBF68, ctx->Rax, ctx->R8, 5);
	HOTFIX(0x0BF3440, ctx->Rax, *((uint64_t *)ctx->Rcx), 5);
	HOTFIX(0x0BF3473, ctx->Rax, ctx->Rcx, 5);
	HOTFIX(0x0BF34A0, ctx->Rax, *((uint64_t *)ctx->Rcx), 5);
	HOTFIX(0x0BF34D3, ctx->Rax, ctx->Rdx, 5);
	HOTFIX(0x0BF3A50, ctx->Rax, *((uint64_t *)ctx->Rcx), 5);
	HOTFIX(0x0BF3A86, ctx->Rax, ctx->Rcx, 5);
	HOTFIX(0x0FE391D, ctx->Rcx, *((uint64_t *)(ctx->Rcx+0xa0)), 9);
	HOTFIX(0x116A610, ctx->Rax, *((uint64_t *)ctx->Rcx), 5);
	HOTFIX(0x116A621, ctx->Rax, *((uint64_t *)(ctx->R13+0x48)), 6);
	HOTFIX(0x1189791, ctx->Rax, ctx->Rbx, 5);

	// HOTFIX(0x118CDFF, ctx->Eax, ctx->Edx, 4)
	HOTFIX(0x118CDFF, ctx->Rax, ctx->Rdx & 0xFFFFFFFFULL, 4)
	// HOTFIX(0x11B3612, ctx->Ecx, ctx->Eax, 4)
	HOTFIX(0x11B3612, ctx->Rcx, ctx->Rax & 0xFFFFFFFFULL, 4)
	// HOTFIX(0x11B3616, ctx->Eax, ctx->R9d, 5)
	HOTFIX(0x11B3616, ctx->Rax, ctx->R9 & 0xFFFFFFFFULL, 5)

	HOTFIX(0x1783C60, ctx->Rax, *((uint64_t *)ctx->R8), 5)
	HOTFIX(0x17841F0, ctx->Rax, *((uint64_t *)ctx->Rcx), 5)

	/*HOTFIX(0x17847F0, ctx->Eax, *((uint32_t *)ctx->Rcx), 4)
	HOTFIX(0x17847FF, ctx->Eax, *((uint32_t *)ctx->Rsi+0xfc), 8)
	HOTFIX(0x1B323A8, ctx->Eax, *((uint32_t *)ctx->R15+0x18), 6)
	HOTFIX(0x1B32A22, ctx->Eax, *((uint32_t *)ctx->R14+0x4), 6)
	HOTFIX(0x1B33BC5, ctx->Eax, *((uint32_t *)ctx->R8), 5)
	HOTFIX(0x1B9A8F5, ctx->Eax, *((uint32_t *)ctx->Rcx+0x6c), 5)
	HOTFIX(0x1B9AE51, ctx->Eax, *((uint32_t *)ctx->Rcx+0x98), 8)
	HOTFIX(0x1B9B6C2, ctx->Eax, *((uint32_t *)ctx->Rcx+0x68), 5)
	HOTFIX(0x1B9BC50, ctx->Ecx, *((uint32_t *)ctx->Rdi+0x98), 8)
	HOTFIX(0x22754BD, ctx->Ebx, *((uint32_t *)ctx->Rsi+0x7c), 5)*/
	HOTFIX(0x17847F0, ctx->Rax, *((uint32_t *)ctx->Rcx), 4)
	HOTFIX(0x17847FF, ctx->Rax, *((uint32_t *)(ctx->Rsi+0xfc)), 8)
	HOTFIX(0x1B323A8, ctx->Rax, *((uint32_t *)(ctx->R15+0x18)), 6)
	HOTFIX(0x1B32A22, ctx->Rax, *((uint32_t *)(ctx->R14+0x4)), 6)
	HOTFIX(0x1B33BC5, ctx->Rax, *((uint32_t *)ctx->R8), 5)
	HOTFIX(0x1B9A8F5, ctx->Rax, *((uint32_t *)(ctx->Rcx+0x6c)), 5)
	HOTFIX(0x1B9AE51, ctx->Rax, *((uint32_t *)(ctx->Rcx+0x98)), 8)
	HOTFIX(0x1B9B6C2, ctx->Rax, *((uint32_t *)(ctx->Rcx+0x68)), 5)
	HOTFIX(0x1B9BC50, ctx->Rcx, *((uint32_t *)(ctx->Rdi+0x98)), 8)
	HOTFIX(0x22754BD, ctx->Rbx, *((uint32_t *)(ctx->Rsi+0x7c)), 5)

	HOTFIX(0x2497A00, ctx->Rax, *((uint64_t *)ctx->Rcx), 5)
	HOTFIX(0x2497A37, ctx->Rax, ctx->Rdx, 5)
	HOTFIX(0x2499CB0, ctx->Rax, *((uint64_t *)ctx->Rcx), 5)
	HOTFIX(0x2499CE2, ctx->Rax, ctx->Rdx, 5)
	HOTFIX(0x2499FC0, ctx->Rax, *((uint64_t *)ctx->Rcx), 5)
	HOTFIX(0x2499FF2, ctx->Rax, ctx->Rdx, 5)
	HOTFIX(0x264E0B0, ctx->Rax, *((uint64_t *)ctx->Rcx), 5)
	HOTFIX(0x264E0EA, ctx->Rax, ctx->Rdx, 5)
	HOTFIX(0x270D300, ctx->Rax, *((uint64_t *)ctx->Rcx), 5)
	HOTFIX(0x270D32C, ctx->Rax, ctx->Rcx, 5)

	// TODO: Add your new hotfixes here, the log should help

	FILE *fp = fopen("hotpatch.log", "at");
	fprintf(fp, "Unknown: %016I64X\n", rip - g_imageBase);
	fclose(fp);

	return EXCEPTION_CONTINUE_SEARCH;
}

// This is an example of an exported function.
POPCNT_HOTPATCH_API int install_hooks(void)
{
	g_imageBase = GetModuleBase("Cyberpunk2077.exe");
	AddVectoredExceptionHandler(1, vec_handler);
	return 0;
}
