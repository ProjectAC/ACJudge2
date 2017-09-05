#pragma once

#include <string>
#include "../include/platform.h"
#include "../include/definitions.h"

#if defined WINDOWS
#include <Windows.h>
#endif

namespace ACJudge2
{
	class Sandbox
	{
	public:
		static Result Run(Tstring cmd, Time time, Memory memory, bool restricted, Tstring fin = T(""), Tstring fout = T(""), Tstring ferr = T(""));

		// [Interface] get path of this sandbox
		static Tstring GetPath();

	private:

#if defined _WIN32

		// Set limits
		// Tags will be saved in 
		static JOBOBJECT_EXTENDED_LIMIT_INFORMATION SetLimits(Time time, Memory memory, bool restricted);

		// Set UI restrictions
		static JOBOBJECT_BASIC_UI_RESTRICTIONS SetRulesUI(bool restricted);

		// Redirect file input/output/error target
		static STARTUPINFO Redirection(Tstring in, Tstring out, Tstring err);

#elif defined _NIX
		// Set time limit
		// Both with setitimer and setrlimit
		// Here time is CPU time, and stime is actual time
		static Return set_time_limit(Time time);

		// Set memory limit
		// With setrlimit
		static Return set_space_limit(Memory memory);

		// Redirect I/O flow <file> to file with name <name>
		static Return set_file(FILE *fp, Tstring file, Tstring mode);

		// Starter
		// This is what the starter have to do before execve
		static void start(Tstring file, Tchar *args[], Time time, Memory memory, bool restricted, Tstring fin, Tstring fout, Tstring ferr);

		// Set time and memory limits for current process
		// Always called immediately after forking
		static Return set_limits(Time time, Memory memory);

		// Set rules (security) for current process
		// Called when <restricted> is true
		// [Thanks] QingDaoU Judger
		// link: https://github.com/QingdaoU/Judger
		static Return set_rules(Tstring s);

		// Set group of current process to guest for safety
		static Return set_gid();

		// Redirect file input/output/error target
		static Return redirection(Tstring in, Tstring out, Tstring err);
#endif
	};
}
