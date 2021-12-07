#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdbool.h>
#include <windows.h>
#include <time.h>
#include "OraFunctions.h"
#include "StringConvert.h"

bool generateJSON(time_t startCTime, SYSTEMTIME startSystemTime, P_SCHEMA_LIST_STUCTURE ExportList, const char *filename)
{
	char *filename_utf8 = winACPstr2utf8strBufAlloc(filename);
	if (filename_utf8 == NULL)
	{
		return FALSE;
	}
	
	FILE *pJSON_file = utf8_fopen(filename_utf8, "w+");
	free(filename_utf8);
	if (pJSON_file == NULL)
	{
		return FALSE;
	}
	

	bool success = FALSE;
	bool statusIncorrect = FALSE;
	int backupSuccessCount = 0;
	int exportWarnings = 0;
	int exportErrors = 0;
	int receiveErrors = 0;
	int deletedWhileBackup = 0;
	SYSTEMTIME SystemTime;
	time_t CTime;

	long long i;
	for (i = 0; i < ExportList->schemaCount; i++)
	{

		if ((ExportList->pSchemaRows)[i].exportStatus == EXPORT_COMPLETE && (ExportList->pSchemaRows)[i].receiveStatus == RECEIVE_COMPLETE)
		{
			backupSuccessCount++;
		}
		else if ((ExportList->pSchemaRows)[i].exportStatus == EXPORT_WARNING && (ExportList->pSchemaRows)[i].receiveStatus == RECEIVE_COMPLETE)
		{
			exportWarnings++;
		}
		else if ((ExportList->pSchemaRows)[i].exportStatus == EXPORT_COMPLETE && (ExportList->pSchemaRows)[i].receiveStatus == RECEIVE_ERROR)
		{
			receiveErrors++;
		}
		else if ((ExportList->pSchemaRows)[i].exportStatus == EXPORT_WARNING && (ExportList->pSchemaRows)[i].receiveStatus == RECEIVE_ERROR)
		{
			exportWarnings++;
			receiveErrors++;
		}
		else if ((ExportList->pSchemaRows)[i].exportStatus == USER_NOT_EXISTS && (ExportList->pSchemaRows)[i].receiveStatus == RECEIVE_SKIP)
		{
			deletedWhileBackup++;
		}
		else if ((ExportList->pSchemaRows)[i].exportStatus == EXPORT_ERROR)
		{
			exportErrors++;
		}
		else
		{
			statusIncorrect = TRUE;
			GetLocalTime(&SystemTime);
			printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - generateJSON: некорректные статусы у схемы %s - export status = %d, receive status = %d.\n", SystemTime.wDay, SystemTime.wMonth, SystemTime.wYear,
				SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond, (ExportList->pSchemaRows)[i].schema,
				(ExportList->pSchemaRows)[i].exportStatus, (ExportList->pSchemaRows)[i].receiveStatus);
		}
	
	}

	if (statusIncorrect == TRUE || exportErrors != 0 || receiveErrors != 0 || exportWarnings != 0)
	{
		success = FALSE;
	}
	else
	{
		success = TRUE;
	}

	CTime = time(NULL);
	GetLocalTime(&SystemTime);

	fprintf(pJSON_file, "{\n");
	fprintf(pJSON_file, "  \"startTime\":%llu,\n", startCTime);
	fprintf(pJSON_file, "  \"startTimeStr\":\"%04d-%02d-%02d %02d:%02d:%02d\",\n", startSystemTime.wYear, startSystemTime.wMonth, startSystemTime.wDay, startSystemTime.wHour, startSystemTime.wMinute, startSystemTime.wSecond);
	fprintf(pJSON_file, "  \"finishTime\":%llu,\n", CTime);
	fprintf(pJSON_file, "  \"finishTimeStr\":\"%04d-%02d-%02d %02d:%02d:%02d\",\n", SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond);
	fprintf(pJSON_file, "  \"success\":\"%s\",\n", success ? "true" : "false");
	fprintf(pJSON_file, "  \"schemaTotalCount\":%d,\n", ExportList->schemaCount);
	fprintf(pJSON_file, "  \"backupSuccessCount\":%d,\n", backupSuccessCount);
	fprintf(pJSON_file, "  \"exportWarnings\":%d,\n", exportWarnings);
	fprintf(pJSON_file, "  \"exportErrors\":%d,\n", exportErrors);
	fprintf(pJSON_file, "  \"receiveErrors\":%d,\n", receiveErrors);
	fprintf(pJSON_file, "  \"deletedWhileBackup\":%d\n", deletedWhileBackup);
	fprintf(pJSON_file, "}\n");

	fclose(pJSON_file);
	return TRUE;
}