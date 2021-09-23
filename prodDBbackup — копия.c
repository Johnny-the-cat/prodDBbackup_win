#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <windows.h>
#include "OraFunctions.h"
#include "StringConvert.h"

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

	printf("%s\n", login);
	printf("%s\n", pass);
	printf("%s\n", dblinkACP);
	printf("%s\n", dumpdirACP);
	printf("%s\n", select);
	
	//-------------------Секция конвертации строк, содержащих национальные символы в строки UTF-8

	//Строка подключения к базе данных. Под эту строку выделяется буфер при конвертации, ее нужно удалить, когда она будет не нужна
	char *dblink = winACPstr2utf8strBufAlloc(dblinkACP);
	if (dblink == NULL)
	{
		return (EXIT_FAILURE);
	}
	printf("%s\n", dblink);

	//Путь к каталогу. Под эту строку выделяется буфер при конвертации, ее нужно удалить, когда она будет не нужна
	char *dumpdir = winACPstr2utf8strBufAlloc(dumpdirACP);
	if (dumpdir == NULL)
	{
		free(dblink);
		return (EXIT_FAILURE);
	}
	printf("%s\n", dumpdir);


	//-------------------------Секция инициализации Оракловых библиотек----------------------------
	//Структура для получения системного времени в главном потоке
	SYSTEMTIME mainThreadTimeStruct;
	GetLocalTime(&mainThreadTimeStruct);
	printf("%02d_%02d_%04d_%02d_%02d_%02d.zip", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear, mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	//printf("Loading oci.dll library...\n");
	
	//Хендл подключаемой библиотеки OCI.dll. По завершении работы нужно освободить, вызвав FreeLibrary(hOCIDll)
	HMODULE hOCIDll = NULL;
	//Структура окружения (Oracle Environment) Инициализируется функцией InitOraEnvironment. Особождается функцией CloseOraEnvironment
	OCIEnv *hOraEnv = NULL;
	//Структура для получения и вывода ошибок работы функций OCI.dll.Инициализируется функцией InitOraEnvironment. Особождается функцией CloseOraEnvironment
	OCIError *hOraErr = NULL;

	hOCIDll = LoadLibrary(L"oci.dll");
	if (hOCIDll == NULL)
	{
		printf("Can't load oci.dll, check that Oracle Client has been installed and PATH environment variable\n");
		exit(EXIT_FAILURE);
	}

	if (!LoadOciFunctions(hOCIDll))
	{
		printf("Can't load OCI functions, OCI.dll is incompatible. Check that Oracle Client has been installed and PATH environment variable\n");
		FreeLibrary(hOCIDll);
		exit(EXIT_FAILURE);
	}

	if (!InitOraEnvironment())
	{
		printf("Can't Init OCI Environment for work, quiting...\n");
		_endthreadex(EXIT_FAILURE);
		return EXIT_FAILURE;
	}

	HANDLE hThread = NULL;

	hThread = (HANDLE)_beginthreadex(NULL, 0, ExportThread, ExpStr, 0, NULL);

	WaitForSingleObject(hThread, INFINITE);


	FreeLibrary(hOCIDll);
	
	
	free(dblink);
	free(dumpdir);

	return 0;
}