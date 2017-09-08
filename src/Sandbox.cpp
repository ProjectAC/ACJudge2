#include <iostream>
#include <fstream>
#include <cpprest/json.h>
#include "../include/Sandbox.h"
#include "../include/Judge.h"

#if defined _WIN32

#include <Psapi.h>
#pragma comment(lib, "Psapi.lib")

using namespace std;
using namespace web;
using namespace ACJudge2;

JOBOBJECT_EXTENDED_LIMIT_INFORMATION Sandbox().SetLimits(Time time, Memory memory, bool restricted)
{
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION ex_lim;

	// Initialize
	ZeroMemory(&ex_lim, sizeof(ex_lim));

	// Memory limit
	if (memory != INFINITE)
	{
		ex_lim.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_JOB_MEMORY;
		ex_lim.JobMemoryLimit = SIZE_T((memory + 1000) * 1000);
	}

	// Time limit
	if (time != INFINITE)
	{
		ex_lim.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_JOB_TIME;
		ex_lim.BasicLimitInformation.PerJobUserTimeLimit.QuadPart = time * 10000;
	}

	if (restricted)
	{
		// Set maximum porcess number
		ex_lim.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_ACTIVE_PROCESS;
		ex_lim.BasicLimitInformation.ActiveProcessLimit = 1;

		// Other limits
		ex_lim.BasicLimitInformation.LimitFlags |=
			JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION |
			JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE |
			JOB_OBJECT_LIMIT_BREAKAWAY_OK;
	}

	return ex_lim;
}

JOBOBJECT_BASIC_UI_RESTRICTIONS Sandbox().SetRulesUI(bool restricted)
{
	JOBOBJECT_BASIC_UI_RESTRICTIONS bs_ui;
	bs_ui.UIRestrictionsClass = 0;
	if (restricted)
	{
		bs_ui.UIRestrictionsClass =
			JOB_OBJECT_UILIMIT_EXITWINDOWS |
			JOB_OBJECT_UILIMIT_READCLIPBOARD |
			JOB_OBJECT_UILIMIT_WRITECLIPBOARD |
			JOB_OBJECT_UILIMIT_SYSTEMPARAMETERS |
			JOB_OBJECT_UILIMIT_DISPLAYSETTINGS |
			JOB_OBJECT_UILIMIT_GLOBALATOMS |
			JOB_OBJECT_UILIMIT_DESKTOP |
			JOB_OBJECT_UILIMIT_HANDLES;
	}
	return bs_ui;
}

STARTUPINFO Sandbox().Redirection(Tstring in, Tstring out, Tstring err)
{
	SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

	// Create StartUpInfo
	STARTUPINFO s = { sizeof(s) };
	ZeroMemory(&s, sizeof(s));
	s.cb = sizeof(STARTUPINFO);
	s.dwFlags = STARTF_USESTDHANDLES;

	// Create file
	if (in != T(""))
	{
		HANDLE fin = CreateFile(in.c_str(),
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			&sa,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			0);
		s.hStdInput = fin;
	}
	if (out != T(""))
	{
		HANDLE fout = CreateFile(out.c_str(),
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			&sa,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			0);
		s.hStdOutput = fout;
	}
	if (err != T(""))
	{
		HANDLE ferr = CreateFile(err.c_str(),
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			&sa,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			0);
		s.hStdError = ferr;
	}

	return s;
}

