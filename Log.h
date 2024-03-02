#pragma once

#include "Base.h"

#define LEN_LOG 2048
#define LEN_LOGFILE_NAME 32
#define LEN_FILE_PATH 128
#define LEN_FILE 128

//每个服务要设置日志文件路径
extern char logFileName[LEN_LOGFILE_NAME];

void SetLogFileName(const char* filename);
void WriteLog(const char* szContent,...);