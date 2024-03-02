#include "CommonFunc.h"
#include <sys/timeb.h>

uint64_t GetTimeStampInMS()
{
	//linuxϵͳ
	/*timeval tv;
	timezone tz;

	gettimeofday(&tv, &tz);

	return (tv.tv_sec * 1000000 + tv.tv_usec) / 1000;*/

	timeb now;
	ftime(&now);
	return now.time * 1000 + now.millitm;
}
