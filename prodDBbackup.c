#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <windows.h>
#include <time.h>
#include "OraFunctions.h"
#include "StringConvert.h"
#include "threadFunctions.h"
#include <process.h>

BOOL parseCmdLine(int argc, char* argv[], char *login, char *pass, char *dblink, char **dumpdir, char **select);

int main(int argc, char* argv[])
{
	//Логин для подключения. Получаем из аргументов командной строки. Так как он должен быть на английском, считаем, что он в кодировке UTF8, без преобразований.
	char login[32] = { 0 };
	//Пароль для подключения. Получаем из аргументов командной строки. Так как он должен быть на английском, считаем, что он в кодировке UTF8, без преобразований.
	char pass[32] = { 0 };
	//Строка подключения к базе данных. Необходимо преобразование.
	char dblinkACP[128] = { 0 };
	//Путь к каталогу, может содержать национальные символы, необходимо преобразование.
	char *dumpdirACP = NULL;
	//Опциональный запрос для получения списка схем. Национальных символов не предполагается, считаем, что он в кодировке UTF8, без преобразований
	char *select = NULL;
	
	//Получаем необходимые данные из аргументов командной строки
	parseCmdLine(argc, argv, login, pass, dblinkACP, &dumpdirACP, &select);

	//printf("%s\n", dblinkACP);
	//printf("%s\n", dumpdirACP);
	
	
	//-------------------Секция конвертации строк, содержащих национальные символы в строки UTF-8

	//Строка подключения к базе данных. Под эту строку выделяется буфер при конвертации, ее нужно удалить, когда она будет не нужна
	char *dblink = winACPstr2utf8strBufAlloc(dblinkACP);
	if (dblink == NULL)
	{
		return (EXIT_FAILURE);
	}
	

	//Путь к каталогу. Под эту строку выделяется буфер при конвертации, ее нужно удалить, когда она будет не нужна
	char *dumpdir = winACPstr2utf8strBufAlloc(dumpdirACP);
	if (dumpdir == NULL)
	{
		free(dblink);
		return (EXIT_FAILURE);
	}
	
	//printf("%s\n", login);
	//printf("%s\n", pass);
	//printf("%s\n", dblink);
	//printf("%s\n", dumpdir);
	

	//-------------------------Секция инициализации Оракловых библиотек----------------------------
	//Структура для получения системного времени в главном потоке
	SYSTEMTIME mainThreadTimeStruct;
	//Хендл подключаемой библиотеки OCI.dll. По завершении работы нужно освободить, вызвав FreeLibrary(hOCIDll)
	HMODULE hOCIDll = NULL;
	//Структура окружения (Oracle Environment) Инициализируется функцией InitOraEnvironment. Особождается функцией CloseOraEnvironment
	OCIEnv *hOraEnv = NULL;
	//Структура для получения и вывода ошибок работы функций OCI.dll.Инициализируется функцией InitOraEnvironment. Особождается функцией CloseOraEnvironment
	OCIError *hOraErr = NULL;

	//СТруктура OCIServer для создания сессии Оракл для потока экспорта схем. Сессия закрывается вызовом функции CloseSession
	OCIServer *hOraServerExportThread = NULL;
	//СТруктура OCISvcCtx для создания сессии Оракл для потока экспорта схем. Сессия закрывается вызовом функции CloseSession
	OCISvcCtx *hOraSvcCtxExportThread = NULL;
	//СТруктура OCISession для создания сессии Оракл для потока экспорта схем. Сессия закрывается вызовом функции CloseSession
	OCISession *hOraSessionExportThread = NULL;

	//СТруктура OCIServer для создания сессии Оракл для потока получения файлов. Сессия закрывается вызовом функции CloseSession
	OCIServer *hOraServerRecieveThread = NULL;
	//СТруктура OCISvcCtx для создания сессии Оракл для потока получения файлов. Сессия закрывается вызовом функции CloseSession
	OCISvcCtx *hOraSvcCtxRecieveThread = NULL;
	//СТруктура OCISession для создания сессии Оракл для потока получения файлов. Сессия закрывается вызовом функции CloseSession
	OCISession *hOraSessionRecieveThread = NULL;

	GetLocalTime(&mainThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Запрос для получения схем - %s\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
		mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond, select);
	//printf("Используемый запрос - %s\n", select);
	
	GetLocalTime(&mainThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Подключаем библиотеку oci.dll\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear, 
		mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	//printf("Loading oci.dll library...\n");
	hOCIDll = LoadLibraryW(L"oci.dll");
	if (hOCIDll == NULL)
	{
		free(dblink);
		free(dumpdir);
		GetLocalTime(&mainThreadTimeStruct);
		printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Не могу загрузить oci.dll. Убедитесь, что Oracle Client необходимой разрядности установлен и добавлен в переменную окружения PATH\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
			mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
		exit(EXIT_FAILURE);
	}

	GetLocalTime(&mainThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Загружаем необходимые OCI функции\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
		mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	if (!LoadOciFunctions(hOCIDll))
	{
		GetLocalTime(&mainThreadTimeStruct);
		printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Не удалось загрузить все необходимые OCI функции. Проверьте версию oci.dll и порядок загрузки библиотек в переменной PATH\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
			mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
		free(dblink);
		free(dumpdir);
		FreeLibrary(hOCIDll);
		exit(EXIT_FAILURE);
	}

	GetLocalTime(&mainThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Инициализируем OCI окружение\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
		mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	if (!InitOraEnvironment(&hOraEnv, &hOraErr))
	{
		GetLocalTime(&mainThreadTimeStruct);
		printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Не получается инициализировать OCI окружение, приложение будет закрыто\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
			mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
		free(dblink);
		free(dumpdir);
		FreeLibrary(hOCIDll);
		return EXIT_FAILURE;
	}

	char tmpLogin[32] = { 0 };
	strcpy(tmpLogin, login);
	_strupr(tmpLogin);

	
	GetLocalTime(&mainThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Создаем сессию для потока экспорта\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
		mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	if (!CreateSession(hOraEnv, hOraErr, &hOraServerExportThread, &hOraSvcCtxExportThread, &hOraSessionExportThread, login, pass, dblink, !strcmp("SYS", tmpLogin)))
	{
		GetLocalTime(&mainThreadTimeStruct);
		printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Не получается подключиться к базе. Освобождаем ресурсы и завершаем работу приложения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
			mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
		
		CloseOraEnvironment(hOraEnv, hOraErr);
		FreeLibrary(hOCIDll);
		free(dblink);
		free(dumpdir);
		return EXIT_FAILURE;
	}


	GetLocalTime(&mainThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Создаем сессию для потока получения файлов\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
		mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	if (!CreateSession(hOraEnv, hOraErr, &hOraServerRecieveThread, &hOraSvcCtxRecieveThread, &hOraSessionRecieveThread, login, pass, dblink, !strcmp("SYS", tmpLogin)))
	{
		GetLocalTime(&mainThreadTimeStruct);
		printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Не получается подключиться к базе. Освобождаем ресурсы и завершаем работу приложения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
			mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);

		CloseSession(hOraErr, hOraServerExportThread, hOraSvcCtxExportThread, hOraSessionExportThread);
		CloseOraEnvironment(hOraEnv, hOraErr);
		FreeLibrary(hOCIDll);
		free(dblink);
		free(dumpdir);
		return EXIT_FAILURE;
	}


	GetLocalTime(&mainThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Выделяем память для заголовочной структуры списка схем\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
		mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	P_SCHEMA_LIST_STUCTURE listSchemaToExport = (P_SCHEMA_LIST_STUCTURE)malloc(sizeof(SCHEMA_LIST_STUCTURE));
	if (listSchemaToExport == NULL)
	{
		GetLocalTime(&mainThreadTimeStruct);
		printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Не получается выделить память. Освобождаем ресурсы и завершаем работу приложения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
			mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);

		CloseSession(hOraErr, hOraServerExportThread, hOraSvcCtxExportThread, hOraSessionExportThread);
		CloseSession(hOraErr, hOraServerRecieveThread, hOraSvcCtxRecieveThread, hOraSessionRecieveThread);
		CloseOraEnvironment(hOraEnv, hOraErr);
		FreeLibrary(hOCIDll);
		free(dblink);
		free(dumpdir);
		return EXIT_FAILURE;
	}

	//Переменная для получения количества пользователей. В следующем блоке мы получаем количество ВСЕХ пользователей в базе, и выделяем для всех памяти. Сделано это для упрощения кода, иначе придется парсить исходный запрос,
	//собирать из него select count (*)... Расход по памяти - около 300 КБ на 1000 пользователей. На момент написания программы на k5db было 1637. ДУмается, что 600 КБ не стоят такой оптимизации.
	long long UsersCount = 0;
	char * selectuserscount = "select count(*) from dba_users";

	GetLocalTime(&mainThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Получаем количество всех схем для выделения памяти\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
		mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	if (GetNumberFromSelect(hOraSvcCtxExportThread, hOraEnv, hOraErr, selectuserscount, &UsersCount) == FALSE)
	{
		GetLocalTime(&mainThreadTimeStruct);
		printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Ошибка получения количества схем. Освобождаем ресурсы и завершаем работу приложения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
			mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);

		CloseSession(hOraErr, hOraServerExportThread, hOraSvcCtxExportThread, hOraSessionExportThread);
		CloseSession(hOraErr, hOraServerRecieveThread, hOraSvcCtxRecieveThread, hOraSessionRecieveThread);
		CloseOraEnvironment(hOraEnv, hOraErr);
		FreeLibrary(hOCIDll);
		free(listSchemaToExport);
		free(dblink);
		free(dumpdir);
		return EXIT_FAILURE;
	}
	//printf("Users count - %lld\n", UsersCount);
	listSchemaToExport->pSchemaRows = NULL;
	listSchemaToExport->schemaCount = 0;

	GetLocalTime(&mainThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Выделяем память для реестра состояния выгружаемых схем\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
		mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	listSchemaToExport->pSchemaRows = (P_SCHEMA_ROW_STUCTURE)malloc(sizeof(SCHEMA_ROW_STUCTURE) * (size_t)UsersCount);
	if (listSchemaToExport->pSchemaRows == NULL)
	{
		GetLocalTime(&mainThreadTimeStruct);
		printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Не получается выделить память. Освобождаем ресурсы и завершаем работу приложения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
			mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);

		CloseSession(hOraErr, hOraServerExportThread, hOraSvcCtxExportThread, hOraSessionExportThread);
		CloseSession(hOraErr, hOraServerRecieveThread, hOraSvcCtxRecieveThread, hOraSessionRecieveThread);
		CloseOraEnvironment(hOraEnv, hOraErr);
		FreeLibrary(hOCIDll);
		free(listSchemaToExport);
		free(dblink);
		free(dumpdir);
		return EXIT_FAILURE;
	}

	
	OCIStmt *hOraSelectUsersStatement = NULL;
	long long counter = 0;
	char username[32] = { 0 };
	sword status = 0;

	GetLocalTime(&mainThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Получаем список пользователей и заполняем реестр для обработки экспорта схем\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
		mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	hOraSelectUsersStatement = GetStringsSetPrepare(hOraSvcCtxExportThread, hOraEnv, hOraErr, select, username, sizeof(username));
	if (hOraSelectUsersStatement == NULL)
	{
		GetLocalTime(&mainThreadTimeStruct);
		printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Не получается запросить список пользователей. Освобождаем ресурсы и завершаем работу приложения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
			mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);

		CloseSession(hOraErr, hOraServerExportThread, hOraSvcCtxExportThread, hOraSessionExportThread);
		CloseSession(hOraErr, hOraServerRecieveThread, hOraSvcCtxRecieveThread, hOraSessionRecieveThread);
		CloseOraEnvironment(hOraEnv, hOraErr);
		FreeLibrary(hOCIDll);
		free(listSchemaToExport->pSchemaRows);
		free(listSchemaToExport);
		free(dblink);
		free(dumpdir);
		return EXIT_FAILURE;
	}
	while ((status = GetStringsSetFetch(hOraSelectUsersStatement, hOraErr)) == OCI_SUCCESS || status == OCI_SUCCESS_WITH_INFO)
	{
		//printf("%s\n", username);
		//insert processing userlist here
		strcpy((listSchemaToExport->pSchemaRows)[listSchemaToExport->schemaCount].schema, username);
		(listSchemaToExport->pSchemaRows)[listSchemaToExport->schemaCount].status = 0;
		listSchemaToExport->schemaCount++;
		//end processing userlist here
	}
	if (status != OCI_SUCCESS && status != OCI_SUCCESS_WITH_INFO && status != OCI_NO_DATA)
	{
		GetLocalTime(&mainThreadTimeStruct);
		printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Не получается получить список пользователей. Освобождаем ресурсы и завершаем работу приложения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
			mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
		
		checkerr(hOraErr, status);
		FreeSqlHandle(hOraSelectUsersStatement);
		CloseSession(hOraErr, hOraServerExportThread, hOraSvcCtxExportThread, hOraSessionExportThread);
		CloseSession(hOraErr, hOraServerRecieveThread, hOraSvcCtxRecieveThread, hOraSessionRecieveThread);
		CloseOraEnvironment(hOraEnv, hOraErr);
		FreeLibrary(hOCIDll);
		free(listSchemaToExport->pSchemaRows);
		free(listSchemaToExport);
		free(dblink);
		free(dumpdir);
		return EXIT_FAILURE;
		
		
	}

	FreeSqlHandle(hOraSelectUsersStatement);


	GetLocalTime(&mainThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Количество пользователей для обработки - %d\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
		mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond, listSchemaToExport->schemaCount);
	//printf("Количество пользователей для обработки - %d\n", listSchemaToExport->schemaCount);
	//long long i;
	//for (i = 0; i < listSchemaToExport->schemaCount; i++)
	//{
	//	printf("%s, status - %d\n", (listSchemaToExport->pSchemaRows)[i].schema, (listSchemaToExport->pSchemaRows)[i].status);
	//}

	//time_t time1, time2;
	//time1 = time(NULL);
	//Sleep(5000);
	//time2 = time(NULL);
	//printf("Разница по времени - %lld секунд\n", time2 - time1);
	
	EXPORT_STUCTURE ExpStr;
	ExpStr.ExportList = listSchemaToExport;
	ExpStr.hOraSvcCtx = hOraSvcCtxExportThread;
	ExpStr.hOraEnv = hOraEnv;
	ExpStr.hOraErr = hOraErr;
	HANDLE hExportThread = NULL;

	hExportThread = (HANDLE)_beginthreadex(NULL, 0, ExportThread, &ExpStr, 0, NULL);

	RECIEVE_STUCTURE RcvStr;
	RcvStr.ExportList = listSchemaToExport;
	RcvStr.hOraSvcCtx = hOraSvcCtxRecieveThread;
	RcvStr.hOraEnv = hOraEnv;
	RcvStr.hOraErr = hOraErr;
	RcvStr.dumpDir = dumpdir;
	HANDLE hRecieveThread = NULL;
	
	hRecieveThread = (HANDLE)_beginthreadex(NULL, 0, RecieveThread, &RcvStr, 0, NULL);

	WaitForSingleObject(hExportThread, INFINITE);

	WaitForSingleObject(hRecieveThread, INFINITE);


	//for (i = 0; i < listSchemaToExport->schemaCount; i++)
	//{
	//	printf("%s, status - %d, filename - %s\n", (listSchemaToExport->pSchemaRows)[i].schema, (listSchemaToExport->pSchemaRows)[i].status,
	//		(listSchemaToExport->pSchemaRows)[i].filename);
	//}
	
	
	GetLocalTime(&mainThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Закрываем сессию для потока экспорта\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
		mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	CloseSession(hOraErr, hOraServerExportThread, hOraSvcCtxExportThread, hOraSessionExportThread);

	GetLocalTime(&mainThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Закрываем сессию для потока получения файлов\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
		mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	CloseSession(hOraErr, hOraServerRecieveThread, hOraSvcCtxRecieveThread, hOraSessionRecieveThread);

	GetLocalTime(&mainThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Закрываем OCI окружение\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
		mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	CloseOraEnvironment(hOraEnv, hOraErr);

	GetLocalTime(&mainThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Освобождаем библиотеку OCI.dll\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
		mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	FreeLibrary(hOCIDll);
	

	GetLocalTime(&mainThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Освобождаем выделенную память\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
		mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	free(listSchemaToExport->pSchemaRows);
	free(listSchemaToExport);
	free(dblink);
	free(dumpdir);


	GetLocalTime(&mainThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Завершаем работу приложения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
		mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	return 0;
}