Result Sandbox().Run(Tstring cmd, Time time, Memory memory, bool restricted, Tstring fin, Tstring fout, Tstring ferr)
{
	// Initialize
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION ex_lim = SetLimits(time, memory, restricted);
	JOBOBJECT_BASIC_UI_RESTRICTIONS bs_ui = SetRulesUI(restricted);
	STARTUPINFO s = Redirection(fin, fout, ferr);
	_PROCESS_INFORMATION p;

	// Init res
	Result res;
	res.time = res.memory = 0;
	res.ret = ResultType::ERR;
	res.val = -1;
	res.msg = T("Unknown system error.");

	// Create JOB object
	HANDLE job = CreateJobObject(NULL, NULL);
	SetInformationJobObject(job, JobObjectExtendedLimitInformation, &ex_lim, sizeof(ex_lim));
	SetInformationJobObject(job, JobObjectBasicUIRestrictions, &bs_ui, sizeof(bs_ui));

	Tchar command[LENGTH];
	try
	{
		Tsprintf(command, cmd.c_str());
	}catch(exception e)
	{
		return res;
	}

	// Create process
	DWORD ret =
		CreateProcess(NULL, command,
			NULL,
			NULL,
			TRUE,
			CREATE_SUSPENDED,
			NULL,
			NULL,
			&s,
			&p);
	if (!ret)
	{
		res.ret = ResultType::ERR;
		res.msg = T("Error while creating subprocess, errcode = ") + ACJudge2::toString(GetLastError());
		return res;
	}

	// Start process
	AssignProcessToJobObject(job, p.hProcess);
	ResumeThread(p.hThread);

	// Start timing
	HANDLE handles[2];
	handles[0] = p.hProcess;
	handles[1] = job;
	ret = WaitForMultipleObjects(2, handles, FALSE, time * 3);

	FILETIME _, __, ___, user;
	SYSTEMTIME suser;
	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessTimes(p.hProcess, &_, &__, &___, &user);
	GetProcessMemoryInfo(p.hProcess, &pmc, sizeof(pmc));
	FileTimeToSystemTime(&user, &suser);

	res.time = suser.wMilliseconds + suser.wSecond * 1000 + suser.wMinute * 60000;
	if (res.time == 0) res.time = 1;
	res.memory = (Memory)(pmc.PeakWorkingSetSize / 1000);

	switch (ret)
	{
	case WAIT_OBJECT_0:
		// Process exited normally
		GetExitCodeProcess(p.hProcess, (LPDWORD)&res.val);
		if (res.val == 1816)  // CPU time TLE
		{
			res.ret = ResultType::TLE;
			res.msg = T("CPU time TLE.");
		}
		else if (res.val == 3221225495u)  // Memory limit exceed
		{
			res.ret = ResultType::MLE;
			res.msg = T("MLE.");
		}
		else if (res.val)  // ResultType value not zero
		{
			Tifstream fin(ferr);
			getline(fin, res.msg, (Tchar)EOF);
			res.ret = ResultType::RTE;
		}
		else  // It seems all right
		{
			res.ret = ResultType::OK;
			res.msg = T("Process exited normally.");
		}
		break;

	case WAIT_TIMEOUT:  // Real time TLE
		res.ret = ResultType::TLE;
		res.msg = T("Real time TLE.");
		break;

	case WAIT_FAILED:  // System error
		res.ret = ResultType::ERR;
		res.msg = T("Error while waiting.");
		break;
	}

	return res;
}

#elif defined __unix__

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <seccomp.h>
#include <fcntl.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include "../Lib/lib.h"
#include "sandbox.h"

using namespace std;
using namespace ACJudge;

ResultType Sandbox().set_time_limit(Time time)
{
	Limit stime = time * 3;
	ITimerVal time_limit;

	time_limit.it_interval.tv_sec = time_limit.it_interval.tv_usec = 0;

	// User mode CPU time
	time_limit.it_value.tv_sec = time / 1000;
	time_limit.it_value.tv_usec = time % 1000 * 1000;
	if (setitimer(ITIMER_VIRTUAL, &time_limit, NULL))
		return ResultType::ERR;

	// All CPU time
	time_limit.it_value.tv_sec = stime / 1000;
	time_limit.it_value.tv_usec = stime % 1000 * 1000;
	if (setitimer(ITIMER_REAL, &time_limit, NULL))
		return ResultType::ERR;

	// setrlimit
	RLimit limit;
	limit.rlim_cur = limit.rlim_max = (rlim_t)(time);
	if (setrlimit(RLIMIT_CPU, &limit) == -1)
		return ResultType::ERR;

	return ResultType::OK;
}

