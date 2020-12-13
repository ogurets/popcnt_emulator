// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <intrin.h>
#include <stdint.h>

#pragma pack(push,8) /* Work around a bug in tlhelp32.h in WIN64, which generates the wrong structure if packing has been changed */
#include <tlhelp32.h>
#pragma pack(pop)

#include <shlwapi.h>
