#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <windows.h>
#include "OraFunctions.h"
#include "StringConvert.h"

BOOL parseCmdLine(int argc, char* argv[], char *login, char *pass, char *dblink, char **dumpdir, char **select);

int main(int argc, char* argv[])
{
	//����� ��� �����������. �������� �� ���������� ��������� ������. ��� ��� �� ������ ���� �� ����������, �������, ��� �� � ��������� UTF8, ��� ��������������.
	char login[32] = { 0 };
	//������ ��� �����������. �������� �� ���������� ��������� ������. ��� ��� �� ������ ���� �� ����������, �������, ��� �� � ��������� UTF8, ��� ��������������.
	char pass[32] = { 0 };
	//������ ����������� � ���� ������. ���������� ��������������.
	char dblinkACP[128] = { 0 };
	//���� � ��������, ����� ��������� ������������ �������, ���������� ��������������.
	char *dumpdirACP = NULL;
	//������������ ������ ��� ��������� ������ ����. ������������ �������� �� ��������������, �������, ��� �� � ��������� UTF8, ��� ��������������
	char *select = NULL;
	
	//�������� ����������� ������ �� ���������� ��������� ������
	parseCmdLine(argc, argv, login, pass, dblinkACP, &dumpdirACP, &select);

	printf("%s\n", login);
	printf("%s\n", pass);
	printf("%s\n", dblinkACP);
	printf("%s\n", dumpdirACP);
	printf("%s\n", select);
	
	//-------------------������ ����������� �����, ���������� ������������ ������� � ������ UTF-8

	//������ ����������� � ���� ������. ��� ��� ������ ���������� ����� ��� �����������, �� ����� �������, ����� ��� ����� �� �����
	char *dblink = winACPstr2utf8strBufAlloc(dblinkACP);
	if (dblink == NULL)
	{
		return (EXIT_FAILURE);
	}
	printf("%s\n", dblink);

	//���� � ��������. ��� ��� ������ ���������� ����� ��� �����������, �� ����� �������, ����� ��� ����� �� �����
	char *dumpdir = winACPstr2utf8strBufAlloc(dumpdirACP);
	if (dumpdir == NULL)
	{
		free(dblink);
		return (EXIT_FAILURE);
	}
	printf("%s\n", dumpdir);


	//-------------------------������ ������������� ��������� ���������----------------------------
	//��������� ��� ��������� ���������� ������� � ������� ������
	SYSTEMTIME mainThreadTimeStruct;
	GetLocalTime(&mainThreadTimeStruct);
	printf("%02d_%02d_%04d_%02d_%02d_%02d.zip", mainThreadTimeStruct.wDay, mainThreadTimeStruct.wMonth, mainThreadTimeStruct.wYear, mainThreadTimeStruct.wHour, mainThreadTimeStruct.wMinute, mainThreadTimeStruct.wSecond);
	//printf("Loading oci.dll library...\n");
	
	//����� ������������ ���������� OCI.dll. �� ���������� ������ ����� ����������, ������ FreeLibrary(hOCIDll)
	HMODULE hOCIDll = NULL;
	//��������� ��������� (Oracle Environment) ���������������� �������� InitOraEnvironment. ������������ �������� CloseOraEnvironment
	OCIEnv *hOraEnv = NULL;
	//��������� ��� ��������� � ������ ������ ������ ������� OCI.dll.���������������� �������� InitOraEnvironment. ������������ �������� CloseOraEnvironment
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