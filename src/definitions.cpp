#include "../include/definitions.h"

using namespace std;
using namespace ACJudge2;
using namespace web;

void ACJudge2::sleep(Time time)
{
#if defined _WIN32
	Sleep(time);
#endif
}