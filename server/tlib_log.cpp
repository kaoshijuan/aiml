#ifndef _TLIB_LOG_C_
#define _TLIB_LOG_C_

#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#include "tlib_log.h"

static int ShiftFiles(char *sLogBaseName, long lMaxLogSize, int iMaxLogNum, char *sErrMsg)
{
   struct stat stStat;
	char sLogFileName[300];
	char sNewLogFileName[300];
	int i;

   sprintf(sLogFileName,"%s.log", sLogBaseName);

	if(stat(sLogFileName, &stStat) < 0)
	{
		if (sErrMsg != NULL)
		{
			strcpy(sErrMsg, "Fail to get file status");
		}
		return -1;
	}

	if (stStat.st_size < lMaxLogSize)
	{
		return 0;
	}

	sprintf(sLogFileName,"%s%d.log", sLogBaseName, iMaxLogNum-1);
	if (access(sLogFileName, F_OK) == 0)
	{
		if (remove(sLogFileName) < 0 )
		{
			if (sErrMsg != NULL)
			{
				strcpy(sErrMsg, "Fail to remove oldest log file");
			}
			return -1;
		}
	}

	for(i = iMaxLogNum-2; i >= 0; i--)
	{
		if (i == 0)
			sprintf(sLogFileName,"%s.log", sLogBaseName);
		else
			sprintf(sLogFileName,"%s%d.log", sLogBaseName, i);
			
		if (access(sLogFileName, F_OK) == 0)
		{
			sprintf(sNewLogFileName,"%s%d.log", sLogBaseName, i+1);
			if (rename(sLogFileName,sNewLogFileName) < 0 )
			{
				if (sErrMsg != NULL)
				{
					strcpy(sErrMsg, "Fail to remove oldest log file");
				}
				return -1;
			}
		}
	}
	return 0;
}

char *TLib_Tools_GetDateTimeStr(time_t *mytime)
{
	static char s[50];
	struct tm curr;

	curr = *localtime(mytime);

    if (curr.tm_year > 50)
	{
		sprintf(s, "%04d-%02d-%02d %02d:%02d:%02d", 
					curr.tm_year+1900, curr.tm_mon+1, curr.tm_mday,
					curr.tm_hour, curr.tm_min, curr.tm_sec);
    }
    else
    {
		sprintf(s, "%04d-%02d-%02d %02d:%02d:%02d",
                    curr.tm_year+2000, curr.tm_mon+1, curr.tm_mday,
                    curr.tm_hour, curr.tm_min, curr.tm_sec);
    }

	return s;
}

char *TLib_Tools_GetCurDateTimeStr(void)
{
	time_t  iCurTime;

    time(&iCurTime);
    return TLib_Tools_GetDateTimeStr(&iCurTime);
}

int TLib_Log_VWriteLog(char *sLogBaseName, long lMaxLogSize, int iMaxLogNum, char *sErrMsg, const char *sFormat, va_list ap)
{
	FILE  *pstFile;
	char sLogFileName[300];

   	sprintf(sLogFileName,"%s.log", sLogBaseName);
	if ((pstFile = fopen(sLogFileName, "a+")) == NULL)
	{
		if (sErrMsg != NULL)
		{
			strcpy(sErrMsg, "Fail to open log file");
		}
		return -1;
	}

	fprintf(pstFile, "[%s] ", TLib_Tools_GetCurDateTimeStr());
	
	vfprintf(pstFile, sFormat, ap);
	
	//fprintf(pstFile, "\n");

	fclose(pstFile);

	return ShiftFiles(sLogBaseName, lMaxLogSize, iMaxLogNum, sErrMsg);
}

int TLib_Log_WriteLog(char *sLogBaseName, long lMaxLogSize, int iMaxLogNum, char *sErrMsg, const char *sFormat, ...)
{
	int iRetCode;
	va_list ap;
	
	va_start(ap, sFormat);
	iRetCode = TLib_Log_VWriteLog(sLogBaseName, lMaxLogSize, iMaxLogNum, sErrMsg, sFormat, ap);
	va_end(ap);

	return iRetCode;
}

static char sLogBaseName[200];
static long lMaxLogSize;
static int iMaxLogNum;
static int iLogInitialized = 0;
static int iIsShow;

void TLib_Log_LogInit(char *sPLogBaseName, long lPMaxLogSize, int iPMaxLogNum, int iShow)
{
	memset(sLogBaseName, 0, sizeof(sLogBaseName));
	strncpy(sLogBaseName, sPLogBaseName, sizeof(sLogBaseName)-1);
	lMaxLogSize = lPMaxLogSize;
	iMaxLogNum = iPMaxLogNum;
	iIsShow = iShow;
	iLogInitialized = 1;
}

void TLib_Log_LogMsgDef(int iShow, const char *sFormat, ...)
{
	va_list ap;
	
	if (iShow != 0)
	{
		printf("[%s] ", TLib_Tools_GetCurDateTimeStr());
	
		va_start(ap, sFormat);
		vprintf(sFormat, ap);
		va_end(ap);

		printf("\n");
	}

	if (iLogInitialized != 0)
	{
		va_start(ap, sFormat);
		TLib_Log_VWriteLog(sLogBaseName, lMaxLogSize, iMaxLogNum, NULL, sFormat, ap);
		va_end(ap);
	}
}

void TLib_Log_LogMsg(const char *sFormat, ...)
{
	va_list ap;
	
	if (iIsShow != 0)
	{
		printf("[%s] ", TLib_Tools_GetCurDateTimeStr());
	
		va_start(ap, sFormat);
		vprintf(sFormat, ap);
		va_end(ap);

		printf("\n");
	}

	if (iLogInitialized != 0)
	{
		va_start(ap, sFormat);
		TLib_Log_VWriteLog(sLogBaseName, lMaxLogSize, iMaxLogNum, NULL, sFormat, ap);
		va_end(ap);
	}
}

#endif
