#pragma once

typedef struct export_struct
{
	P_SCHEMA_LIST_STUCTURE ExportList;
	OCISvcCtx * hOraSvcCtx;
	OCIEnv * hOraEnv;
	OCIError *hOraErr;
} EXPORT_STUCTURE, *P_EXPORT_STUCTURE;

typedef struct recieve_struct
{
	P_SCHEMA_LIST_STUCTURE ExportList;
	OCISvcCtx * hOraSvcCtx;
	OCIEnv * hOraEnv;
	OCIError *hOraErr;
	char * dumpDir;
} RECIEVE_STUCTURE, *P_RECIEVE_STUCTURE;

//Функция обратного вызова для запуска в отдельном потоке, идет по списку схем и запускает по очереди задачи экпорта
unsigned WINAPI ExportThread(void * ppArgs);

//Функция обратного вызова для запуска в отдельном потоке, идет по списку схем, забирает дампы с сервера и удаляет их с сервера
unsigned WINAPI RecieveThread(void * ppArgs);