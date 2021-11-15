#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdbool.h>
#include <windows.h>
#include "OraFunctions.h"
#include "threadFunctions.h"
#include <process.h>
#include <time.h>
#include "StringConvert.h"

//#define DEBUG_LOGGING

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
	P_EXPORT_STUCTURE ExpStr = (P_EXPORT_STUCTURE)ppArgs;
	
	//Задержка по времени для каждого потока, так как одновременный запуск нескольких задач экспорта вызывает ошибку Оракла
	//Задержка вводилась до того, как было исправлено получение окружения, но это неплохая идея, чтобы не стартовать в один момент экспорт
	Sleep((ExpStr->threadNumber - 1) * 1000);
	
	//структура времени для потока экспорта.
	SYSTEMTIME exportThreadTimeStruct;
	GetLocalTime(&exportThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [exportThread_%d] - Поток экспорта начинает работу\n", exportThreadTimeStruct.wDay, exportThreadTimeStruct.wMonth, exportThreadTimeStruct.wYear,
		exportThreadTimeStruct.wHour, exportThreadTimeStruct.wMinute, exportThreadTimeStruct.wSecond, ExpStr->threadNumber);

	time_t startTime, endTime;
	startTime = time(NULL);
	char dumpname[256];
	char logname[256];
	
	
	long long i;
	for (i = 0; i < ExpStr->ExportList->schemaCount; i++)
	{

		EnterCriticalSection(ExpStr->ExportCriticalSection);
		
		if ((ExpStr->ExportList->pSchemaRows)[i].exportStatus == NOT_PROCESSED)
		{
			(ExpStr->ExportList->pSchemaRows)[i].exportStatus = EXPORT_RUNNING;
			LeaveCriticalSection(ExpStr->ExportCriticalSection);
		}
		else
		{
			LeaveCriticalSection(ExpStr->ExportCriticalSection);
			continue;
		}
		
		
		
		if (IsUserExists(ExpStr->hOraSvcCtx, ExpStr->hOraEnv, ExpStr->hOraErr, (ExpStr->ExportList->pSchemaRows)[i].schema) == USER_NOT_EXISTS)
		{
			GetLocalTime(&exportThreadTimeStruct);
			printf("[exportThread_%d] - i = %lld\n", ExpStr->threadNumber, i);
			printf("%02d.%02d.%04d %02d:%02d:%02d [exportThread_%d] - Пользователь %s не найден в базе, пропускаем...\n", exportThreadTimeStruct.wDay, exportThreadTimeStruct.wMonth, exportThreadTimeStruct.wYear,
				exportThreadTimeStruct.wHour, exportThreadTimeStruct.wMinute, exportThreadTimeStruct.wSecond, ExpStr->threadNumber, (ExpStr->ExportList->pSchemaRows)[i].schema);
			(ExpStr->ExportList->pSchemaRows)[i].exportStatus = USER_NOT_EXISTS;
			continue;
		}

		GetLocalTime(&exportThreadTimeStruct);
		sprintf((ExpStr->ExportList->pSchemaRows)[i].filename, "%s_%02d_%02d_%04d_%02d_%02d_%02d", (ExpStr->ExportList->pSchemaRows)[i].schema,
			exportThreadTimeStruct.wDay, exportThreadTimeStruct.wMonth, exportThreadTimeStruct.wYear, 
			exportThreadTimeStruct.wHour, exportThreadTimeStruct.wMinute, exportThreadTimeStruct.wSecond);

		//Убираем из имени файла символы '\', '/' и '%'
		int tempcounter;
		for (tempcounter = 0; (ExpStr->ExportList->pSchemaRows)[i].filename[tempcounter] != 0; tempcounter++)
		{
			if ((ExpStr->ExportList->pSchemaRows)[i].filename[tempcounter] == '\\' || (ExpStr->ExportList->pSchemaRows)[i].filename[tempcounter] == '/' || (ExpStr->ExportList->pSchemaRows)[i].filename[tempcounter] == '%')
			{
				(ExpStr->ExportList->pSchemaRows)[i].filename[tempcounter] = '_';
			}
		}
		
		sprintf(dumpname, "%s.dmp", (ExpStr->ExportList->pSchemaRows)[i].filename);
		sprintf(logname, "%s.log", (ExpStr->ExportList->pSchemaRows)[i].filename);

		strcpy((ExpStr->ExportList->pSchemaRows)[i].datapumpDirName, ExpStr->datapumpDir);
		
		GetLocalTime(&exportThreadTimeStruct);
		printf("%02d.%02d.%04d %02d:%02d:%02d [exportThread_%d] - Запускаем экспорт %s в каталог %s\n", exportThreadTimeStruct.wDay, exportThreadTimeStruct.wMonth, exportThreadTimeStruct.wYear,
			exportThreadTimeStruct.wHour, exportThreadTimeStruct.wMinute, exportThreadTimeStruct.wSecond, ExpStr->threadNumber, (ExpStr->ExportList->pSchemaRows)[i].schema, (ExpStr->ExportList->pSchemaRows)[i].datapumpDirName);
		if (!ExecuteExportJob(ExpStr->hOraSvcCtx, ExpStr->hOraEnv, ExpStr->hOraErr, 
			(ExpStr->ExportList->pSchemaRows)[i].schema, (ExpStr->ExportList->pSchemaRows)[i].datapumpDirName, dumpname, logname))
		{
			GetLocalTime(&exportThreadTimeStruct);
			printf("%02d.%02d.%04d %02d:%02d:%02d [exportThread_%d] - Внимание! Ошибка экспорта схемы %s\n", exportThreadTimeStruct.wDay, exportThreadTimeStruct.wMonth, exportThreadTimeStruct.wYear,
				exportThreadTimeStruct.wHour, exportThreadTimeStruct.wMinute, exportThreadTimeStruct.wSecond, ExpStr->threadNumber, (ExpStr->ExportList->pSchemaRows)[i].schema);
			(ExpStr->ExportList->pSchemaRows)[i].exportStatus = EXPORT_ERROR;

			
			break;
			
		}

		(ExpStr->ExportList->pSchemaRows)[i].exportStatus = EXPORT_COMPLETE;
	}

	endTime = time(NULL);
	GetLocalTime(&exportThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [exportThread_%d] - Общее время экспорта - %lld:%02lld:%02lld\n", exportThreadTimeStruct.wDay, exportThreadTimeStruct.wMonth, exportThreadTimeStruct.wYear,
		exportThreadTimeStruct.wHour, exportThreadTimeStruct.wMinute, exportThreadTimeStruct.wSecond, ExpStr->threadNumber, (endTime - startTime)/3600, ((endTime - startTime) % 3600) / 60, ((endTime - startTime) % 3600) % 60);


	GetLocalTime(&exportThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [exportThread_%d] - Поток экспорта завершает работу\n", exportThreadTimeStruct.wDay, exportThreadTimeStruct.wMonth, exportThreadTimeStruct.wYear,
		exportThreadTimeStruct.wHour, exportThreadTimeStruct.wMinute, exportThreadTimeStruct.wSecond, ExpStr->threadNumber);
	_endthreadex(EXIT_SUCCESS);
	return EXIT_SUCCESS;
}


