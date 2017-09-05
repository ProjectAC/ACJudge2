#include "../include/platform.h"
#include "../include/Main.h"

#if defined _WIN32
int main(int argc, char *args[])
#elif define __unix__

#endif
{
	ACJudge2::Main(T("sol-pic.com:"));
}