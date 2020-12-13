// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the POPCNT_HOTPATCH_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// POPCNT_HOTPATCH_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef POPCNT_HOTPATCH_EXPORTS
#define POPCNT_HOTPATCH_API __declspec(dllexport)
#else
#define POPCNT_HOTPATCH_API __declspec(dllimport)
#endif

extern "C" POPCNT_HOTPATCH_API unsigned char icudt65_dat[];
POPCNT_HOTPATCH_API int install_hooks(void);