ResultType Sandbox().set_space_limit(Memory memory)
{
	// setrlimit
	RLimit limit;
	limit.rlim_cur = limit.rlim_max = (rlim_t)(memory + 20000000);
	if (setrlimit(RLIMIT_AS, &limit) == -1)
		return ResultType::ERR;
	return ResultType::OK;
}

ResultType Sandbox().set_file(FILE *fp, Tstring file, Tstring mode)
{
	FILE *newfp;
	file = get_path() + file;
	if ((newfp = fopen(file.c_str(), mode.c_str())) == NULL)
		return ResultType::ERR;
	if (dup2(fileno(newfp), fileno(fp)) == -1)
		return ResultType::ERR;
	return ResultType::OK;
}

void Sandbox().start(Tstring file, Tchar *args[], Time time, Memory memory, bool restricted, Tstring fin, Tstring fout, Tstring ferr)
{
	Tstring s = (file[0] == '.' ? get_path() + file : file);
	Tchar *arguments[1000];
	int argc;
	//Tchar const *envp[] ={"PATH=/bin:/usr/bin"), T("TERM=console"), NULL};

	arguments[0] = (Tchar *)s.c_str();
	for (argc = 1; args[argc]; argc++)
		arguments[argc] = args[argc];
	arguments[argc] = NULL;

	if (set_limits(time, memory) == ResultType::ERR)
		exit(ResultType::ERR);
	if (redirection(fin, fout, ferr) == ResultType::ERR)
		exit(ResultType::ERR);
	if (restricted && (set_rules(s) == ResultType::ERR))  // Removed: setgid
		exit(ResultType::ERR);
	if (execvp(s.c_str(), arguments))
	{
		perror("Error");
		exit(ResultType::ERR);
	}
}

ResultType Sandbox().redirection(Tstring in, Tstring out, Tstring err)
{
	if (in != T("") && set_file(stdin, in, T("r")) == ResultType::ERR)
		return ResultType::ERR;
	if (out != T("") && set_file(stdout, out, T("w")) == ResultType::ERR)
		return ResultType::ERR;
	if (err != T("") && set_file(stderr, err, T("w")) == ResultType::ERR)
		return ResultType::ERR;
	return ResultType::OK;
}

ResultType Sandbox().set_limits(Time time, Memory memory)
{
	if (time != INFINITE)
		if (set_time_limit(time) == ResultType::ERR)
			return ResultType::ERR;
	if (memory != INFINITE)
		if (set_space_limit(memory * 1024) == ResultType::ERR)
			return ResultType::ERR;
	return ResultType::OK;
}

ResultType Sandbox().set_gid()
{
	passwd *pwd = getpwnam("nobody");
	const unsigned int groups[] = { pwd->pw_gid };

	if (setgid(pwd->pw_gid) == -1)
		return ResultType::ERR;
	if (setgroups(1, groups) == -1)
		return ResultType::ERR;
	if (setuid(pwd->pw_uid) == -1)
		return ResultType::ERR;

	return ResultType::OK;
}

ResultType Sandbox().set_rules(Tstring file)
{
	// White list
	int whitelist[] = { SCMP_SYS(read), SCMP_SYS(fstat),
		SCMP_SYS(mmap), SCMP_SYS(mprotect),
		SCMP_SYS(munmap), SCMP_SYS(open),
		SCMP_SYS(arch_prctl), SCMP_SYS(brk),
		SCMP_SYS(access), SCMP_SYS(exit_group),
		SCMP_SYS(close) };
	int whitelist_length = sizeof(whitelist) / sizeof(int);
	int i;
	scmp_filter_ctx ctx;

	// Init
	if (!(ctx = seccomp_init(SCMP_ACT_KILL)))
		return ResultType::ERR;

	// Add whitelist
	for (i = 0; i < whitelist_length; i++)
		if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, whitelist[i], 0))
			return ResultType::ERR;

	// Enable execve
	if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(execve), 1, SCMP_A0(SCMP_CMP_EQ, (scmp_datum_t)file.c_str())))
		return ResultType::ERR;
	// Allow FD but 0 1 2 only
	if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 1, SCMP_A0(SCMP_CMP_LE, 2)))
		return ResultType::ERR;

	//Load
	if (seccomp_load(ctx))
		return ResultType::ERR;

	seccomp_release(ctx);
	return ResultType::OK;
}

