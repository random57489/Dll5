#pragma once

class CSignature
{
public:
	DWORD dwFindPattern(DWORD dwAddress, DWORD dwLength, const char* szPattern);
	HMODULE GetModuleHandleSafe(const wchar_t* pszModuleName);
	DWORD GetClientSignature(const char* chPattern);
	DWORD GetEngineSignature(const char* chPattern);
	DWORD FindPatternEx(const wchar_t* name, const char* chPattern);
};

extern CSignature gSignatures;
