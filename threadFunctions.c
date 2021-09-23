#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <windows.h>
#include "OraFunctions.h"
#include "threadFunctions.h"
#include <process.h>
#include <time.h>
#include "StringConvert.h"

void RemoveDoubleBackslashes(char * winPathString)
{
	int i;

	for (i = 0; *(winPathString + i) != 0; i++)
	{
		if (*(winPathString + i) == '\\' && *(winPathString + i + 1) == '\\')
		{
			*(winPathString + i + 1) = 0;
			strcat(winPathString, winPathString + i + 2);
		}
	}
}

unsigned WINAPI ExportThread(void * ppArgs)
{
	//структура времени для потока экспорта.
	SYSTEMTIME exportThreadTimeStruct;
	GetLocalTime(&exportThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [exportThread] - Поток экпорта начинает работу\n", exportThreadTimeStruct.wDay, exportThreadTimeStruct.wMonth, exportThreadTimeStruct.wYear,
		exportThreadTimeStruct.wHour, exportThreadTimeStruct.wMinute, exportThreadTimeStruct.wSecond);

	time_t startTime, endTime;
	startTime = time(NULL);
	char dumpname[256];
	char logname[256];
	
	

	P_EXPORT_STUCTURE ExpStr = (P_EXPORT_STUCTURE)ppArgs;

	
	long long i;
	for (i = 0; i < ExpStr->ExportList->schemaCount; i++)
	{
		//printf("%s, status - %d\n", (ExpStr->ExportList->pSchemaRows)[i].schema, (ExpStr->ExportList->pSchemaRows)[i].status);
		
		if (IsUserExists(ExpStr->hOraSvcCtx, ExpStr->hOraEnv, ExpStr->hOraErr, (ExpStr->ExportList->pSchemaRows)[i].schema) == USER_NOT_EXISTS)
		{
			GetLocalTime(&exportThreadTimeStruct);
			printf("%02d.%02d.%04d %02d:%02d:%02d [exportThread] - Пользователь %s не найден в базе, пропускаем...\n", exportThreadTimeStruct.wDay, exportThreadTimeStruct.wMonth, exportThreadTimeStruct.wYear,
				exportThreadTimeStruct.wHour, exportThreadTimeStruct.wMinute, exportThreadTimeStruct.wSecond, (ExpStr->ExportList->pSchemaRows)[i].schema);
			(ExpStr->ExportList->pSchemaRows)[i].status = USER_NOT_EXISTS;
			continue;
		}

		GetLocalTime(&exportThreadTimeStruct);
		sprintf((ExpStr->ExportList->pSchemaRows)[i].filename, "%s_%02d_%02d_%04d_%02d_%02d_%02d", (ExpStr->ExportList->pSchemaRows)[i].schema,
			exportThreadTimeStruct.wDay, exportThreadTimeStruct.wMonth, exportThreadTimeStruct.wYear, 
			exportThreadTimeStruct.wHour, exportThreadTimeStruct.wMinute, exportThreadTimeStruct.wSecond);
		
		sprintf(dumpname, "%s.dmp", (ExpStr->ExportList->pSchemaRows)[i].filename);
		sprintf(logname, "%s.log", (ExpStr->ExportList->pSchemaRows)[i].filename);

		
		GetLocalTime(&exportThreadTimeStruct);
		printf("%02d.%02d.%04d %02d:%02d:%02d [exportThread] - Запускаем экспорт %s\n", exportThreadTimeStruct.wDay, exportThreadTimeStruct.wMonth, exportThreadTimeStruct.wYear,
			exportThreadTimeStruct.wHour, exportThreadTimeStruct.wMinute, exportThreadTimeStruct.wSecond, (ExpStr->ExportList->pSchemaRows)[i].schema);
		if (!ExecuteExportJob(ExpStr->hOraSvcCtx, ExpStr->hOraEnv, ExpStr->hOraErr, 
			(ExpStr->ExportList->pSchemaRows)[i].schema, "DATA_PUMP_DIR", dumpname, logname))
		{
			GetLocalTime(&exportThreadTimeStruct);
			printf("%02d.%02d.%04d %02d:%02d:%02d [exportThread] - Ошибка экспорта схемы %s, завершаем работу потока экспорта\n", exportThreadTimeStruct.wDay, exportThreadTimeStruct.wMonth, exportThreadTimeStruct.wYear,
				exportThreadTimeStruct.wHour, exportThreadTimeStruct.wMinute, exportThreadTimeStruct.wSecond, (ExpStr->ExportList->pSchemaRows)[i].schema);
			(ExpStr->ExportList->pSchemaRows)[i].status = EXPORT_ERROR;
			break;
			
		}

		(ExpStr->ExportList->pSchemaRows)[i].status = EXPORT_COMPLETE;
	}

	endTime = time(NULL);
	GetLocalTime(&exportThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [exportThread] - Общее время экспорта - %lld:%02lld:%02lld\n", exportThreadTimeStruct.wDay, exportThreadTimeStruct.wMonth, exportThreadTimeStruct.wYear,
		exportThreadTimeStruct.wHour, exportThreadTimeStruct.wMinute, exportThreadTimeStruct.wSecond, (endTime - startTime)/3600, ((endTime - startTime) % 3600) / 60, ((endTime - startTime) % 3600) % 60);
	//printf("Время выполнения потока - %lld секунд\n");


	GetLocalTime(&exportThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [exportThread] - Поток экпорта звершает работу\n", exportThreadTimeStruct.wDay, exportThreadTimeStruct.wMonth, exportThreadTimeStruct.wYear,
		exportThreadTimeStruct.wHour, exportThreadTimeStruct.wMinute, exportThreadTimeStruct.wSecond);
	_endthreadex(EXIT_SUCCESS);
	return EXIT_SUCCESS;
}


