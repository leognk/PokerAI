#ifndef OPT_MEMORY_H
#define OPT_MEMORY_H

#include "windows.h"
#include "psapi.h"
#include "StringManip.h"

namespace opt {

inline DWORDLONG totalVirtualMem()
{
	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);
	return memInfo.ullTotalPageFile;
}

inline DWORDLONG virtualMemUsed()
{
	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);
	return memInfo.ullTotalPageFile - memInfo.ullAvailPageFile;
}

inline SIZE_T virtualMemUsedByMe()
{
	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
	return pmc.PrivateUsage;
}

inline DWORDLONG totalPhysMem()
{
	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);
	return memInfo.ullTotalPhys;
}

inline DWORDLONG physMemUsed()
{
	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);
	return memInfo.ullTotalPhys - memInfo.ullAvailPhys;
}

inline SIZE_T physMemUsedByMe()
{
	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
	return pmc.WorkingSetSize;
}

inline std::string vmUsedByMeStr(const unsigned precision = 1)
{
	return opt::prettyNum(opt::virtualMemUsedByMe(), precision, true) + "o";
}

inline std::string ramUsedByMeStr(const unsigned precision = 1)
{
	return opt::prettyNum(opt::physMemUsedByMe(), precision, true) + "o";
}

} // opt

#endif // OPT_MEMORY_H