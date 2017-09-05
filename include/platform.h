#pragma once

#include <fstream>
#include <string>
#include <sstream>
#include <cstring>

#if defined _WIN32
#include <Windows.h>
#endif

namespace ACJudge2
{
#if defined _WIN32

#define T(x) L##x
#define Tsprintf wsprintf
#define Tprintf wprintf
#define Tscanf wscanf
#define Tcin wcin
#define Tcout wcout
#define Tistringstream wistringstream

	typedef std::wstring Tstring;
	typedef std::wofstream Tofstream;
	typedef std::wifstream Tifstream;
	typedef std::wistringstream TISStream;
	typedef wchar_t Tchar;
#endif
};