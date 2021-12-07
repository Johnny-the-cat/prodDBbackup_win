#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdbool.h>
#include <windows.h>
#include <time.h>
#include "OraFunctions.h"
#include "StringConvert.h"
#include "threadFunctions.h"
#include <process.h>
#include "reportFunctions.h"

bool parseCmdLine(int argc, char* argv[], char *login, char *pass, char *dblink, char **dumpdir, char **select, char **jsonreportfile, bool *consistent);

int main(int argc, char* argv[])
{
	UINT CurrentCP = GetConsoleCP();
	SetConsoleOutputCP(65001);

	//Время начала работы программы в формате C-time
	time_t startTime;
	startTime = time(NULL);

	//Время начала работы программы в человеческом формате
	SYSTEMTIME startTimeStruct;
	GetLocalTime(&startTimeStruct);
	
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
	//Полное имя JSON отчета, может содержать национальные символы, необходимо преобразование.
	char *jsonreportfileACP = NULL;
	//Флаг консистентности бекапа
	bool consistent = FALSE;
	
	//Получаем необходимые данные из аргументов командной строки
	parseCmdLine(argc, argv, login, pass, dblinkACP, &dumpdirACP, &select, &jsonreportfileACP, &consistent);

	//printf("%s\n", dblinkACP);
	//printf("%s\n", dumpdirACP);
	//if (jsonreportfileACP != NULL)
	//{
	//	printf("jsonreportfileACP - %s\n", jsonreportfileACP);
	//}
	
	
	
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
	
	//printf("login - %s\n", login);
	//printf("pass - %s\n", pass);
	//printf("dblink - %s\n", dblink);
	//printf("dumpdir - %s\n", dumpdir);
	

	//-------------------------Секция инициализации Оракловых библиотек----------------------------
	//Структура для получения системного времени в главном потоке
	SYSTEMTIME mainThreadTimeStruct;
	//Хендл подключаемой библиотеки OCI.dll. По завершении работы нужно освободить, вызвав FreeLibrary(hOCIDll)
	HMODULE hOCIDll = NULL;
	//Структура окружения (Oracle Environment) Инициализируется функцией InitOraEnvironment. Особождается функцией CloseOraEnvironment
	OCIEnv *hOraEnv = NULL;
	//Структура для получения и вывода ошибок работы функций OCI.dll.Инициализируется функцией InitOraEnvironment. Особождается функцией CloseOraEnvironment
	OCIError *hOraErr = NULL;

	//СТруктура OCIServer для создания сессии Оракл для главного потока. Сессия закрывается вызовом функции CloseSession
	OCIServer *hOraServerMainThread = NULL;
	//СТруктура OCISvcCtx для создания сессии Оракл для главного потока. Сессия закрывается вызовом функции CloseSession
	OCISvcCtx *hOraSvcCtxMainThread = NULL;
	//СТруктура OCISession для создания сессии Оракл для главного потока. Сессия закрывается вызовом функции CloseSession
	OCISession *hOraSessionMainThread = NULL;


	GetLocalTime(&mainThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Запрос для получения схем - %s\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
		mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond, select);

	if (consistent == TRUE)
	{
		GetLocalTime(&mainThreadTimeStruct);
		printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Указана опция создания консистентного бекапа. Может понадобится увеличенный размер табличного пространства UNDO\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
			mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	}
	
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
		FreeLibrary(hOCIDll);
		free(dblink);
		free(dumpdir);
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
		FreeLibrary(hOCIDll);
		free(dblink);
		free(dumpdir);
		return EXIT_FAILURE;
	}

	char tmpLogin[32] = { 0 };
	strcpy(tmpLogin, login);
	_strupr(tmpLogin);

	
	GetLocalTime(&mainThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Создаем сессию для главного потока\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
		mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	if (!CreateSession(hOraEnv, hOraErr, &hOraServerMainThread, &hOraSvcCtxMainThread, &hOraSessionMainThread, login, pass, dblink, !strcmp("SYS", tmpLogin)))
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


	//GetLocalTime(&mainThreadTimeStruct);
	//printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Выделяем память для заголовочной структуры списка схем\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
	//	mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	P_SCHEMA_LIST_STUCTURE listSchemaToExport = (P_SCHEMA_LIST_STUCTURE)malloc(sizeof(SCHEMA_LIST_STUCTURE));
	if (listSchemaToExport == NULL)
	{
		GetLocalTime(&mainThreadTimeStruct);
		printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Не получается выделить память для заголовочной структуры списка схем. Освобождаем ресурсы и завершаем работу приложения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
			mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);

		CloseSession(hOraErr, hOraServerMainThread, hOraSvcCtxMainThread, hOraSessionMainThread);
		CloseOraEnvironment(hOraEnv, hOraErr);
		FreeLibrary(hOCIDll);
		free(dblink);
		free(dumpdir);
		return EXIT_FAILURE;
	}

	//Переменная для получения количества пользователей. В следующем блоке мы получаем количество ВСЕХ пользователей в базе, и выделяем для всех памяти. Сделано это для упрощения кода, иначе придется парсить исходный запрос,
	//собирать из него select count (*)... Расход по памяти - около 320 КБ на 1000 пользователей. На момент написания программы на основном сервере БД было 1637. Думается, что 600 КБ не стоят такой оптимизации.
	long long UsersCount = 0;
	char * selectuserscount = "select count(*) from dba_users";

	//GetLocalTime(&mainThreadTimeStruct);
	//printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Получаем количество всех схем для выделения памяти\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
	//	mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	if (GetNumberFromSelect(hOraSvcCtxMainThread, hOraEnv, hOraErr, selectuserscount, &UsersCount) == FALSE)
	{
		GetLocalTime(&mainThreadTimeStruct);
		printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Ошибка получения количества всех схем. Освобождаем ресурсы и завершаем работу приложения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
			mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);

		CloseSession(hOraErr, hOraServerMainThread, hOraSvcCtxMainThread, hOraSessionMainThread);
		CloseOraEnvironment(hOraEnv, hOraErr);
		FreeLibrary(hOCIDll);
		free(dblink);
		free(dumpdir);
		free(listSchemaToExport);
		return EXIT_FAILURE;
	}
	listSchemaToExport->pSchemaRows = NULL;
	listSchemaToExport->schemaCount = 0;

	//GetLocalTime(&mainThreadTimeStruct);
	//printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Выделяем память для реестра состояния выгружаемых схем\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
	//	mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	listSchemaToExport->pSchemaRows = (P_SCHEMA_ROW_STUCTURE)malloc(sizeof(SCHEMA_ROW_STUCTURE) * (size_t)UsersCount);
	if (listSchemaToExport->pSchemaRows == NULL)
	{
		GetLocalTime(&mainThreadTimeStruct);
		printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Не получается выделить память для реестра состояния выгружаемых схем. Освобождаем ресурсы и завершаем работу приложения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
			mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);

		CloseSession(hOraErr, hOraServerMainThread, hOraSvcCtxMainThread, hOraSessionMainThread);
		CloseOraEnvironment(hOraEnv, hOraErr);
		FreeLibrary(hOCIDll);
		free(dblink);
		free(dumpdir);
		free(listSchemaToExport);
		return EXIT_FAILURE;
	}

	
	OCIStmt *hOraSelectUsersStatement = NULL;
	long long counter = 0;
	char username[32] = { 0 };
	sword status = 0;

	//GetLocalTime(&mainThreadTimeStruct);
	//printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Получаем список пользователей и заполняем реестр для обработки экспорта схем\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
	//	mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	hOraSelectUsersStatement = GetStringsSetPrepare(hOraSvcCtxMainThread, hOraEnv, hOraErr, select, username, sizeof(username));
	if (hOraSelectUsersStatement == NULL)
	{
		GetLocalTime(&mainThreadTimeStruct);
		printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Не получается запросить список пользователей для бекапа. Освобождаем ресурсы и завершаем работу приложения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
			mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);

		CloseSession(hOraErr, hOraServerMainThread, hOraSvcCtxMainThread, hOraSessionMainThread);
		CloseOraEnvironment(hOraEnv, hOraErr);
		FreeLibrary(hOCIDll);
		free(dblink);
		free(dumpdir);
		free(listSchemaToExport->pSchemaRows);
		free(listSchemaToExport);
		return EXIT_FAILURE;
	}
	while ((status = GetStringsSetFetch(hOraSelectUsersStatement, hOraErr)) == OCI_SUCCESS || status == OCI_SUCCESS_WITH_INFO)
	{
		//printf("%s\n", username);
		//insert processing userlist here
		strcpy((listSchemaToExport->pSchemaRows)[listSchemaToExport->schemaCount].schema, username);
		(listSchemaToExport->pSchemaRows)[listSchemaToExport->schemaCount].exportStatus = NOT_PROCESSED;
		(listSchemaToExport->pSchemaRows)[listSchemaToExport->schemaCount].receiveStatus = NOT_PROCESSED;
		listSchemaToExport->schemaCount++;
		//end processing userlist here
	}
	if (status != OCI_SUCCESS && status != OCI_SUCCESS_WITH_INFO && status != OCI_NO_DATA)
	{
		checkerr(hOraErr, status);
		GetLocalTime(&mainThreadTimeStruct);
		printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Ошибка при получении списка пользователей для бекапа. Освобождаем ресурсы и завершаем работу приложения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
			mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
		
		FreeSqlHandle(hOraSelectUsersStatement);
		CloseSession(hOraErr, hOraServerMainThread, hOraSvcCtxMainThread, hOraSessionMainThread);
		CloseOraEnvironment(hOraEnv, hOraErr);
		FreeLibrary(hOCIDll);
		free(dblink);
		free(dumpdir);
		free(listSchemaToExport->pSchemaRows);
		free(listSchemaToExport);
		return EXIT_FAILURE;
		
	}

	FreeSqlHandle(hOraSelectUsersStatement);


	GetLocalTime(&mainThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Количество пользователей для обработки - %d\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
		mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond, listSchemaToExport->schemaCount);

	//int tmp;
	//for (tmp = 0; tmp < listSchemaToExport->schemaCount; tmp++)
	//{
	//	printf("%s\n", (listSchemaToExport->pSchemaRows)[tmp].schema);
	//}

	
	//-----------------------------------Подсчет количества потоков для задач экспорта датапампа и инициализация пула сессий для экспорта--------------------------------------
	//Количество потоков экспорта равно количеству специально созданных датапамп каталогов на сервере БД. Каталог должен иметь имя типа PRODDBBACKUP_DIR_n
	long long ExportThreadsCount = 0;
	long long DatapumpDirsCount = 0;
	char * selectDataPumpDirsCount = "select count(*) from all_directories where DIRECTORY_NAME like 'PRODDBBACKUP_DIR_%'";
	P_DATAPUMP_DIR_ROW_STRUCTURE DatapumpDirList = NULL;

	//GetLocalTime(&mainThreadTimeStruct);
	//printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Получаем количество датапамп каталогов PRODDBBACKUP_DIR_n на сервере БД\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
	//	mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	if (GetNumberFromSelect(hOraSvcCtxMainThread, hOraEnv, hOraErr, selectDataPumpDirsCount, &DatapumpDirsCount) == FALSE)
	{
		GetLocalTime(&mainThreadTimeStruct);
		printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Ошибка получения количества датапамп каталогов  PRODDBBACKUP_DIR_n на сервере. Освобождаем ресурсы и завершаем работу приложения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
			mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);

		CloseSession(hOraErr, hOraServerMainThread, hOraSvcCtxMainThread, hOraSessionMainThread);
		CloseOraEnvironment(hOraEnv, hOraErr);
		FreeLibrary(hOCIDll);
		free(dblink);
		free(dumpdir);
		free(listSchemaToExport->pSchemaRows);
		free(listSchemaToExport);
		return EXIT_FAILURE;
	}
	GetLocalTime(&mainThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Количество датапамп каталогов PRODDBBACKUP_DIR_n на сервере - %lld\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
		mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond, DatapumpDirsCount);

	if (DatapumpDirsCount == 0)
	{
		ExportThreadsCount = 1;

		GetLocalTime(&mainThreadTimeStruct);
		printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Количество потоков для экспорта - %lld\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
			mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond, ExportThreadsCount);

		//GetLocalTime(&mainThreadTimeStruct);
		//printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Выделяем память для списка датапамп каталогов\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
		//	mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
		DatapumpDirList = (P_DATAPUMP_DIR_ROW_STRUCTURE)malloc((size_t)ExportThreadsCount * sizeof(DATAPUMP_DIR_ROW_STRUCTURE));
		if (DatapumpDirList == NULL)
		{
			GetLocalTime(&mainThreadTimeStruct);
			printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Ошибка выделения памяти для списка датапамп каталогов. Освобождаем ресурсы и завершаем работу приложения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
				mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);

			CloseSession(hOraErr, hOraServerMainThread, hOraSvcCtxMainThread, hOraSessionMainThread);
			CloseOraEnvironment(hOraEnv, hOraErr);
			FreeLibrary(hOCIDll);
			free(dblink);
			free(dumpdir);
			free(listSchemaToExport->pSchemaRows);
			free(listSchemaToExport);
			return EXIT_FAILURE;
		}
		memset(DatapumpDirList, 0, (size_t)ExportThreadsCount * sizeof(DATAPUMP_DIR_ROW_STRUCTURE));

		strcpy(DatapumpDirList[0].datapumpDirName, "DATA_PUMP_DIR");
		DatapumpDirList[0].serialNumber = 1;

		//int iii;
		//for (iii = 0; iii < ExportThreadsCount; iii++)
		//{
		//	printf("DirNumber - %d, name - %s\n", DatapumpDirList[iii].serialNumber, DatapumpDirList[iii].datapumpDirName);
		//}
	}
	else
	{
		ExportThreadsCount = DatapumpDirsCount;

		GetLocalTime(&mainThreadTimeStruct);
		printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Количество потоков для экспорта - %lld\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
			mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond, ExportThreadsCount);

		//GetLocalTime(&mainThreadTimeStruct);
		//printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Выделяем память для списка датапамп каталогов\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
		//	mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
		DatapumpDirList = (P_DATAPUMP_DIR_ROW_STRUCTURE)malloc((size_t)DatapumpDirsCount * sizeof(DATAPUMP_DIR_ROW_STRUCTURE));
		if (DatapumpDirList == NULL)
		{
			GetLocalTime(&mainThreadTimeStruct);
			printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Ошибка выделения памяти для списка датапамп каталогов. Освобождаем ресурсы и завершаем работу приложения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
				mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);

			CloseSession(hOraErr, hOraServerMainThread, hOraSvcCtxMainThread, hOraSessionMainThread);
			CloseOraEnvironment(hOraEnv, hOraErr);
			FreeLibrary(hOCIDll);
			free(dblink);
			free(dumpdir);
			free(listSchemaToExport->pSchemaRows);
			free(listSchemaToExport);
			return EXIT_FAILURE;
		}
		memset(DatapumpDirList, 0, (size_t)DatapumpDirsCount * sizeof(DATAPUMP_DIR_ROW_STRUCTURE));
		
		//GetLocalTime(&mainThreadTimeStruct);
		//printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Получаем список датапамп каталогов PRODDBBACKUP_DIR_n c сервера\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
		//	mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);

		OCIStmt *hOraSelectDirsStatement = NULL;
		long long dirsCounter = 0;
		char datapump_directory_name[32];
		char * selectDataPumpDirs = "select DIRECTORY_NAME from all_directories where DIRECTORY_NAME like 'PRODDBBACKUP_DIR_%' order by 1";

		hOraSelectDirsStatement = GetStringsSetPrepare(hOraSvcCtxMainThread, hOraEnv, hOraErr, selectDataPumpDirs, datapump_directory_name, sizeof(datapump_directory_name));
		if (hOraSelectUsersStatement == NULL)
		{
			GetLocalTime(&mainThreadTimeStruct);
			printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Не получается запросить список датапамп каталогов PRODDBBACKUP_DIR_n. Освобождаем ресурсы и завершаем работу приложения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
				mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);

			CloseSession(hOraErr, hOraServerMainThread, hOraSvcCtxMainThread, hOraSessionMainThread);
			CloseOraEnvironment(hOraEnv, hOraErr);
			FreeLibrary(hOCIDll);
			free(dblink);
			free(dumpdir);
			free(listSchemaToExport->pSchemaRows);
			free(listSchemaToExport);
			free(DatapumpDirList);
			return EXIT_FAILURE;
		}
		while ((status = GetStringsSetFetch(hOraSelectDirsStatement, hOraErr)) == OCI_SUCCESS || status == OCI_SUCCESS_WITH_INFO)
		{
			//printf("%s\n", username);
			//insert processing userlist here
			strcpy((DatapumpDirList)[dirsCounter].datapumpDirName, datapump_directory_name);
			DatapumpDirList[dirsCounter].serialNumber = (int)dirsCounter + 1;
			dirsCounter++;
			//end processing userlist here
		}
		if (status != OCI_SUCCESS && status != OCI_SUCCESS_WITH_INFO && status != OCI_NO_DATA)
		{
			checkerr(hOraErr, status);
			GetLocalTime(&mainThreadTimeStruct);
			printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Ошибка при получении списка датапамп каталогов PRODDBBACKUP_DIR_n. Освобождаем ресурсы и завершаем работу приложения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
				mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);

			FreeSqlHandle(hOraSelectDirsStatement);
			CloseSession(hOraErr, hOraServerMainThread, hOraSvcCtxMainThread, hOraSessionMainThread);
			CloseOraEnvironment(hOraEnv, hOraErr);
			FreeLibrary(hOCIDll);
			free(dblink);
			free(dumpdir);
			free(listSchemaToExport->pSchemaRows);
			free(listSchemaToExport);
			free(DatapumpDirList);
			return EXIT_FAILURE;

		}

		FreeSqlHandle(hOraSelectDirsStatement);

		//int iii;
		//for (iii = 0; iii < DatapumpDirsCount; iii++)
		//{
		//	printf("DirNumber - %d, name - %s\n", DatapumpDirList[iii].serialNumber, DatapumpDirList[iii].datapumpDirName);
		//}

	}


	//GetLocalTime(&mainThreadTimeStruct);
	//printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Выделяем память для массива указателей hOraServer для потоков экспорта\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
	//	mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);

	//Память для указателей OCIServer для создания сессии Оракл для потока получения файлов. Сессия закрывается вызовом функции CloseSession
	OCIServer **hOraServerExportThread = (OCIServer **)malloc((size_t)ExportThreadsCount * sizeof(OCIServer *));
	if (hOraServerExportThread == NULL)
	{
		GetLocalTime(&mainThreadTimeStruct);
		printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Не получается выделить память для массива указателей hOraServer для потоков экспорта. Освобождаем ресурсы и завершаем работу приложения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
			mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);

		CloseSession(hOraErr, hOraServerMainThread, hOraSvcCtxMainThread, hOraSessionMainThread);
		CloseOraEnvironment(hOraEnv, hOraErr);
		FreeLibrary(hOCIDll);
		free(dblink);
		free(dumpdir);
		free(listSchemaToExport->pSchemaRows);
		free(listSchemaToExport);
		free(DatapumpDirList);
		return EXIT_FAILURE;
	}
	memset(hOraServerExportThread, 0, (size_t)ExportThreadsCount * sizeof(OCIServer *));


	//GetLocalTime(&mainThreadTimeStruct);
	//printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Выделяем память для массива указателей OCISvcCtx для потоков экспорта\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
	//	mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);

	//Память для указателей OCISvcCtx для создания сессии Оракл для потока получения файлов. Сессия закрывается вызовом функции CloseSession
	OCISvcCtx **hOraSvcCtxExportThread = (OCISvcCtx **)malloc((size_t)ExportThreadsCount * sizeof(OCISvcCtx *));
	if (hOraSvcCtxExportThread == NULL)
	{
		GetLocalTime(&mainThreadTimeStruct);
		printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Не получается выделить память для массива указателей OCISvcCtx для потоков экспорта. Освобождаем ресурсы и завершаем работу приложения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
			mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);

		CloseSession(hOraErr, hOraServerMainThread, hOraSvcCtxMainThread, hOraSessionMainThread);
		CloseOraEnvironment(hOraEnv, hOraErr);
		FreeLibrary(hOCIDll);
		free(dblink);
		free(dumpdir);
		free(listSchemaToExport->pSchemaRows);
		free(listSchemaToExport);
		free(DatapumpDirList);
		free(hOraServerExportThread);
		return EXIT_FAILURE;
	}
	memset(hOraSvcCtxExportThread, 0, (size_t)ExportThreadsCount * sizeof(OCISvcCtx *));


	//GetLocalTime(&mainThreadTimeStruct);
	//printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Выделяем память для массива указателей OCISession для потоков экспорта\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
	//	mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	
	//Память для указателей OCISession для создания сессии Оракл для потока получения файлов. Сессия закрывается вызовом функции CloseSession
	OCISession **hOraSessionExportThread = (OCISession **)malloc((size_t)ExportThreadsCount * sizeof(OCISession *));
	if (hOraSessionExportThread == NULL)
	{
		GetLocalTime(&mainThreadTimeStruct);
		printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Не получается выделить память для массива указателей OCISession для потоков экспорта. Освобождаем ресурсы и завершаем работу приложения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
			mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);

		CloseSession(hOraErr, hOraServerMainThread, hOraSvcCtxMainThread, hOraSessionMainThread);
		CloseOraEnvironment(hOraEnv, hOraErr);
		FreeLibrary(hOCIDll);
		free(dblink);
		free(dumpdir);
		free(listSchemaToExport->pSchemaRows);
		free(listSchemaToExport);
		free(DatapumpDirList);
		free(hOraServerExportThread);
		free(hOraSvcCtxExportThread);
		return EXIT_FAILURE;
	}
	memset(hOraSessionExportThread, 0, (size_t)ExportThreadsCount * sizeof(OCISession *));


	//----------------------------------------------------Подсчет количества потоков для получения--------------------------------------------------------
	int ReceiveThreadsCount = 0;
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);

	ReceiveThreadsCount = sysinfo.dwNumberOfProcessors;
	GetLocalTime(&mainThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Количество логических процессоров - %d\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
		mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond, ReceiveThreadsCount);

	if (ReceiveThreadsCount > listSchemaToExport->schemaCount)
	{
		ReceiveThreadsCount = listSchemaToExport->schemaCount;
	}
	GetLocalTime(&mainThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Количество потоков для приема файлов - %d\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
		mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond, ReceiveThreadsCount);

	
	
	//GetLocalTime(&mainThreadTimeStruct);
	//printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Выделяем память для массива указателей hOraServer для потоков получения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
	//	mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	
	//Память для указателей OCIServer для создания сессии Оракл для потока получения файлов. Сессия закрывается вызовом функции CloseSession
	OCIServer **hOraServerRecieveThread = (OCIServer **)malloc(ReceiveThreadsCount * sizeof(OCIServer *));
	if (hOraServerRecieveThread == NULL)
	{
		GetLocalTime(&mainThreadTimeStruct);
		printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Не получается выделить память для массива указателей hOraServer для потоков получения. Освобождаем ресурсы и завершаем работу приложения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
			mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);

		CloseSession(hOraErr, hOraServerMainThread, hOraSvcCtxMainThread, hOraSessionMainThread);
		CloseOraEnvironment(hOraEnv, hOraErr);
		FreeLibrary(hOCIDll);
		free(dblink);
		free(dumpdir);
		free(listSchemaToExport->pSchemaRows);
		free(listSchemaToExport);
		free(DatapumpDirList);
		free(hOraServerExportThread);
		free(hOraSvcCtxExportThread);
		free(hOraSessionExportThread);
		return EXIT_FAILURE;
	}
	memset(hOraServerRecieveThread, 0, ReceiveThreadsCount * sizeof(OCIServer *));


	//GetLocalTime(&mainThreadTimeStruct);
	//printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Выделяем память для массива указателей OCISvcCtx для потоков получения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
	//	mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	
	//Память для указателей OCISvcCtx для создания сессии Оракл для потока получения файлов. Сессия закрывается вызовом функции CloseSession
	OCISvcCtx **hOraSvcCtxRecieveThread = (OCISvcCtx **)malloc(ReceiveThreadsCount * sizeof(OCISvcCtx *));
	if (hOraSvcCtxRecieveThread == NULL)
	{
		GetLocalTime(&mainThreadTimeStruct);
		printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Не получается выделить память для массива указателей OCISvcCtx для потоков получения. Освобождаем ресурсы и завершаем работу приложения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
			mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);

		CloseSession(hOraErr, hOraServerMainThread, hOraSvcCtxMainThread, hOraSessionMainThread);
		CloseOraEnvironment(hOraEnv, hOraErr);
		FreeLibrary(hOCIDll);
		free(dblink);
		free(dumpdir);
		free(listSchemaToExport->pSchemaRows);
		free(listSchemaToExport);
		free(DatapumpDirList);
		free(hOraServerExportThread);
		free(hOraSvcCtxExportThread);
		free(hOraSessionExportThread);
		free(hOraServerRecieveThread);
		return EXIT_FAILURE;
	}
	memset(hOraSvcCtxRecieveThread, 0, ReceiveThreadsCount * sizeof(OCISvcCtx *));


	//GetLocalTime(&mainThreadTimeStruct);
	//printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Выделяем память для массива указателей OCISession для потоков получения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
	//	mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	
	//Память для указателей OCISession для создания сессии Оракл для потока получения файлов. Сессия закрывается вызовом функции CloseSession
	OCISession **hOraSessionRecieveThread = (OCISession **)malloc(ReceiveThreadsCount * sizeof(OCISession *));
	if (hOraSessionRecieveThread == NULL)
	{
		GetLocalTime(&mainThreadTimeStruct);
		printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Не получается выделить память для массива указателей OCISession для потоков получения. Освобождаем ресурсы и завершаем работу приложения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
			mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);

		CloseSession(hOraErr, hOraServerMainThread, hOraSvcCtxMainThread, hOraSessionMainThread);
		CloseOraEnvironment(hOraEnv, hOraErr);
		FreeLibrary(hOCIDll);
		free(dblink);
		free(dumpdir);
		free(listSchemaToExport->pSchemaRows);
		free(listSchemaToExport);
		free(DatapumpDirList);
		free(hOraServerExportThread);
		free(hOraSvcCtxExportThread);
		free(hOraSessionExportThread);
		free(hOraServerRecieveThread);
		free(hOraSvcCtxRecieveThread);
		return EXIT_FAILURE;
	}
	memset(hOraSessionRecieveThread, 0, ReceiveThreadsCount * sizeof(OCISession *));


	//---------------------------------------------------------Устанавливаем пул сессий для потоков экспорта----------------------------------------------------
	int EstablishedExportSessionCounter;
	for (EstablishedExportSessionCounter = 0; EstablishedExportSessionCounter < ExportThreadsCount; EstablishedExportSessionCounter++)
	{
		//GetLocalTime(&mainThreadTimeStruct);
		//printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Создаем сессию %d для потока экспорта\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
		//	mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond, EstablishedExportSessionCounter + 1);
		if (!CreateSession(hOraEnv, hOraErr, hOraServerExportThread + EstablishedExportSessionCounter, hOraSvcCtxExportThread + EstablishedExportSessionCounter, hOraSessionExportThread + EstablishedExportSessionCounter,
			login, pass, dblink, !strcmp("SYS", tmpLogin)))
		{
			GetLocalTime(&mainThreadTimeStruct);
			printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Не получается создать достаточно подключений для экспорта. Освобождаем ресурсы и завершаем работу приложения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
				mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);

			CloseSession(hOraErr, hOraServerMainThread, hOraSvcCtxMainThread, hOraSessionMainThread);
			for (EstablishedExportSessionCounter--; EstablishedExportSessionCounter >= 0; EstablishedExportSessionCounter--)
			{
				CloseSession(hOraErr, *(hOraServerExportThread + EstablishedExportSessionCounter), *(hOraSvcCtxExportThread + EstablishedExportSessionCounter), *(hOraSessionExportThread + EstablishedExportSessionCounter));
			}
			CloseOraEnvironment(hOraEnv, hOraErr);
			FreeLibrary(hOCIDll);
			free(dblink);
			free(dumpdir);
			free(listSchemaToExport->pSchemaRows);
			free(listSchemaToExport);
			free(DatapumpDirList);
			free(hOraServerExportThread);
			free(hOraSvcCtxExportThread);
			free(hOraSessionExportThread);
			free(hOraServerRecieveThread);
			free(hOraSvcCtxRecieveThread);
			free(hOraSessionRecieveThread);
			return EXIT_FAILURE;
		}
		
	}

	

	//----------------------------------------------------------Устанавливаем пул сессий для потоков получения----------------------------------
	int EstablishedReceiveSessionCounter;
	for (EstablishedReceiveSessionCounter = 0; EstablishedReceiveSessionCounter < ReceiveThreadsCount; EstablishedReceiveSessionCounter++)
	{
		//GetLocalTime(&mainThreadTimeStruct);
		//printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Создаем сессию %d для потока получения файлов\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
		//	mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond, EstablishedReceiveSessionCounter + 1);
		if (!CreateSession(hOraEnv, hOraErr, hOraServerRecieveThread + EstablishedReceiveSessionCounter, hOraSvcCtxRecieveThread + EstablishedReceiveSessionCounter, hOraSessionRecieveThread + EstablishedReceiveSessionCounter,
			login, pass, dblink, !strcmp("SYS", tmpLogin)))
		{
			GetLocalTime(&mainThreadTimeStruct);
			printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Не получается создать достаточно подключений для получения файлов. Освобождаем ресурсы и завершаем работу приложения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
				mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);

			CloseSession(hOraErr, hOraServerMainThread, hOraSvcCtxMainThread, hOraSessionMainThread);

			for (EstablishedExportSessionCounter--; EstablishedExportSessionCounter >= 0; EstablishedExportSessionCounter--)
			{
				CloseSession(hOraErr, *(hOraServerExportThread + EstablishedExportSessionCounter), *(hOraSvcCtxExportThread + EstablishedExportSessionCounter), *(hOraSessionExportThread + EstablishedExportSessionCounter));
			}

			for (EstablishedReceiveSessionCounter--; EstablishedReceiveSessionCounter >= 0; EstablishedReceiveSessionCounter--)
			{
				CloseSession(hOraErr, *(hOraServerRecieveThread + EstablishedReceiveSessionCounter), *(hOraSvcCtxRecieveThread + EstablishedReceiveSessionCounter), *(hOraSessionRecieveThread + EstablishedReceiveSessionCounter));
			}

			CloseOraEnvironment(hOraEnv, hOraErr);
			FreeLibrary(hOCIDll);
			free(dblink);
			free(dumpdir);
			free(listSchemaToExport->pSchemaRows);
			free(listSchemaToExport);
			free(DatapumpDirList);
			free(hOraServerExportThread);
			free(hOraSvcCtxExportThread);
			free(hOraSessionExportThread);
			free(hOraServerRecieveThread);
			free(hOraSvcCtxRecieveThread);
			free(hOraSessionRecieveThread);
			return EXIT_FAILURE;
		}
		
	}
	

	//-----------------------------------------------Инициализируем память для потоков экспорта-----------------------------------------
	//Память для указателей на структуры экспорта для передачи в потоки
	P_EXPORT_STUCTURE ExportStructs = (P_EXPORT_STUCTURE)malloc((size_t)ExportThreadsCount * sizeof(EXPORT_STUCTURE));
	if (ExportStructs == NULL)
	{
		GetLocalTime(&mainThreadTimeStruct);
		printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Не получается выделить память для структур для потоков экспорта. Освобождаем ресурсы и завершаем работу приложения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
			mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);

		CloseSession(hOraErr, hOraServerMainThread, hOraSvcCtxMainThread, hOraSessionMainThread);

		for (EstablishedExportSessionCounter--; EstablishedExportSessionCounter >= 0; EstablishedExportSessionCounter--)
		{
			CloseSession(hOraErr, *(hOraServerExportThread + EstablishedExportSessionCounter), *(hOraSvcCtxExportThread + EstablishedExportSessionCounter), *(hOraSessionExportThread + EstablishedExportSessionCounter));
		}

		for (EstablishedReceiveSessionCounter--; EstablishedReceiveSessionCounter >= 0; EstablishedReceiveSessionCounter--)
		{
			CloseSession(hOraErr, *(hOraServerRecieveThread + EstablishedReceiveSessionCounter), *(hOraSvcCtxRecieveThread + EstablishedReceiveSessionCounter), *(hOraSessionRecieveThread + EstablishedReceiveSessionCounter));
		}
		
		CloseOraEnvironment(hOraEnv, hOraErr);
		FreeLibrary(hOCIDll);
		free(dblink);
		free(dumpdir);
		free(listSchemaToExport->pSchemaRows);
		free(listSchemaToExport);
		free(DatapumpDirList);
		free(hOraServerExportThread);
		free(hOraSvcCtxExportThread);
		free(hOraSessionExportThread);
		free(hOraServerRecieveThread);
		free(hOraSvcCtxRecieveThread);
		free(hOraSessionRecieveThread);
		return EXIT_FAILURE;
	}
	memset(ExportStructs, 0, (size_t)ExportThreadsCount * sizeof(EXPORT_STUCTURE));

	HANDLE *ExportThreadHandle = (HANDLE *)malloc((size_t)ExportThreadsCount * sizeof(HANDLE));
	if (ExportThreadHandle == NULL)
	{
		GetLocalTime(&mainThreadTimeStruct);
		printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Не получается выделить память для хенлов потоков экспорта. Освобождаем ресурсы и завершаем работу приложения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
			mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);

		CloseSession(hOraErr, hOraServerMainThread, hOraSvcCtxMainThread, hOraSessionMainThread);

		for (EstablishedExportSessionCounter--; EstablishedExportSessionCounter >= 0; EstablishedExportSessionCounter--)
		{
			CloseSession(hOraErr, *(hOraServerExportThread + EstablishedExportSessionCounter), *(hOraSvcCtxExportThread + EstablishedExportSessionCounter), *(hOraSessionExportThread + EstablishedExportSessionCounter));
		}

		for (EstablishedReceiveSessionCounter--; EstablishedReceiveSessionCounter >= 0; EstablishedReceiveSessionCounter--)
		{
			CloseSession(hOraErr, *(hOraServerRecieveThread + EstablishedReceiveSessionCounter), *(hOraSvcCtxRecieveThread + EstablishedReceiveSessionCounter), *(hOraSessionRecieveThread + EstablishedReceiveSessionCounter));
		}

		CloseOraEnvironment(hOraEnv, hOraErr);
		FreeLibrary(hOCIDll);
		free(dblink);
		free(dumpdir);
		free(listSchemaToExport->pSchemaRows);
		free(listSchemaToExport);
		free(DatapumpDirList);
		free(hOraServerExportThread);
		free(hOraSvcCtxExportThread);
		free(hOraSessionExportThread);
		free(hOraServerRecieveThread);
		free(hOraSvcCtxRecieveThread);
		free(hOraSessionRecieveThread);
		free(ExportStructs);
		return EXIT_FAILURE;
	}
	memset(ExportThreadHandle, 0, (size_t)ExportThreadsCount * sizeof(HANDLE));
	
	
	//-----------------------------------------------Инициализируем память для потоков приема файлов-----------------------------------------
	//Память для указателей на структуры приема для передачи в потоки
	P_RECIEVE_STUCTURE ReceiveStructs = (P_RECIEVE_STUCTURE)malloc((size_t)ReceiveThreadsCount * sizeof(RECIEVE_STUCTURE));
	if (ReceiveStructs == NULL)
	{
		GetLocalTime(&mainThreadTimeStruct);
		printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Не получается выделить память для структур для потоков приема файлов. Освобождаем ресурсы и завершаем работу приложения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
			mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);

		CloseSession(hOraErr, hOraServerMainThread, hOraSvcCtxMainThread, hOraSessionMainThread);

		for (EstablishedExportSessionCounter--; EstablishedExportSessionCounter >= 0; EstablishedExportSessionCounter--)
		{
			CloseSession(hOraErr, *(hOraServerExportThread + EstablishedExportSessionCounter), *(hOraSvcCtxExportThread + EstablishedExportSessionCounter), *(hOraSessionExportThread + EstablishedExportSessionCounter));
		}

		for (EstablishedReceiveSessionCounter--; EstablishedReceiveSessionCounter >= 0; EstablishedReceiveSessionCounter--)
		{
			CloseSession(hOraErr, *(hOraServerRecieveThread + EstablishedReceiveSessionCounter), *(hOraSvcCtxRecieveThread + EstablishedReceiveSessionCounter), *(hOraSessionRecieveThread + EstablishedReceiveSessionCounter));
		}

		CloseOraEnvironment(hOraEnv, hOraErr);
		FreeLibrary(hOCIDll);
		free(dblink);
		free(dumpdir);
		free(listSchemaToExport->pSchemaRows);
		free(listSchemaToExport);
		free(DatapumpDirList);
		free(hOraServerExportThread);
		free(hOraSvcCtxExportThread);
		free(hOraSessionExportThread);
		free(hOraServerRecieveThread);
		free(hOraSvcCtxRecieveThread);
		free(hOraSessionRecieveThread);
		free(ExportStructs);
		free(ExportThreadHandle);
		return EXIT_FAILURE;
	}
	memset(ReceiveStructs, 0, (size_t)ReceiveThreadsCount * sizeof(RECIEVE_STUCTURE));

	HANDLE *ReceiveThreadHandle = (HANDLE *)malloc((size_t)ReceiveThreadsCount * sizeof(HANDLE));
	if (ReceiveThreadHandle == NULL)
	{
		GetLocalTime(&mainThreadTimeStruct);
		printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Не получается выделить память для хенлов потоков приема файлов. Освобождаем ресурсы и завершаем работу приложения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
			mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);

		CloseSession(hOraErr, hOraServerMainThread, hOraSvcCtxMainThread, hOraSessionMainThread);

		for (EstablishedExportSessionCounter--; EstablishedExportSessionCounter >= 0; EstablishedExportSessionCounter--)
		{
			CloseSession(hOraErr, *(hOraServerExportThread + EstablishedExportSessionCounter), *(hOraSvcCtxExportThread + EstablishedExportSessionCounter), *(hOraSessionExportThread + EstablishedExportSessionCounter));
		}

		for (EstablishedReceiveSessionCounter--; EstablishedReceiveSessionCounter >= 0; EstablishedReceiveSessionCounter--)
		{
			CloseSession(hOraErr, *(hOraServerRecieveThread + EstablishedReceiveSessionCounter), *(hOraSvcCtxRecieveThread + EstablishedReceiveSessionCounter), *(hOraSessionRecieveThread + EstablishedReceiveSessionCounter));
		}

		CloseOraEnvironment(hOraEnv, hOraErr);
		FreeLibrary(hOCIDll);
		free(dblink);
		free(dumpdir);
		free(listSchemaToExport->pSchemaRows);
		free(listSchemaToExport);
		free(DatapumpDirList);
		free(hOraServerExportThread);
		free(hOraSvcCtxExportThread);
		free(hOraSessionExportThread);
		free(hOraServerRecieveThread);
		free(hOraSvcCtxRecieveThread);
		free(hOraSessionRecieveThread);
		free(ExportStructs);
		free(ExportThreadHandle);
		free(ReceiveStructs);
		return EXIT_FAILURE;
	}
	memset(ReceiveThreadHandle, 0, (size_t)ReceiveThreadsCount * sizeof(HANDLE));
	
	
	//Критические секции для согласования потоков экспорта и приема файлов
	CRITICAL_SECTION ExportCriticalSection, ReceiveCriticalSection;
	if (InitializeCriticalSectionAndSpinCount(&ExportCriticalSection, 4000) == 0 || InitializeCriticalSectionAndSpinCount(&ReceiveCriticalSection, 4000) == 0)
	{
		GetLocalTime(&mainThreadTimeStruct);
		printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Не получается инициализировать критические секции для потоков экспорта и приема файлов. Освобождаем ресурсы и завершаем работу приложения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
			mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);

		CloseSession(hOraErr, hOraServerMainThread, hOraSvcCtxMainThread, hOraSessionMainThread);

		for (EstablishedExportSessionCounter--; EstablishedExportSessionCounter >= 0; EstablishedExportSessionCounter--)
		{
			CloseSession(hOraErr, *(hOraServerExportThread + EstablishedExportSessionCounter), *(hOraSvcCtxExportThread + EstablishedExportSessionCounter), *(hOraSessionExportThread + EstablishedExportSessionCounter));
		}

		for (EstablishedReceiveSessionCounter--; EstablishedReceiveSessionCounter >= 0; EstablishedReceiveSessionCounter--)
		{
			CloseSession(hOraErr, *(hOraServerRecieveThread + EstablishedReceiveSessionCounter), *(hOraSvcCtxRecieveThread + EstablishedReceiveSessionCounter), *(hOraSessionRecieveThread + EstablishedReceiveSessionCounter));
		}

		CloseOraEnvironment(hOraEnv, hOraErr);
		FreeLibrary(hOCIDll);
		free(dblink);
		free(dumpdir);
		free(listSchemaToExport->pSchemaRows);
		free(listSchemaToExport);
		free(DatapumpDirList);
		free(hOraServerExportThread);
		free(hOraSvcCtxExportThread);
		free(hOraSessionExportThread);
		free(hOraServerRecieveThread);
		free(hOraSvcCtxRecieveThread);
		free(hOraSessionRecieveThread);
		free(ExportStructs);
		free(ExportThreadHandle);
		free(ReceiveStructs);
		free(ReceiveThreadHandle);
		return EXIT_FAILURE;
	}
	
	//Переменная для хранения критической ошибки. При ошибке экспорта все потоки завершат свою работу
	//Концепция изменилась. После со схемой программа не завешает работу а переходит к следующей. Не используется
	int CriticalError = FALSE;

	int i;

	GetLocalTime(&mainThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Запускаем потоки экспорта\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
		mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	for (i = 0; i < ExportThreadsCount; i++)
	{
		ExportStructs[i].ExportList = listSchemaToExport;
		ExportStructs[i].hOraSvcCtx = *(hOraSvcCtxExportThread + i);
		ExportStructs[i].hOraEnv = hOraEnv;
		ExportStructs[i].hOraErr = hOraErr;
		ExportStructs[i].datapumpDir = DatapumpDirList[i].datapumpDirName;
		ExportStructs[i].threadNumber = i + 1;
		ExportStructs[i].ExportCriticalSection = &ExportCriticalSection;
		ExportStructs[i].CriticalError = &CriticalError;
		ExportStructs[i].Consistent = consistent;

		ExportThreadHandle[i] = (HANDLE)_beginthreadex(NULL, 0, ExportThread, ExportStructs + i, 0, NULL);
	}
	
	GetLocalTime(&mainThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Запускаем потоки получения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
		mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	for (i = 0; i < ReceiveThreadsCount; i++)
	{
		ReceiveStructs[i].ExportList = listSchemaToExport;
		ReceiveStructs[i].hOraSvcCtx = *(hOraSvcCtxRecieveThread + i);
		ReceiveStructs[i].hOraEnv = hOraEnv;
		ReceiveStructs[i].hOraErr = hOraErr;
		ReceiveStructs[i].dumpDir = dumpdir;
		ReceiveStructs[i].threadNumber = i + 1;
		ReceiveStructs[i].ReceiveCriticalSection = &ReceiveCriticalSection;
		ReceiveStructs[i].CriticalError = &CriticalError;

		ReceiveThreadHandle[i] = (HANDLE)_beginthreadex(NULL, 0, RecieveThread, ReceiveStructs + i, 0, NULL);
	}

	//printf("Жду завержения потоков\n");
	WaitForMultipleObjects((DWORD)ExportThreadsCount, ExportThreadHandle, TRUE, INFINITE);
	WaitForMultipleObjects((DWORD)ReceiveThreadsCount, ReceiveThreadHandle, TRUE, INFINITE);

	GetLocalTime(&mainThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Все потоки завершили свою работу\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
		mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	

	//for (i = 0; i < listSchemaToExport->schemaCount; i++)
	//{
	//	printf("%s, status - %d, filename - %s\n", (listSchemaToExport->pSchemaRows)[i].schema, (listSchemaToExport->pSchemaRows)[i].status,
	//		(listSchemaToExport->pSchemaRows)[i].filename);
	//}
	
	DeleteCriticalSection(&ExportCriticalSection);
	DeleteCriticalSection(&ReceiveCriticalSection);
	
	//GetLocalTime(&mainThreadTimeStruct);
	//printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Закрываем сессию для главного потока\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
	//	mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	CloseSession(hOraErr, hOraServerMainThread, hOraSvcCtxMainThread, hOraSessionMainThread);


	//GetLocalTime(&mainThreadTimeStruct);
	//printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Закрываем сессии для потоков экспорта\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
	//	mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	for (EstablishedExportSessionCounter--; EstablishedExportSessionCounter >= 0; EstablishedExportSessionCounter--)
	{
		CloseSession(hOraErr, *(hOraServerExportThread + EstablishedExportSessionCounter), *(hOraSvcCtxExportThread + EstablishedExportSessionCounter), *(hOraSessionExportThread + EstablishedExportSessionCounter));
	}
	

	//GetLocalTime(&mainThreadTimeStruct);
	//printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Закрываем сессии для потоков получения файлов\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
	//	mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	for (EstablishedReceiveSessionCounter--; EstablishedReceiveSessionCounter >= 0; EstablishedReceiveSessionCounter--)
	{
		CloseSession(hOraErr, *(hOraServerRecieveThread + EstablishedReceiveSessionCounter), *(hOraSvcCtxRecieveThread + EstablishedReceiveSessionCounter), *(hOraSessionRecieveThread + EstablishedReceiveSessionCounter));
	}


	//GetLocalTime(&mainThreadTimeStruct);
	//printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Закрываем OCI окружение\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
	//	mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	CloseOraEnvironment(hOraEnv, hOraErr);

	//GetLocalTime(&mainThreadTimeStruct);
	//printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Освобождаем библиотеку OCI.dll\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
	//	mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	FreeLibrary(hOCIDll);
	

	//Формируем json файл отчета, если указан параметр jsonreportfile=
	if (jsonreportfileACP != NULL)
	{
		GetLocalTime(&mainThreadTimeStruct);
		printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Формируем файл отчета\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
			mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
		generateJSON(startTime, startTimeStruct, listSchemaToExport, jsonreportfileACP);
	}


	//GetLocalTime(&mainThreadTimeStruct);
	//printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Освобождаем выделенную память\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
	//	mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);

	free(dblink);
	free(dumpdir);
	free(listSchemaToExport->pSchemaRows);
	free(listSchemaToExport);
	free(DatapumpDirList);
	free(hOraServerExportThread);
	free(hOraSvcCtxExportThread);
	free(hOraSessionExportThread);
	free(hOraServerRecieveThread);
	free(hOraSvcCtxRecieveThread);
	free(hOraSessionRecieveThread);
	free(ExportStructs);
	free(ExportThreadHandle);
	free(ReceiveStructs);
	free(ReceiveThreadHandle);


	GetLocalTime(&mainThreadTimeStruct);
	printf("%02d.%02d.%04d %02d:%02d:%02d [mainThread] - Завершаем работу приложения\n", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear,
		mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	
	SetConsoleOutputCP(CurrentCP);
	return 0;
}