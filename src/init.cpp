#include "../include/platform.h"
#include "../include/Main.h"

using namespace ACJudge2;

#if defined _WIN32
int wmain(int argc, Tchar *args[])
#elif define __unix__
int main(int argc, char *args[])
#endif
{
	Main main(args[1]);

	return 0;
}