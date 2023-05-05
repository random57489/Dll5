
#include <Windows.h>
#include "signature.h"
#include <time.h>

#define INRANGE(x,a,b)    (x >= a && x <= b) 
#define getBits( x )    (INRANGE((x&(~0x20)),'A','F') ? ((x&(~0x20)) - 'A' + 0xa) : (INRANGE(x,'0','9') ? x - '0' : 0))
#define getByte( x )    (getBits(x[0]) << 4 | getBits(x[1]))

DWORD CSignature::dwFindPattern(DWORD dwAddress, DWORD dwLength, const char* szPattern)
{
	// Change this to check for multiple matches to be safer. This only needs to run once after every update...
	const char* pat = szPattern;
	DWORD firstMatch = NULL;
	for (DWORD pCur = dwAddress; pCur < dwLength; pCur++)
	{
		if (!*pat) return firstMatch;
		if (*(PBYTE)pat == '\?' || *(BYTE*)pCur == getByte(pat)) {
			if (!firstMatch) firstMatch = pCur;
			if (!pat[2]) return firstMatch;
			if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?') pat += 3;
			else pat += 2;
		}
		else {
			pat = szPattern;
			firstMatch = 0;
		}
	}
	return NULL;
}
//DWORD CSignature::dwFindPattern(BYTE* dwAddress, BYTE* dwLength, const char* szPattern) {
//	DWORD firstMatch;
//	for (BYTE* current = dwAddress; current < dwLength; ++current)
//	{
//		if ((szPattern[index] != '\?' || szPattern[index] != '\?\?') && (*current) == szPattern[index])
//			index+=3;
//		if (index == sizeof(szPattern) - 1)
//			return (int)current - index;
//	}
//}



//===================================================================================
HMODULE CSignature::GetModuleHandleSafe(const wchar_t* pszModuleName)
{
	HMODULE hmModuleHandle = NULL;

	do
	{
		hmModuleHandle = GetModuleHandle(pszModuleName);
		Sleep(1);
	} while (hmModuleHandle == NULL);

	return hmModuleHandle;
}
//===================================================================================
DWORD CSignature::GetClientSignature(const char* chPattern)
{
	static HMODULE hmModule = GetModuleHandleSafe(L"client.dll");
	static PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)hmModule;
	static PIMAGE_NT_HEADERS pNTHeaders = (PIMAGE_NT_HEADERS)(((DWORD)hmModule) + pDOSHeader->e_lfanew);
	return dwFindPattern(((DWORD)hmModule) + pNTHeaders->OptionalHeader.BaseOfCode, ((DWORD)hmModule) + pNTHeaders->OptionalHeader.SizeOfCode, chPattern);
}
//===================================================================================
DWORD CSignature::GetEngineSignature(const char* chPattern)
{
	static HMODULE hmModule = GetModuleHandleSafe(L"engine.dll");
	static PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)hmModule;
	static PIMAGE_NT_HEADERS pNTHeaders = (PIMAGE_NT_HEADERS)(((DWORD)hmModule) + pDOSHeader->e_lfanew);
	return dwFindPattern(((DWORD)hmModule) + pNTHeaders->OptionalHeader.BaseOfCode, ((DWORD)hmModule) + pNTHeaders->OptionalHeader.SizeOfCode, chPattern);
}

DWORD CSignature::FindPatternEx(const wchar_t* name, const char* chPattern)
{
	static HMODULE hmModule = GetModuleHandleSafe(name);
	static PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)hmModule;
	static PIMAGE_NT_HEADERS pNTHeaders = (PIMAGE_NT_HEADERS)(((DWORD)hmModule) + pDOSHeader->e_lfanew);
	return dwFindPattern(((DWORD)hmModule) + pNTHeaders->OptionalHeader.BaseOfCode, ((DWORD)hmModule) + pNTHeaders->OptionalHeader.SizeOfCode, chPattern);
}
CSignature gSignatures;