unsigned WINAPI RecieveThread(void * ppArgs)
{
	//структура времени для потока получения.
	SYSTEMTIME recieveThreadTimeStruct;
	GetLocalTime(&recieveThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [receiveThread] - Поток передачи файлов начинает работу\n", recieveThreadTimeStruct.wDay, recieveThreadTimeStruct.wMonth, recieveThreadTimeStruct.wYear,
		recieveThreadTimeStruct.wHour, recieveThreadTimeStruct.wMinute, recieveThreadTimeStruct.wSecond);

	time_t startTime, endTime, workTime = 0, idleTime = 0;
	
	char dumpname[256];
	char logname[256];
	char zipname[256];

	P_RECIEVE_STUCTURE RecieveStr = (P_RECIEVE_STUCTURE)ppArgs;

	long long i;
	for (i = 0; i < RecieveStr->ExportList->schemaCount; i++)
	{
		startTime = time(NULL);
		
		if ((RecieveStr->ExportList->pSchemaRows)[i].status == NOT_PROCESSED)
		{
			GetLocalTime(&recieveThreadTimeStruct);
			printf("%02d.%02d.%04d %02d:%02d:%02d [receiveThread] - Ждем, когда закончится экспорт %s\n", recieveThreadTimeStruct.wDay, recieveThreadTimeStruct.wMonth, recieveThreadTimeStruct.wYear,
				recieveThreadTimeStruct.wHour, recieveThreadTimeStruct.wMinute, recieveThreadTimeStruct.wSecond, (RecieveStr->ExportList->pSchemaRows)[i].schema);
		}
		while ((RecieveStr->ExportList->pSchemaRows)[i].status == NOT_PROCESSED)
		{
			Sleep(3000);
		}
		
		endTime = time(NULL);
		idleTime = idleTime + (endTime - startTime);

		startTime = time(NULL);
		if ((RecieveStr->ExportList->pSchemaRows)[i].status == USER_NOT_EXISTS)
		{
			GetLocalTime(&recieveThreadTimeStruct);
			printf("%02d.%02d.%04d %02d:%02d:%02d [receiveThread] - Пользователя %s нет в базе, пропускаем\n", recieveThreadTimeStruct.wDay, recieveThreadTimeStruct.wMonth, recieveThreadTimeStruct.wYear,
				recieveThreadTimeStruct.wHour, recieveThreadTimeStruct.wMinute, recieveThreadTimeStruct.wSecond, (RecieveStr->ExportList->pSchemaRows)[i].schema);
			continue;
		}
		else if ((RecieveStr->ExportList->pSchemaRows)[i].status == EXPORT_ERROR)
		{
			GetLocalTime(&recieveThreadTimeStruct);
			printf("%02d.%02d.%04d %02d:%02d:%02d [receiveThread] - Экпорт %s завершился с ошибкой, завершем работу потока\n", recieveThreadTimeStruct.wDay, recieveThreadTimeStruct.wMonth, recieveThreadTimeStruct.wYear,
				recieveThreadTimeStruct.wHour, recieveThreadTimeStruct.wMinute, recieveThreadTimeStruct.wSecond, (RecieveStr->ExportList->pSchemaRows)[i].schema);
			break;
		}
		else if ((RecieveStr->ExportList->pSchemaRows)[i].status == EXPORT_COMPLETE)
		{
			sprintf(dumpname, "%s.dmp", (RecieveStr->ExportList->pSchemaRows)[i].filename);
			sprintf(logname, "%s.log", (RecieveStr->ExportList->pSchemaRows)[i].filename);
			sprintf(zipname, "%s\\%s.zip", RecieveStr->dumpDir, (RecieveStr->ExportList->pSchemaRows)[i].filename);
			RemoveDoubleBackslashes(zipname);
			//printf("zipname - %s\n", zipname);
			

			GetLocalTime(&recieveThreadTimeStruct);
			printf("%02d.%02d.%04d %02d:%02d:%02d [receiveThread] - Получаем дамп схемы %s \n", recieveThreadTimeStruct.wDay, recieveThreadTimeStruct.wMonth, recieveThreadTimeStruct.wYear,
				recieveThreadTimeStruct.wHour, recieveThreadTimeStruct.wMinute, recieveThreadTimeStruct.wSecond, (RecieveStr->ExportList->pSchemaRows)[i].schema);
			if (IsDBLocal(RecieveStr->hOraSvcCtx, RecieveStr->hOraEnv, RecieveStr->hOraErr) == LOCALDB)
			{
				if (!LocalGetFileFromDatabaseToZip(RecieveStr->hOraSvcCtx, RecieveStr->hOraEnv, RecieveStr->hOraErr, 
					zipname, "DATA_PUMP_DIR", dumpname))
				{
					utf8_fdelete(zipname);
					if (!GetFileFromDatabaseToZip(RecieveStr->hOraSvcCtx, RecieveStr->hOraEnv, RecieveStr->hOraErr,
						zipname, "DATA_PUMP_DIR", dumpname))
					{
						GetLocalTime(&recieveThreadTimeStruct);
						printf("%02d.%02d.%04d %02d:%02d:%02d [receiveThread] - Дамп схемы %s получить не удалось\n", recieveThreadTimeStruct.wDay, recieveThreadTimeStruct.wMonth, recieveThreadTimeStruct.wYear,
							recieveThreadTimeStruct.wHour, recieveThreadTimeStruct.wMinute, recieveThreadTimeStruct.wSecond, (RecieveStr->ExportList->pSchemaRows)[i].schema);
					}
				}
			}
			else
			{
				if (!GetFileFromDatabaseToZip(RecieveStr->hOraSvcCtx, RecieveStr->hOraEnv, RecieveStr->hOraErr,
					zipname, "DATA_PUMP_DIR", dumpname))
				{
					GetLocalTime(&recieveThreadTimeStruct);
					printf("%02d.%02d.%04d %02d:%02d:%02d [receiveThread] - Дамп схемы %s получить не удалось\n", recieveThreadTimeStruct.wDay, recieveThreadTimeStruct.wMonth, recieveThreadTimeStruct.wYear,
						recieveThreadTimeStruct.wHour, recieveThreadTimeStruct.wMinute, recieveThreadTimeStruct.wSecond, (RecieveStr->ExportList->pSchemaRows)[i].schema);
				}
			}

			//GetLocalTime(&recieveThreadTimeStruct);
			//printf("%02d.%02d.%04d %02d:%02d:%02d [recieveThread] - Получаем лог схемы %s \n", recieveThreadTimeStruct.wDay, recieveThreadTimeStruct.wMonth, recieveThreadTimeStruct.wYear,
			//	recieveThreadTimeStruct.wHour, recieveThreadTimeStruct.wMinute, recieveThreadTimeStruct.wSecond, (RecieveStr->ExportList->pSchemaRows)[i].schema);
			if (!GetFileFromDatabaseToZip(RecieveStr->hOraSvcCtx, RecieveStr->hOraEnv, RecieveStr->hOraErr,
				zipname, "DATA_PUMP_DIR", logname))
			{
				GetLocalTime(&recieveThreadTimeStruct);
				printf("%02d.%02d.%04d %02d:%02d:%02d [receiveThread] - Лог экспорта схемы %s получить не удалось\n", recieveThreadTimeStruct.wDay, recieveThreadTimeStruct.wMonth, recieveThreadTimeStruct.wYear,
					recieveThreadTimeStruct.wHour, recieveThreadTimeStruct.wMinute, recieveThreadTimeStruct.wSecond, (RecieveStr->ExportList->pSchemaRows)[i].schema);
			}

			
			//GetLocalTime(&recieveThreadTimeStruct);
			//printf("%02d.%02d.%04d %02d:%02d:%02d [recieveThread] - Чистим катлог БД после экспорта %s\n", recieveThreadTimeStruct.wDay, recieveThreadTimeStruct.wMonth, recieveThreadTimeStruct.wYear,
			//	recieveThreadTimeStruct.wHour, recieveThreadTimeStruct.wMinute, recieveThreadTimeStruct.wSecond, (RecieveStr->ExportList->pSchemaRows)[i].schema);
			DeleteFileFromOradir(RecieveStr->hOraSvcCtx, RecieveStr->hOraEnv, RecieveStr->hOraErr, "DATA_PUMP_DIR", dumpname);
			DeleteFileFromOradir(RecieveStr->hOraSvcCtx, RecieveStr->hOraEnv, RecieveStr->hOraErr, "DATA_PUMP_DIR", logname);
		}

		
		endTime = time(NULL);
		workTime = workTime + (endTime - startTime);

	}

	GetLocalTime(&recieveThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [receiveThread] - Поток получения данных завершает работу\n", recieveThreadTimeStruct.wDay, recieveThreadTimeStruct.wMonth, recieveThreadTimeStruct.wYear,
		recieveThreadTimeStruct.wHour, recieveThreadTimeStruct.wMinute, recieveThreadTimeStruct.wSecond);
	
	GetLocalTime(&recieveThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [receiveThread] - Время ожидания - %lld:%02lld:%02lld, передачи данных - %lld:%02lld:%02lld\n", recieveThreadTimeStruct.wDay, recieveThreadTimeStruct.wMonth, recieveThreadTimeStruct.wYear,
		recieveThreadTimeStruct.wHour, recieveThreadTimeStruct.wMinute, recieveThreadTimeStruct.wSecond, 
		idleTime/3600, (idleTime % 3600) / 60 , (idleTime % 3600) % 60, workTime/3600, (workTime % 3600) / 60, (workTime % 3600) % 60);

	_endthreadex(EXIT_SUCCESS);
	return EXIT_SUCCESS;
}