Result Sandbox().run(Tstring file, Tchar *args[], Time time, Memory memory, bool restricted, Tstring fin, Tstring fout, Tstring ferr)
{
	int starter;
	int pid, status, signal, retval;
	Result res;
	RUsage resource_usage;

	// Init res
	res.time = res.memory = 0;
	res.ret = ResultType::OK;
	res.val = -1;

	// Fork for starter
	if ((starter = fork()) < 0)
		return res.ret = ResultType::ERR, res;
	else if (starter == 0)  // Starter subprocess
	{
		start(file, args, time, memory, restricted, fin, fout, ferr);
		// If this return is accessed, then something wrong must happened
		exit(ResultType::ERR);
	}

	// Else
	// Main process
	// Deal with the return value

	// Wait for the subprocess to quit 
	if ((pid = wait3(&status, 0, &resource_usage)) == -1)
		return res.ret = ResultType::ERR, res;

	// Get CPU time
	res.time = (int)(resource_usage.ru_utime.tv_sec * 1000 +
		resource_usage.ru_utime.tv_usec / 1000 +
		resource_usage.ru_stime.tv_sec * 1000 +
		resource_usage.ru_stime.tv_usec / 1000);
	if (res.time == 0)
		res.time = 1;

	// Get memory
	res.memory = resource_usage.ru_maxrss;

	// Get signal
	if (WIFSIGNALED(status))
	{
		signal = WTERMSIG(status);
		if (signal == SIGALRM)  // Real time TLE
		{
			res.ret = ResultType::TLE;
			res.msg = T("Real time TLE.");
		}
		else if (signal == SIGVTALRM)  // CPU time TLE
		{
			res.ret = ResultType::TLE;
			res.msg = T("CPU time TLE.");
		}
		else if (signal == SIGSEGV)  // Segment fault
			if (memory != INFINITE && res.memory > memory)  // MLE
			{
				res.ret = ResultType::MLE;
				res.msg = T("MLE.");
			}
			else  // Signaled RTE
			{
				res.ret = ResultType::RTE;
				res.msg = T("Signaled RTE.\nStack overflow, NULL pointer or something like that.");
			}
		else if (signal == SIGFPE)
		{
			res.ret = ResultType::RTE;
			res.msg = T("Signaled RTE.\nFloating point error.");
		}
		else if (signal == SIGKILL)
		{
			res.ret = ResultType::MLE;
			res.msg = T("Process killed (Memory Limit Exceed).");
		}
		else
		{
			res.ret = ResultType::RTE;
			res.msg = T("Syscall failed.\nThis might happen when memory limit exceeded.\nBut it also might caused by an illegal syscall, such as fork.");
		}
	}
	else
	{
		if (memory != INFINITE && res.memory > memory)  // ResultType::MLE
		{
			res.ret = ResultType::MLE;
			res.msg = T("MLE.");
		}

		retval = WEXITSTATUS(status);
		if (retval == ResultType::ERR || retval == 256 + ResultType::ERR)  // System ResultType::ERRor
		{
			Tchar *file = new Tchar[1000000];
			Tifstream fin((char *)(get_path() + T("errlog")).c_str());
			fin.getline(file, 1000000, EOF);

			res.ret = ResultType::ERR;
			res.val = retval;
			res.msg = T("System Error #") + i2s(retval) + T(".\nErrlog:\n") + file;
			delete file;
		}
		else if (retval)  //RTE
		{
			Tchar *file = new Tchar[1000000];
			Tifstream fin((char *)(get_path() + T("errlog")).c_str());
			fin.getline(file, 1000000, EOF);

			res.ret = ResultType::RTE;
			res.val = retval;
			res.msg = T("Returned ") + i2s(retval) + T(".\nErrlog:\n") + file;
			delete file;
		}
		else
		{
			res.ret = ResultType::OK;  // All right
			res.val = 0;
			res.msg = T("Process exited normally.");
		}
	}

	return res;
}

#endif