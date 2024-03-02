#include "Log.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
//#include <unistd.h>
#include <io.h>
#include <time.h>

char logFileName[LEN_LOGFILE_NAME] = "";

void WriteLog(const char* szContent, ...)
{
	char szResult[LEN_LOG] = { 0 };
	va_list args;
	va_start(args, szContent);
	//vsnprintf_s(szResult, LEN_LOG, szContent, args); // windows
	vsnprintf(szResult, LEN_LOG, szContent, args);
	va_end(args);
	//sprintf_s(szResult, "%s\n", szResult);  //windows
	sprintf(szResult, "%s\n", szResult);
#if ((defined DEBUG) || (defined _DEBUG))
	OutputDebugString(szResult);
#endif // DEBUG
	printf("%s", szResult);

	FILE* logFile = nullptr;
	char logPath[LEN_FILE_PATH] = { 0 };
	sprintf(logPath, "./%s_log", logFileName);
	//linuxϵͳ
	/*if (_access(logPath, 0) != 0)
	{
		char cmdStr[256] = { 0 };
		sprintf(cmdStr, "mkdir %s", logPath);
		system(cmdStr);
	}*/
	if (_access(logPath, 0) != 0)
	{
		if (!CreateDirectory(logPath, NULL))
			WriteLog("%s:%d, create filepath:%s fail", __FILE__, __LINE__, logPath);
	}

	char fileName[LEN_FILE] = { 0 };
	char timeFormat[256] = { 0 };
	time_t t;
	time(&t);
	tm* currTime = localtime(&t);
	sprintf(timeFormat, "%04d-%02d-%02d", currTime->tm_year + 1900, currTime->tm_mon + 1, currTime->tm_mday);
	sprintf(fileName, "%s %s.log", timeFormat, logFileName);
	sprintf(logPath, "%s//%s", logPath, fileName);

	for (int i = 0; i < 3; ++i)
	{
		logFile = fopen(logPath, "ab");
		if (logFile)
			break;
		Sleep(1);
	}

	if (logFile)
	{
		char cont[LEN_LOG] = { 0 };
		sprintf(cont, "[%s %02d:%02d:%02d] %s", timeFormat, currTime->tm_hour, currTime->tm_min, currTime->tm_sec, szResult);
		fwrite(cont, 1, strlen(cont), logFile);
		fclose(logFile);
	}
}

void SetLogFileName(const char* filename)
{
	memcpy_s(logFileName, LEN_LOGFILE_NAME, filename, LEN_LOGFILE_NAME);
}