unsigned WINAPI RecieveThread(void * ppArgs)
{
	P_RECIEVE_STUCTURE RecieveStr = (P_RECIEVE_STUCTURE)ppArgs;
	
	//структура времени для потока получения.
	SYSTEMTIME recieveThreadTimeStruct;
	GetLocalTime(&recieveThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [receiveThread_%d] - Поток передачи файлов начинает работу\n", recieveThreadTimeStruct.wDay, recieveThreadTimeStruct.wMonth, recieveThreadTimeStruct.wYear,
		recieveThreadTimeStruct.wHour, recieveThreadTimeStruct.wMinute, recieveThreadTimeStruct.wSecond, RecieveStr->threadNumber);

	time_t startTime, endTime, workTime = 0, idleTime = 0;
	
	char dumpname[256];
	char logname[256];
	char zipname[256];
	char dbdirname[32];
	bool recieve_error = FALSE;

	
	long long i;
	for (i = 0; i < RecieveStr->ExportList->schemaCount; i++)
	{
		startTime = time(NULL);

		EnterCriticalSection(RecieveStr->ReceiveCriticalSection);
		
		if  ((RecieveStr->ExportList->pSchemaRows)[i].receiveStatus != NOT_PROCESSED)
		{
			//Если статус приема не NOT_PROCESSED, значит какой-то поток уже взял в работу, пропускаем
			LeaveCriticalSection(RecieveStr->ReceiveCriticalSection);
			continue;
		}
		else if ((RecieveStr->ExportList->pSchemaRows)[i].exportStatus == EXPORT_ERROR || (RecieveStr->ExportList->pSchemaRows)[i].exportStatus == USER_NOT_EXISTS)
		{
			GetLocalTime(&recieveThreadTimeStruct);
			printf("%02d.%02d.%04d %02d:%02d:%02d [receiveThread_%d] - Ошибка экспорта схемы %s, либо пользователь уже удален\n", recieveThreadTimeStruct.wDay, recieveThreadTimeStruct.wMonth, recieveThreadTimeStruct.wYear,
				recieveThreadTimeStruct.wHour, recieveThreadTimeStruct.wMinute, recieveThreadTimeStruct.wSecond, RecieveStr->threadNumber, (RecieveStr->ExportList->pSchemaRows)[i].schema);
			//Если экспорт завершен и статус приема EXPORT_ERROR или USER_NOT_EXISTS, пропускаем
			LeaveCriticalSection(RecieveStr->ReceiveCriticalSection);
			continue;
		}
		else if ((RecieveStr->ExportList->pSchemaRows)[i].exportStatus == EXPORT_COMPLETE && (RecieveStr->ExportList->pSchemaRows)[i].receiveStatus == NOT_PROCESSED)
		{
			(RecieveStr->ExportList->pSchemaRows)[i].receiveStatus = RECEIVE_RUNNING;
			GetLocalTime(&recieveThreadTimeStruct);
			printf("%02d.%02d.%04d %02d:%02d:%02d [receiveThread_%d] - Экспорт схемы %s завершен, начинаем выгрузку\n", recieveThreadTimeStruct.wDay, recieveThreadTimeStruct.wMonth, recieveThreadTimeStruct.wYear,
				recieveThreadTimeStruct.wHour, recieveThreadTimeStruct.wMinute, recieveThreadTimeStruct.wSecond, RecieveStr->threadNumber, (RecieveStr->ExportList->pSchemaRows)[i].schema);
			
			LeaveCriticalSection(RecieveStr->ReceiveCriticalSection);
		}
		else if (((RecieveStr->ExportList->pSchemaRows)[i].exportStatus == EXPORT_RUNNING || (RecieveStr->ExportList->pSchemaRows)[i].exportStatus == NOT_PROCESSED) && (RecieveStr->ExportList->pSchemaRows)[i].receiveStatus == NOT_PROCESSED)
		{
			(RecieveStr->ExportList->pSchemaRows)[i].receiveStatus = RECEIVE_RUNNING;
			GetLocalTime(&recieveThreadTimeStruct);
			printf("%02d.%02d.%04d %02d:%02d:%02d [receiveThread_%d] - Схема %s еще не выгружена, ждем завершения\n", recieveThreadTimeStruct.wDay, recieveThreadTimeStruct.wMonth, recieveThreadTimeStruct.wYear,
				recieveThreadTimeStruct.wHour, recieveThreadTimeStruct.wMinute, recieveThreadTimeStruct.wSecond, RecieveStr->threadNumber, (RecieveStr->ExportList->pSchemaRows)[i].schema);

			LeaveCriticalSection(RecieveStr->ReceiveCriticalSection);

			while ((RecieveStr->ExportList->pSchemaRows)[i].exportStatus == EXPORT_RUNNING || (RecieveStr->ExportList->pSchemaRows)[i].exportStatus == NOT_PROCESSED)
			{
				Sleep(3000);
			}

		}
		else
		{
			//Этот пункт был добавлен для дебага. При анализе всех вариантов было просчитано, что все возможные ситуации обрабатываются предыдущими условаиями, но оставляем его на всякий случай
			GetLocalTime(&recieveThreadTimeStruct);
			printf("%02d.%02d.%04d %02d:%02d:%02d [receiveThread_%d] - Необрабатываемая ситуация, у схемы %s export status = %d, receive status = %d. Снимаем блокировку и пропускаем\n", recieveThreadTimeStruct.wDay, recieveThreadTimeStruct.wMonth, recieveThreadTimeStruct.wYear,
				recieveThreadTimeStruct.wHour, recieveThreadTimeStruct.wMinute, recieveThreadTimeStruct.wSecond, RecieveStr->threadNumber, (RecieveStr->ExportList->pSchemaRows)[i].schema,
				(RecieveStr->ExportList->pSchemaRows)[i].exportStatus, (RecieveStr->ExportList->pSchemaRows)[i].receiveStatus);

			LeaveCriticalSection(RecieveStr->ReceiveCriticalSection);
			continue;
		}


		endTime = time(NULL);
		idleTime = idleTime + (endTime - startTime);

		startTime = time(NULL);
		if ((RecieveStr->ExportList->pSchemaRows)[i].exportStatus == USER_NOT_EXISTS)
		{
			(RecieveStr->ExportList->pSchemaRows)[i].receiveStatus = RECEIVE_SKIP;
			GetLocalTime(&recieveThreadTimeStruct);
			printf("%02d.%02d.%04d %02d:%02d:%02d [receiveThread_%d] - Пользователя %s нет в базе, пропускаем\n", recieveThreadTimeStruct.wDay, recieveThreadTimeStruct.wMonth, recieveThreadTimeStruct.wYear,
				recieveThreadTimeStruct.wHour, recieveThreadTimeStruct.wMinute, recieveThreadTimeStruct.wSecond, RecieveStr->threadNumber, (RecieveStr->ExportList->pSchemaRows)[i].schema);
			continue;
		}
		else if ((RecieveStr->ExportList->pSchemaRows)[i].exportStatus == EXPORT_ERROR)
		{
			(RecieveStr->ExportList->pSchemaRows)[i].receiveStatus = RECEIVE_SKIP;
			GetLocalTime(&recieveThreadTimeStruct);
			printf("%02d.%02d.%04d %02d:%02d:%02d [receiveThread_%d] - Экспорт %s завершился с ошибкой, пропускаем\n", recieveThreadTimeStruct.wDay, recieveThreadTimeStruct.wMonth, recieveThreadTimeStruct.wYear,
				recieveThreadTimeStruct.wHour, recieveThreadTimeStruct.wMinute, recieveThreadTimeStruct.wSecond, RecieveStr->threadNumber, (RecieveStr->ExportList->pSchemaRows)[i].schema);
			continue;
		}
		else if ((RecieveStr->ExportList->pSchemaRows)[i].exportStatus == EXPORT_COMPLETE)
		{
			sprintf(dumpname, "%s.dmp", (RecieveStr->ExportList->pSchemaRows)[i].filename);
			sprintf(logname, "%s.log", (RecieveStr->ExportList->pSchemaRows)[i].filename);
			sprintf(zipname, "%s\\%s.zip", RecieveStr->dumpDir, (RecieveStr->ExportList->pSchemaRows)[i].filename);
			RemoveDoubleBackslashes(zipname);
			strcpy(dbdirname,(RecieveStr->ExportList->pSchemaRows)[i].datapumpDirName);
			

			GetLocalTime(&recieveThreadTimeStruct);
			printf("%02d.%02d.%04d %02d:%02d:%02d [receiveThread_%d] - Получаем дамп схемы %s \n", recieveThreadTimeStruct.wDay, recieveThreadTimeStruct.wMonth, recieveThreadTimeStruct.wYear,
				recieveThreadTimeStruct.wHour, recieveThreadTimeStruct.wMinute, recieveThreadTimeStruct.wSecond, RecieveStr->threadNumber, (RecieveStr->ExportList->pSchemaRows)[i].schema);
			if (IsDBLocal(RecieveStr->hOraSvcCtx, RecieveStr->hOraEnv, RecieveStr->hOraErr) == LOCALDB)
			{
				if (!LocalGetFileFromDatabaseToZip(RecieveStr->hOraSvcCtx, RecieveStr->hOraEnv, RecieveStr->hOraErr, 
					zipname, dbdirname, dumpname))
				{
					utf8_fdelete(zipname);
					if (!GetFileFromDatabaseToZip(RecieveStr->hOraSvcCtx, RecieveStr->hOraEnv, RecieveStr->hOraErr,
						zipname, dbdirname, dumpname))
					{
						recieve_error = TRUE;
						(RecieveStr->ExportList->pSchemaRows)[i].receiveStatus = RECEIVE_ERROR;
						GetLocalTime(&recieveThreadTimeStruct);
						printf("%02d.%02d.%04d %02d:%02d:%02d [receiveThread_%d] - Дамп схемы %s получить не удалось\n", recieveThreadTimeStruct.wDay, recieveThreadTimeStruct.wMonth, recieveThreadTimeStruct.wYear,
							recieveThreadTimeStruct.wHour, recieveThreadTimeStruct.wMinute, recieveThreadTimeStruct.wSecond, RecieveStr->threadNumber, (RecieveStr->ExportList->pSchemaRows)[i].schema);
					}
				}
			}
			else
			{
				if (!GetFileFromDatabaseToZip(RecieveStr->hOraSvcCtx, RecieveStr->hOraEnv, RecieveStr->hOraErr,
					zipname, dbdirname, dumpname))
				{
					recieve_error = TRUE;
					(RecieveStr->ExportList->pSchemaRows)[i].receiveStatus = RECEIVE_ERROR;
					GetLocalTime(&recieveThreadTimeStruct);
					printf("%02d.%02d.%04d %02d:%02d:%02d [receiveThread_%d] - Дамп схемы %s получить не удалось\n", recieveThreadTimeStruct.wDay, recieveThreadTimeStruct.wMonth, recieveThreadTimeStruct.wYear,
						recieveThreadTimeStruct.wHour, recieveThreadTimeStruct.wMinute, recieveThreadTimeStruct.wSecond, RecieveStr->threadNumber, (RecieveStr->ExportList->pSchemaRows)[i].schema);
				}
			}

			if (!GetFileFromDatabaseToZip(RecieveStr->hOraSvcCtx, RecieveStr->hOraEnv, RecieveStr->hOraErr,
				zipname, dbdirname, logname))
			{
				recieve_error = TRUE;
				(RecieveStr->ExportList->pSchemaRows)[i].receiveStatus = RECEIVE_ERROR;
				GetLocalTime(&recieveThreadTimeStruct);
				printf("%02d.%02d.%04d %02d:%02d:%02d [receiveThread_%d] - Лог экспорта схемы %s получить не удалось\n", recieveThreadTimeStruct.wDay, recieveThreadTimeStruct.wMonth, recieveThreadTimeStruct.wYear,
					recieveThreadTimeStruct.wHour, recieveThreadTimeStruct.wMinute, recieveThreadTimeStruct.wSecond, RecieveStr->threadNumber, (RecieveStr->ExportList->pSchemaRows)[i].schema);
			}

			
			if (recieve_error == FALSE)
			{
				(RecieveStr->ExportList->pSchemaRows)[i].receiveStatus = RECEIVE_COMPLETE;
				DeleteFileFromOradir(RecieveStr->hOraSvcCtx, RecieveStr->hOraEnv, RecieveStr->hOraErr, dbdirname, dumpname);
				DeleteFileFromOradir(RecieveStr->hOraSvcCtx, RecieveStr->hOraEnv, RecieveStr->hOraErr, dbdirname, logname);
			}
			recieve_error = FALSE;
		}

		
		endTime = time(NULL);
		workTime = workTime + (endTime - startTime);

	}

	GetLocalTime(&recieveThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [receiveThread_%d] - Поток получения данных завершает работу\n", recieveThreadTimeStruct.wDay, recieveThreadTimeStruct.wMonth, recieveThreadTimeStruct.wYear,
		recieveThreadTimeStruct.wHour, recieveThreadTimeStruct.wMinute, recieveThreadTimeStruct.wSecond, RecieveStr->threadNumber);
	
	GetLocalTime(&recieveThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [receiveThread_%d] - Время ожидания - %lld:%02lld:%02lld, передачи данных - %lld:%02lld:%02lld\n", recieveThreadTimeStruct.wDay, recieveThreadTimeStruct.wMonth, recieveThreadTimeStruct.wYear,
		recieveThreadTimeStruct.wHour, recieveThreadTimeStruct.wMinute, recieveThreadTimeStruct.wSecond, RecieveStr->threadNumber,
		idleTime/3600, (idleTime % 3600) / 60 , (idleTime % 3600) % 60, workTime/3600, (workTime % 3600) / 60, (workTime % 3600) % 60);

	_endthreadex(EXIT_SUCCESS);
	return EXIT_SUCCESS;
}
