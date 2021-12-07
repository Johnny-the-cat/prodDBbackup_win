#define _CRT_SECURE_NO_WARNINGS

//Третья версия API - OCIEnv, OCIError и хендлы сессий передаются в функции в качестве паметров, передаваемые строки - char (UTF8)
#include <stdio.h>
#include <stdbool.h>
#include <windows.h>
#include "oci.h"
#include "oratypes.h"
#include "orl.h"
#include "ocipfndfn.h"
#include <process.h>
#include <tlhelp32.h>
#include <Commctrl.h>
#include "OraFunctions.h"
#include <strsafe.h>

//Заголовки для сжатия
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
# include <direct.h>
# include <io.h>

#include "zip.h"

//#define USEWIN32IOAPI

#include "iowin32.h"

//#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FOPEN_FUNC(filename, mode) utf8_fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)


#define WRITEBUFFERSIZE (16384)
#define MAXFILENAME (256)


#define CONSOLE_ERROR_OUTPUT
//#define MESSAGEBOX_ERROR_OUTPUT

//#define DEBUG_LOGGING

pOCIEnvCreate OCIEnvCreate;

pOCIEnvNlsCreate OCIEnvNlsCreate;

pOCITerminate OCITerminate;

pOCIHandleAlloc OCIHandleAlloc;

pOCIServerAttach OCIServerAttach;

pOCIAttrSet OCIAttrSet;

pOCISessionBegin OCISessionBegin;

pOCISessionEnd OCISessionEnd;

pOCIStmtPrepare OCIStmtPrepare;

pOCIDefineByPos OCIDefineByPos;

pOCIStmtExecute OCIStmtExecute;

pOCIBindByName OCIBindByName;

pOCIBindByPos OCIBindByPos;

pOCITransCommit OCITransCommit;

pOCIHandleFree OCIHandleFree;

pOCIServerDetach OCIServerDetach;

pOCIStmtFetch2 OCIStmtFetch2;

pOCIErrorGet OCIErrorGet;

pOCIRawAllocSize OCIRawAllocSize;

pOCIRawAssignBytes OCIRawAssignBytes;

pOCILobFileSetName OCILobFileSetName;

pOCIDescriptorAlloc OCIDescriptorAlloc;

pOCIDescriptorFree OCIDescriptorFree;

pOCILobFileOpen OCILobFileOpen;

pOCILobFileClose OCILobFileClose;

pOCILobGetLength2 OCILobGetLength2;

pOCILobRead2 OCILobRead2;

pOCILobFileExists OCILobFileExists;


/*Функция получает адреса нужных процедур и функций из oci.dll*/
bool LoadOciFunctions(HMODULE hOCIDll)
{
	OCIEnvCreate = (pOCIEnvCreate)GetProcAddress(hOCIDll,
		"OCIEnvCreate");
	if (OCIEnvCreate == NULL) {
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"LoadOciFunctions: can't load OCIEnvCreate", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("LoadOciFunctions: can't load OCIEnvCreate\n");
#endif
		return FALSE;
	}
	

	OCIEnvNlsCreate = (pOCIEnvNlsCreate)GetProcAddress(hOCIDll,
		"OCIEnvNlsCreate");
	if (OCIEnvNlsCreate == NULL) {
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"LoadOciFunctions: can't load OCIEnvNlsCreate", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("LoadOciFunctions: can't load OCIEnvNlsCreate\n");
#endif
		return FALSE;
	}
	

	OCITerminate = (pOCITerminate)GetProcAddress(hOCIDll,
		"OCITerminate");
	if (OCITerminate == NULL) {
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"LoadOciFunctions: can't load OCITerminate", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("LoadOciFunctions: can't load OCITerminate\n");
#endif		
		return FALSE;
	}
	

	OCIHandleAlloc = (pOCIHandleAlloc)GetProcAddress(hOCIDll,
		"OCIHandleAlloc");
	if (OCIHandleAlloc == NULL) {
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"LoadOciFunctions: can't load OCIHandleAlloc", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("LoadOciFunctions: can't load OCIHandleAlloc\n");
#endif
		return FALSE;
	}
	
	
	OCIServerAttach = (pOCIServerAttach)GetProcAddress(hOCIDll,
		"OCIServerAttach");
	if (OCIServerAttach == NULL) {
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"LoadOciFunctions: can't load OCIServerAttach", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("LoadOciFunctions: can't load OCIServerAttach\n");
#endif
		return FALSE;
	}
	
	
	OCIAttrSet = (pOCIAttrSet)GetProcAddress(hOCIDll,
		"OCIAttrSet");
	if (OCIAttrSet == NULL) {
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"LoadOciFunctions: can't load OCIAttrSet", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("LoadOciFunctions: can't load OCIAttrSet\n");
#endif
		return FALSE;
	}
	
	
	OCISessionBegin = (pOCISessionBegin)GetProcAddress(hOCIDll,
		"OCISessionBegin");
	if (OCISessionBegin == NULL) {
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"LoadOciFunctions: can't load OCISessionBegin", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("LoadOciFunctions: can't load OCISessionBegin\n");
#endif
		return FALSE;
	}
	
	
	OCISessionEnd = (pOCISessionEnd)GetProcAddress(hOCIDll,
		"OCISessionEnd");
	if (OCISessionEnd == NULL) {
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"LoadOciFunctions: can't load OCISessionEnd", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("LoadOciFunctions: can't load OCISessionEnd\n");
#endif
		return FALSE;
	}
	

	OCIStmtPrepare = (pOCIStmtPrepare)GetProcAddress(hOCIDll,
		"OCIStmtPrepare");
	if (OCIStmtPrepare == NULL) {
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"LoadOciFunctions: can't load OCIStmtPrepare", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("LoadOciFunctions: can't load OCIStmtPrepare\n");
#endif
		return FALSE;
	}
	

	OCIDefineByPos = (pOCIDefineByPos)GetProcAddress(hOCIDll,
		"OCIDefineByPos");
	if (OCIDefineByPos == NULL) {
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"LoadOciFunctions: can't load OCIDefineByPos", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("LoadOciFunctions: can't load OCIDefineByPos\n");
#endif
		return FALSE;
	}
	

	OCIStmtExecute = (pOCIStmtExecute)GetProcAddress(hOCIDll,
		"OCIStmtExecute");
	if (OCIStmtExecute == NULL) {
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"LoadOciFunctions: can't load OCIStmtExecute", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("LoadOciFunctions: can't load OCIStmtExecute\n");
#endif
		return FALSE;
	}
	
	
	OCIBindByName = (pOCIBindByName)GetProcAddress(hOCIDll,
		"OCIBindByName");
	if (OCIBindByName == NULL) {
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"LoadOciFunctions: can't load OCIBindByName", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("LoadOciFunctions: can't load OCIBindByName\n");
#endif
		return FALSE;
	}
	
	
	OCIBindByPos = (pOCIBindByPos)GetProcAddress(hOCIDll,
		"OCIBindByPos");
	if (OCIBindByPos == NULL) {
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"LoadOciFunctions: can't load OCIBindByPos", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("LoadOciFunctions: can't load OCIBindByPos\n");
#endif
		return FALSE;
	}
	
	
	OCITransCommit = (pOCITransCommit)GetProcAddress(hOCIDll,
		"OCITransCommit");
	if (OCITransCommit == NULL) {
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"LoadOciFunctions: can't load OCITransCommit", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("LoadOciFunctions: can't load OCITransCommit\n");
#endif
		return FALSE;
	}
	
	
	OCIHandleFree = (pOCIHandleFree)GetProcAddress(hOCIDll,
		"OCIHandleFree");
	if (OCIHandleFree == NULL) {
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"LoadOciFunctions: can't load OCIHandleFree", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("LoadOciFunctions: can't load OCIHandleFree\n");
#endif
		return FALSE;
	}
	
	
	OCIServerDetach = (pOCIServerDetach)GetProcAddress(hOCIDll,
		"OCIServerDetach");
	if (OCIServerDetach == NULL) {
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"LoadOciFunctions: can't load OCIServerDetach", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("LoadOciFunctions: can't load OCIServerDetach\n");
#endif
		return FALSE;
	}
	
	
	OCIStmtFetch2 = (pOCIStmtFetch2)GetProcAddress(hOCIDll,
		"OCIStmtFetch2");
	if (OCIStmtFetch2 == NULL) {
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"LoadOciFunctions: can't load OCIStmtFetch2", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("LoadOciFunctions: can't load OCIStmtFetch2\n");
#endif
		return FALSE;
	}
	
	
	OCIErrorGet = (pOCIErrorGet)GetProcAddress(hOCIDll,
		"OCIErrorGet");
	if (OCIErrorGet == NULL) {
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"LoadOciFunctions: can't load OCIErrorGet", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("LoadOciFunctions: can't load OCIErrorGet\n");
#endif
		return FALSE;
	}
	
	
	OCIRawAllocSize = (pOCIRawAllocSize)GetProcAddress(hOCIDll,
		"OCIRawAllocSize");
	if (OCIRawAllocSize == NULL) {
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"LoadOciFunctions: can't load OCIRawAllocSize", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("LoadOciFunctions: can't load OCIRawAllocSize\n");
#endif
		return FALSE;
	}
	
	
	OCIRawAssignBytes = (pOCIRawAssignBytes)GetProcAddress(hOCIDll,
		"OCIRawAssignBytes");
	if (OCIRawAssignBytes == NULL) {
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"LoadOciFunctions: can't load OCIRawAssignBytes", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("LoadOciFunctions: can't load OCIRawAssignBytes\n");
#endif
		return FALSE;
	}
	
	
	OCILobFileSetName = (pOCILobFileSetName)GetProcAddress(hOCIDll,
		"OCILobFileSetName");
	if (OCILobFileSetName == NULL) {
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"LoadOciFunctions: can't load OCILobFileSetName", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("LoadOciFunctions: can't load OCILobFileSetName\n");
#endif
		return FALSE;
	}
	

	
	OCIDescriptorAlloc = (pOCIDescriptorAlloc)GetProcAddress(hOCIDll,
		"OCIDescriptorAlloc");
	if (OCIDescriptorAlloc == NULL) {
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"LoadOciFunctions: can't load OCIDescriptorAlloc", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("LoadOciFunctions: can't load OCIDescriptorAlloc\n");
#endif
		return FALSE;
	}
	
	
	OCIDescriptorFree = (pOCIDescriptorFree)GetProcAddress(hOCIDll,
		"OCIDescriptorFree");
	if (OCIDescriptorFree == NULL) {
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"LoadOciFunctions: can't load OCIDescriptorFree", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("LoadOciFunctions: can't load OCIDescriptorFree\n");
#endif
		return FALSE;
	}
	
	
	OCILobFileOpen = (pOCILobFileOpen)GetProcAddress(hOCIDll,
		"OCILobFileOpen");
	if (OCILobFileOpen == NULL) {
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"LoadOciFunctions: can't load OCILobFileOpen", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("LoadOciFunctions: can't load OCILobFileOpen\n");
#endif
		return FALSE;
	}
	
	
	OCILobFileClose = (pOCILobFileClose)GetProcAddress(hOCIDll,
		"OCILobFileClose");
	if (OCILobFileClose == NULL) {
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"LoadOciFunctions: can't load OCILobFileClose", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("LoadOciFunctions: can't load OCILobFileClose\n");
#endif
		return FALSE;
	}
	
	
	OCILobGetLength2 = (pOCILobGetLength2)GetProcAddress(hOCIDll,
		"OCILobGetLength2");
	if (OCILobGetLength2 == NULL) {
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"LoadOciFunctions: can't load OCILobGetLength2", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("LoadOciFunctions: can't load OCILobGetLength2\n");
#endif
		return FALSE;
	}
	

	OCILobRead2 = (pOCILobRead2)GetProcAddress(hOCIDll,
		"OCILobRead2");
	if (OCILobRead2 == NULL) {
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"LoadOciFunctions: can't load OCILobRead2", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("LoadOciFunctions: can't load OCILobRead2\n");
#endif
		return FALSE;
	}
	
	
	OCILobFileExists = (pOCILobFileExists)GetProcAddress(hOCIDll,
		"OCILobFileExists");
	if (OCILobFileExists == NULL) {
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"LoadOciFunctions: can't load OCILobFileExists", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("LoadOciFunctions: can't load OCILobFileExists\n");
#endif
		return FALSE;
	}
	
	return TRUE;
}

void checkerr(OCIError *errhp, sword status)
{
	text errbuf[512];
	WCHAR wchar_errbuf[512];
	sb4 errcode = 0;

	switch (status)
	{
	case OCI_SUCCESS:
		//(void)printf("OCI_SUCCESS\n");
		break;

	case OCI_SUCCESS_WITH_INFO:
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Error - OCI_SUCCESS_WITH_INFO", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		(void)printf("Error - OCI_SUCCESS_WITH_INFO\n");
#endif
		break;

	case OCI_NEED_DATA:
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Error - OCI_NEED_DATA", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		(void)printf("Error - OCI_NEED_DATA\n");
#endif
		break;

	case OCI_NO_DATA:
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Error - OCI_NODATA", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		(void)printf("Error - OCI_NODATA\n");
#endif
		break;

	case OCI_ERROR:
		(void)OCIErrorGet((dvoid *)errhp, (ub4)1, (text *)NULL, &errcode,
			errbuf, (ub4) sizeof(errbuf), OCI_HTYPE_ERROR);
		MultiByteToWideChar(CP_UTF8, 0, (char *)errbuf, 512, wchar_errbuf, 512);
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, wchar_errbuf, L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		(void)printf("Error - %.*s\n", 512, errbuf);
#endif
		break;

	case OCI_INVALID_HANDLE:
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Error - OCI_INVALID_HANDLE", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		(void)printf("Error - OCI_INVALID_HANDLE\n");
#endif
		break;

	case OCI_STILL_EXECUTING:
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Error - OCI_STILL_EXECUTE", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		(void)printf("Error - OCI_STILL_EXECUTE\n");
#endif
		break;

	case OCI_CONTINUE:
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Error - OCI_CONTINUE", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		(void)printf("Error - OCI_CONTINUE\n");
#endif
		break;

	default:
		break;
	}
}


/*Функция инициализирует окружение OCIEnv и OCIError*/
bool InitOraEnvironment(OCIEnv **hOraEnv, OCIError **hOraErr)
{
	OCIEnv *localHandleOraEnv = NULL;
	OCIError *localHanleOraErr = NULL;

	
	if (OCIEnvCreate((OCIEnv **)&localHandleOraEnv,
		//(ub4)OCI_DEFAULT | OCI_OBJECT,
		(ub4)OCI_THREADED,
		(const void  *)0,
		(const void  * (*)(void  *, size_t))0,
		(const void  * (*)(void  *, void  *, size_t))0,
		(const void(*)(void  *, void  *))0,
		(size_t)0, (void  **)0))

	{
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Application error, InitOraEnvironment function - can not create OCI Environment handle", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("Application error, InitOraEnvironment function - can not create OCI Environment handle\n");
#endif
		return FALSE;
	}
	

	localHanleOraErr = NULL;
	if (OCIHandleAlloc((const void *)localHandleOraEnv,
		(void **)&localHanleOraErr,
		OCI_HTYPE_ERROR,
		(size_t)0,
		(void **)0))
	{
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Application error, InitOraEnvironment function - can not allocate OCI Error handle", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("Application error, InitOraEnvironment function - can not allocate OCI Error handle\n");
#endif
		return FALSE;
	}
	

	*hOraEnv = localHandleOraEnv;
	*hOraErr = localHanleOraErr;

	return TRUE;

}

/*Процедура освобождает Error Handle и Environment Handle завершает сеанс работы с OCI*/
void CloseOraEnvironment(OCIEnv *hOraEnv, OCIError *hOraErr)
{
	if (OCIHandleFree(hOraErr, OCI_HTYPE_ERROR))
	{
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Application error, CloseOraEnvironment function - can not free OCI Error handle", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("Application error, CloseOraEnvironment function - can not free OCI Error handle\n");
#endif
	}

	if (OCIHandleFree(hOraEnv, OCI_HTYPE_ENV))
	{
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Application error, CloseOraEnvironment function - can not free OCI Env handle", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("Application error, CloseOraEnvironment function - can not free OCI Env handle\n");
#endif
	}

	OCITerminate(OCI_DEFAULT);
}


/*Инициируем OCISvcCtx, OCIServer, OCISession и подулючаемся к серверу*/
bool CreateSession(OCIEnv *hOraEnv, OCIError *hOraErr, OCIServer **hOraServer, OCISvcCtx **hOraSvcCtx, OCISession **hOraSession, char *usernameutf8, char *passwordutf8, char *dbnameutf8, bool assysdba)
{
	OCIServer *localHandleOraServer;
	OCISvcCtx *localHandleOraSvcCtx;
	OCISession *localHandleSession;


	localHandleOraServer = NULL;	
	if (OCIHandleAlloc((const void *)hOraEnv,
		(void **)&localHandleOraServer,
		OCI_HTYPE_SERVER,
		(size_t)0,
		(dvoid **)0))
	{
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Application error, CreateSession function - can not allocate server handle", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("Application error, CreateSession function - can not allocate server handle\n");
#endif
		return FALSE;
	}
	

	if (OCIServerAttach(localHandleOraServer, hOraErr, (const OraText *)dbnameutf8, (sb4)strlen(dbnameutf8), (ub4)OCI_DEFAULT) != OCI_SUCCESS)
	{
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Не могу подключиться к серверу БД. Проверьте TNS или строку подключения. Возможно, ошибка сети", L"Ошибка", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("Unable to attach to server. Check TNS name or connect string. May be network error\n");
#endif
		return FALSE;
	}

	localHandleOraSvcCtx = NULL;	
	if (OCIHandleAlloc((const void *)hOraEnv,
		(void **)&localHandleOraSvcCtx,
		OCI_HTYPE_SVCCTX,
		(size_t)0,
		(dvoid **)0))
	{
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Application error, CreateSession function - can not allocate service handle", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("Application error, CreateSession function - can not allocate service handle\n");
#endif
		return FALSE;
	}
	

	if (OCIAttrSet((void *)localHandleOraSvcCtx, OCI_HTYPE_SVCCTX, (void *)localHandleOraServer, (ub4)0, OCI_ATTR_SERVER, (OCIError *)hOraErr) != OCI_SUCCESS)
	{
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Application error, CreateSession function - unable to put server handle to service context", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("Application error, CreateSession function - unable to put server handle to service context for\n");
#endif
		return FALSE;
	}
	
	/*Выделяем хендл для сессии Администратора*/
	localHandleSession = NULL;
	if (OCIHandleAlloc((const void *)hOraEnv, (void **)&localHandleSession, (ub4)OCI_HTYPE_SESSION, (size_t)0, (void **)0))
	{
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Application error, CreateSession function - can not allocate session handle", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("Application error, CreateSession function - can not allocate session handle\n");
#endif
		return FALSE;
	}
	

	/*Устанавливаем аттрибут "Имя пользователя"*/
	if (OCIAttrSet((void *)localHandleSession, (ub4)OCI_HTYPE_SESSION, (void *)usernameutf8, (ub4)strlen(usernameutf8), (ub4)OCI_ATTR_USERNAME, hOraErr) != OCI_SUCCESS)
	{
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Application error, CreateSession function - unable to set username to session handle", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("Application error, CreateSession function - unable to set username to session handle\n");
#endif
		return FALSE;
	}

	/*Устанавливаем аттрибут "Пароль"*/
	if (OCIAttrSet((void *)localHandleSession, (ub4)OCI_HTYPE_SESSION, (void *)passwordutf8, (ub4)strlen(passwordutf8), (ub4)OCI_ATTR_PASSWORD, hOraErr) != OCI_SUCCESS)
	{
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Application error, CreateSession function - unable to set password to session handle", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("Application error, CreateSession function - unable to set password to session handle\n");
#endif
		return FALSE;
	}

	/*Устанавливаем сессию*/
	if (OCISessionBegin(localHandleOraSvcCtx, hOraErr, localHandleSession, OCI_CRED_RDBMS, (ub4)(OCI_DEFAULT | (assysdba ? OCI_SYSDBA : 0))) != OCI_SUCCESS)
	{
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Не получается установить рабочую сессию. Проверьте правильность пользователя и пароля", L"Ошибка", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("Unable to begin session. Check your login and password is correct\n");
#endif
		return FALSE;
	}

	if (OCIAttrSet((void *)localHandleOraSvcCtx, (ub4)OCI_HTYPE_SVCCTX, (void *)localHandleSession, (ub4)0, (ub4)OCI_ATTR_SESSION, hOraErr) != OCI_SUCCESS)
	{
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Application error, CreateSession function - unable to set session handle into the service context handle", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("Application error, CreateSession function - unable to set session handle into the service context handle.\n");
#endif
		return FALSE;
	}

	*hOraServer = localHandleOraServer;
	*hOraSvcCtx = localHandleOraSvcCtx;
	*hOraSession = localHandleSession;

	return TRUE;
}

/*Отключаемся от сервера, и оосвобождаем хендлы OCISvcCtx, OCIServer, OCISession*/
void CloseSession(OCIError *hOraErr, OCIServer *hOraServer, OCISvcCtx *hOraSvcCtx, OCISession *hOraSession)
{
	if (OCISessionEnd(hOraSvcCtx, hOraErr, hOraSession, OCI_DEFAULT))
	{
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Application error, CloseSession function - unable to end session", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("Application error, CloseSession function - unable to end session.\n");
#endif
	}

	if (OCIHandleFree(hOraSession, OCI_HTYPE_SESSION))
	{
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Application error, CloseSession function - unable to free session handle", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("Application error, CloseSession function - unable to free session handle\n");
#endif
	}

	if (OCIHandleFree(hOraSvcCtx, OCI_HTYPE_SVCCTX))
	{
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Application error, CloseSession function - unable to free service context handle", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("Application error, CloseSession function - unable to free service context handle\n");
#endif
	}

	if (OCIServerDetach(hOraServer, hOraErr, (ub4)OCI_DEFAULT))
	{
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Application error, CloseSession function - unable to detach server", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("Application error, CloseSession function - unable to detach server\n");
#endif
	}

	if (OCIHandleFree(hOraServer, OCI_HTYPE_SERVER))
	{
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Application error, CloseSession function - unable to free server handle", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("Application error, CloseSession function - unable to free server handle\n");
#endif
	}
}

OCIStmt *GetStringsSetPrepare(OCISvcCtx *hOraSvcCtx, OCIEnv *hOraEnv, OCIError *hOraErr, const char * strSelect, char * strResult, int sizeStrResult)
{
	sword status = 0;

	OCIStmt *hOraPlsqlUserToListStatement = NULL;
	if (OCIHandleAlloc((const void *)hOraEnv, (void **)&hOraPlsqlUserToListStatement, OCI_HTYPE_STMT, (size_t)0, (void **)0))
	{
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Application error, GetUserlist function - can not allocate hOraPlsqlUserToListStatement handle", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("Application error, GetStringsSetPrepare function - can not allocate hOraPlsqlStatement handle\n");
#endif
		return NULL;
	}
	

	if (OCIStmtPrepare(hOraPlsqlUserToListStatement, hOraErr, (const OraText *)strSelect, (ub4)strlen(strSelect), (ub4)OCI_NTV_SYNTAX, (ub4)OCI_DEFAULT) != OCI_SUCCESS)
	{
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Application error, GetUserlist function - unable to prepare plsql_user_to_list_statement", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("Application error, GetStringsSetPrepare function - unable to prepare sql statement\n");
#endif
		OCIHandleFree(hOraPlsqlUserToListStatement, OCI_HTYPE_STMT);
		return NULL;
	}

	OCIDefine *OraUsernameDefine = NULL;
	if (OCIDefineByPos(hOraPlsqlUserToListStatement, &OraUsernameDefine, hOraErr, 1, (void *)strResult, (sword)sizeStrResult, SQLT_STR, (void *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT) != OCI_SUCCESS)
	{
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Application error, GetUserlist function - unable to DefineByPos OraUsernameDefine", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("Application error, GetStringsSetPrepare function - unable to DefineByPos OraUsernameDefine\n");
#endif
		OCIHandleFree(hOraPlsqlUserToListStatement, OCI_HTYPE_STMT);
		return NULL;
	}

	status = OCIStmtExecute(hOraSvcCtx, hOraPlsqlUserToListStatement, hOraErr, (ub4)0, (ub4)0, (CONST OCISnapshot *) NULL, (OCISnapshot *)NULL, OCI_DEFAULT);
	if (status != OCI_SUCCESS && status != OCI_SUCCESS_WITH_INFO)
	{
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Application error, GetUserlist function - unable to execute plsql_user_to_list_statement", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("Application error, GetStringsSetPrepare function - unable to execute  sql statement\n");
#endif
		checkerr(hOraErr, status);
		OCIHandleFree(hOraPlsqlUserToListStatement, OCI_HTYPE_STMT);
		return NULL;
	}

	return hOraPlsqlUserToListStatement;

}

sword GetStringsSetFetch(OCIStmt *hOraPlsqlUserToListStatement, OCIError *hOraErr)
{
	sword status = 0;
	status = OCIStmtFetch2(hOraPlsqlUserToListStatement, hOraErr, 1, OCI_DEFAULT, 0, OCI_DEFAULT);

	return status;
}

void FreeSqlHandle(OCIStmt *hOraPlsqlUserToListStatement)
{
	OCIHandleFree(hOraPlsqlUserToListStatement, OCI_HTYPE_STMT);
}


bool GetNumberFromSelect(OCISvcCtx *hOraSvcCtx, OCIEnv *hOraEnv, OCIError *hOraErr, const char * strSelect, long long *Result)
{
	sword status = 0;

	OCIStmt *hOraPlsqlGetNumberStatement = NULL;
	if (OCIHandleAlloc((const void *)hOraEnv, (void **)&hOraPlsqlGetNumberStatement, OCI_HTYPE_STMT, (size_t)0, (void **)0))
	{
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Application error, GetUserlist function - can not allocate hOraPlsqlGetNumberStatement handle", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("Application error, GetStringsSetPrepare function - can not allocate hOraPlsqlGetNumberStatement handle\n");
#endif
		return FALSE;
	}
	

	if (OCIStmtPrepare(hOraPlsqlGetNumberStatement, hOraErr, (const OraText *)strSelect, (ub4)strlen(strSelect), (ub4)OCI_NTV_SYNTAX, (ub4)OCI_DEFAULT) != OCI_SUCCESS)
	{
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Application error, GetUserlist function - unable to prepare sql statement", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("Application error, GetStringsSetPrepare function - unable to prepare sql statement\n");
#endif
		OCIHandleFree(hOraPlsqlGetNumberStatement, OCI_HTYPE_STMT);
		return FALSE;
	}

	OCIDefine *OraDefine = NULL;
	if (OCIDefineByPos(hOraPlsqlGetNumberStatement, &OraDefine, hOraErr, 1, (void *)Result, (sword)sizeof(long long), SQLT_INT, (void *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT) != OCI_SUCCESS)
	{
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Application error, GetUserlist function - unable to DefineByPos", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("Application error, GetStringsSetPrepare function - unable to DefineByPos\n");
#endif
		OCIHandleFree(hOraPlsqlGetNumberStatement, OCI_HTYPE_STMT);
		return FALSE;
	}

	status = OCIStmtExecute(hOraSvcCtx, hOraPlsqlGetNumberStatement, hOraErr, (ub4)1, (ub4)0, (CONST OCISnapshot *) NULL, (OCISnapshot *)NULL, OCI_DEFAULT);
	if (status != OCI_SUCCESS && status != OCI_SUCCESS_WITH_INFO)
	{
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Application error, GetUserlist function - unable to execute sql statement", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("Application error, GetStringsSetPrepare function - unable to execute  sql statement\n");
#endif
		checkerr(hOraErr, status);
		OCIHandleFree(hOraPlsqlGetNumberStatement, OCI_HTYPE_STMT);
		return FALSE;
	}

	return TRUE;

}

//Функция возвращает USER_EXISTS либо USER_NOT_EXISTS
int IsUserExists(OCISvcCtx *hOraSvcCtx, OCIEnv *hOraEnv, OCIError *hOraErr, const char *schema)
{
	char schemautf8[32];
	strncpy(schemautf8, schema, 32);
	//Переводим в верхний регистр
	int i;
	for (i = 0; schemautf8[i] != 0; i++)
	{
		schemautf8[i] = toupper(schemautf8[i]);
	}

	sword user_exists = 0;
	sword status = 0;
	const char plsql_user_exists_statement[] = "select count(*) from dba_users where username like :USERNAME";

	OCIStmt *hOraPlsqlUserExistsStatement = NULL;
	if (OCIHandleAlloc((const void *)hOraEnv, (void **)&hOraPlsqlUserExistsStatement, OCI_HTYPE_STMT, (size_t)0, (void **)0))
	{
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Application error, IsUserExists function - can not allocate hOraPlsqlUserExistsStatement handle", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("Application error, IsUserExists function - can not allocate hOraPlsqlUserExistsStatement handle\n");
#endif
		return FALSE;
	}
	

	if (OCIStmtPrepare(hOraPlsqlUserExistsStatement, hOraErr, (const OraText *)plsql_user_exists_statement, (ub4)strlen(plsql_user_exists_statement), (ub4)OCI_NTV_SYNTAX, (ub4)OCI_DEFAULT) != OCI_SUCCESS)
	{
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Application error, IsUserExists function - unable to prepare statement plsql_user_exists_statement", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("Application error, IsUserExists function - unable to prepare statement plsql_user_exists_statement.\n");
#endif
		OCIHandleFree(hOraPlsqlUserExistsStatement, OCI_HTYPE_STMT);
		return FALSE;
	}

	OCIBind  *bnd1p = NULL;
	if (OCIBindByName(hOraPlsqlUserExistsStatement, &bnd1p, hOraErr, (text *) ":USERNAME", -1, (void *)schemautf8, (sb4)(strlen(schemautf8) + 1), SQLT_STR, (void *)0, (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT))
	{
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Application error, IsUserExists function - unable to bind USERNAME", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("Application error, IsUserExists function - unable to bind USERNAME\n");
#endif
		OCIHandleFree(hOraPlsqlUserExistsStatement, OCI_HTYPE_STMT);
		return FALSE;
	}


	OCIDefine *OraUserCountDefine = NULL;
	if (OCIDefineByPos(hOraPlsqlUserExistsStatement, &OraUserCountDefine, hOraErr, 1, (void *)&user_exists, (sword)sizeof(user_exists), SQLT_INT, (void *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT) != OCI_SUCCESS)
	{
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Application error, IsUserExists function - unable to DefineByPos OraUserCountDefine", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("Application error, IsUserExists function - unable to DefineByPos OraUserCountDefine.\n");
#endif
		OCIHandleFree(hOraPlsqlUserExistsStatement, OCI_HTYPE_STMT);
		return FALSE;
	}


	status = OCIStmtExecute(hOraSvcCtx, hOraPlsqlUserExistsStatement, hOraErr, (ub4)1, (ub4)0, (CONST OCISnapshot *) NULL, (OCISnapshot *)NULL, OCI_DEFAULT);
	if (status != OCI_SUCCESS && status != OCI_SUCCESS_WITH_INFO)
	{
#ifdef MESSAGEBOX_ERROR_OUTPUT
		MessageBox(MainWindowHandle, L"Application error, IsUserExists function - unable to execute hOraPlsqlUserExistsStatement", L"Error", MB_OK | MB_ICONERROR);
#endif
#ifdef CONSOLE_ERROR_OUTPUT
		printf("Application error, IsUserExists function - unable to execute hOraPlsqlUserExistsStatement.\n");
#endif
		checkerr(hOraErr, status);
		OCIHandleFree(hOraPlsqlUserExistsStatement, OCI_HTYPE_STMT);
		return FALSE;
	}

	if (!user_exists)
	{
		OCIHandleFree(hOraPlsqlUserExistsStatement, OCI_HTYPE_STMT);
		return USER_NOT_EXISTS;
	}
	else
	{
		OCIHandleFree(hOraPlsqlUserExistsStatement, OCI_HTYPE_STMT);
		return USER_EXISTS;
	}
}

//Эта версия функции отличается от подобной в datapumpexp тем, что здесь не запускается в отдельном потоке функция отслеживания прогресса экспорта.
//мы просто запускаем экспорт и ждем его завершения
int ExecuteExportJob(OCISvcCtx *hOraSvcCtx, OCIEnv *hOraEnv, OCIError *hOraErr, const char *schemautf8, const char *oradirutf8, const char *dumpfilenameutf8, const char *logfilenameutf8)
{

	char schema_expression[50];
	sprintf(schema_expression, "IN ('%s')", schemautf8);
	char jobname[30];
	sword job_name_counter = 1;
	sword countFromJobs = 1;
	sword status = 0;

	char *getJobName = (char *)malloc(128);
	OCIStmt *hGetCountStmt = NULL;
	for (job_name_counter = 1; countFromJobs == 1; job_name_counter++)
	{
		snprintf(jobname, 30, "OML_EXP_%s_%d", schemautf8, job_name_counter);
		sprintf(getJobName, "SELECT count(*) FROM dba_datapump_jobs where job_name LIKE '%s'", jobname);

		hGetCountStmt = NULL;
		if (OCIHandleAlloc((const void *)hOraEnv, (void **)&hGetCountStmt, OCI_HTYPE_STMT, (size_t)0, (void **)0))
		{
			printf("Application error, ExecuteExportJob function - can not allocate hGetCountStmt handle\n");
			free(getJobName);
			return FALSE;
		}

		if (OCIStmtPrepare(hGetCountStmt, hOraErr, (const OraText *)getJobName, (ub4)strlen(getJobName), (ub4)OCI_NTV_SYNTAX, (ub4)OCI_DEFAULT) != OCI_SUCCESS)
		{
			printf("Application error, ExecuteExportJob function - unable to prepare getJobName statement.\n");
			OCIHandleFree(hGetCountStmt, OCI_HTYPE_STMT);
			free(getJobName);
			return FALSE;
		}

		OCIDefine *OraCountDefine = NULL;
		if (OCIDefineByPos(hGetCountStmt, &OraCountDefine, hOraErr, 1, (void *)&countFromJobs, (sword)sizeof(countFromJobs), SQLT_INT, (void *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT) != OCI_SUCCESS)
		{
			printf("Application error, ExecuteExportJob function - unable to DefineByPos OraCountDefine.\n");
			OCIHandleFree(hGetCountStmt, OCI_HTYPE_STMT);
			free(getJobName);
			return FALSE;
		}
		status = OCIStmtExecute(hOraSvcCtx, hGetCountStmt, hOraErr, (ub4)1, (ub4)0, (CONST OCISnapshot *) NULL, (OCISnapshot *)NULL, OCI_DEFAULT);
		if (status != OCI_SUCCESS && status != OCI_SUCCESS_WITH_INFO)
		{
			printf("Application error, ExecuteExportJob function - unable to execute \"%s\"\n", getJobName);
			checkerr(hOraErr, status);
			OCIHandleFree(hGetCountStmt, OCI_HTYPE_STMT);
			free(getJobName);
			return FALSE;
		}

		
		OCIHandleFree(hGetCountStmt, OCI_HTYPE_STMT);
	}
	

	const char plsql_export_schema_statement[] = "DECLARE\
                    d1 NUMBER;\
                    job_state VARCHAR2(100);\
                    BEGIN\
                    d1 := DBMS_DATAPUMP.OPEN('EXPORT', 'SCHEMA', NULL, :job_name, 'COMPATIBLE');\
                    DBMS_DATAPUMP.ADD_FILE(d1, :dumpfile, :oracle_dir, NULL, DBMS_DATAPUMP.KU$_FILE_TYPE_DUMP_FILE, 0);\
                    DBMS_DATAPUMP.ADD_FILE(d1, :logfile, :oracle_dir, NULL, DBMS_DATAPUMP.KU$_FILE_TYPE_LOG_FILE, 1);\
                    DBMS_DATAPUMP.METADATA_FILTER(d1, 'SCHEMA_EXPR', :schema);\
                    DBMS_DATAPUMP.START_JOB(d1);\
                    DBMS_DATAPUMP.WAIT_FOR_JOB(d1, job_state);\
                    DBMS_DATAPUMP.DETACH(d1);\
                    END;";

	OCIStmt *hOraPlsqlExpSchemaStatement = NULL;
	if (OCIHandleAlloc((const void *)hOraEnv, (void **)&hOraPlsqlExpSchemaStatement, OCI_HTYPE_STMT, (size_t)0, (void **)0))
	{
		printf("Application error, ExecuteExportJob function - can not allocate hOraPlsqlExpSchemaStatement handle\n");
		free(getJobName);
		return FALSE;
	}
	

	if (OCIStmtPrepare(hOraPlsqlExpSchemaStatement, hOraErr, (const OraText *)plsql_export_schema_statement, (ub4)strlen(plsql_export_schema_statement), (ub4)OCI_NTV_SYNTAX, (ub4)OCI_DEFAULT) != OCI_SUCCESS)
	{
		printf("Application error, ExecuteExportJob function - unable to prepare plsql_export_schema_statement statement.\n");
		OCIHandleFree(hOraPlsqlExpSchemaStatement, OCI_HTYPE_STMT);
		free(getJobName);
		return FALSE;
	}

	OCIBind  *bnd1p = NULL;
	OCIBind  *bnd2p = NULL;
	OCIBind  *bnd3p = NULL;
	OCIBind  *bnd4p = NULL;
	OCIBind  *bnd5p = NULL;
	OCIBindByName(hOraPlsqlExpSchemaStatement, &bnd1p, hOraErr, (text *)":job_name", -1, (void *)jobname, (sb4)(strlen(jobname) + 1), SQLT_STR, (void *)0, (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
	OCIBindByName(hOraPlsqlExpSchemaStatement, &bnd2p, hOraErr, (text *)":dumpfile", -1, (void *)dumpfilenameutf8, (sb4)(strlen(dumpfilenameutf8) + 1), SQLT_STR, (void *)0, (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
	OCIBindByName(hOraPlsqlExpSchemaStatement, &bnd3p, hOraErr, (text *)":logfile", -1, (void *)logfilenameutf8, (sb4)(strlen(logfilenameutf8) + 1), SQLT_STR, (void *)0, (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
	OCIBindByName(hOraPlsqlExpSchemaStatement, &bnd4p, hOraErr, (text *)":oracle_dir", -1, (void *)oradirutf8, (sb4)(strlen(oradirutf8) + 1), SQLT_STR, (void *)0, (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
	OCIBindByName(hOraPlsqlExpSchemaStatement, &bnd5p, hOraErr, (text *)":schema", -1, (void *)schema_expression, (sb4)(strlen(schema_expression) + 1), SQLT_STR, (void *)0, (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);

	
	status = OCIStmtExecute(hOraSvcCtx, hOraPlsqlExpSchemaStatement, hOraErr, (ub4)1, (ub4)0, (CONST OCISnapshot *) NULL, (OCISnapshot *)NULL, OCI_DEFAULT);
	if (status != OCI_SUCCESS && status != OCI_SUCCESS_WITH_INFO)
	{
		printf("Application error, ExecuteExportJob function - unable to execute datapump export statement, %s\n", schema_expression);
		checkerr(hOraErr, status);
		OCIHandleFree(hOraPlsqlExpSchemaStatement, OCI_HTYPE_STMT);
		free(getJobName);
		return FALSE;
	}
	
	OCIHandleFree(hOraPlsqlExpSchemaStatement, OCI_HTYPE_STMT);
	free(getJobName);

	return TRUE;
}


//Эта версия функции ExecuteExportJob, в которой мы отслеживаем сообщения об ошибках.
//Отдельный поток для отслеживания прогресса на запускается, мы просто запускаем экспорт и ждем его завершения
int ExecuteExportJobErrorTracking(OCISvcCtx *hOraSvcCtx, OCIEnv *hOraEnv, OCIError *hOraErr, const char *schemautf8, const char *oradirutf8, const char *dumpfilenameutf8, const char *logfilenameutf8, int *errorCount, bool consistent)
{

	char schema_expression[50];
	sprintf(schema_expression, "IN ('%s')", schemautf8);
	char jobname[30];
	sword job_name_counter = 1;
	sword countFromJobs = 1;
	sword status = 0;
	int exportErrorCount = 0;

	char *getJobName = (char *)malloc(128);
	OCIStmt *hGetCountStmt = NULL;
	for (job_name_counter = 1; countFromJobs == 1; job_name_counter++)
	{
		snprintf(jobname, 30, "OML_EXP_%s_%d", schemautf8, job_name_counter);
		sprintf(getJobName, "SELECT count(*) FROM dba_datapump_jobs where job_name LIKE '%s'", jobname);

		hGetCountStmt = NULL;
		if (OCIHandleAlloc((const void *)hOraEnv, (void **)&hGetCountStmt, OCI_HTYPE_STMT, (size_t)0, (void **)0))
		{
			printf("Application error, ExecuteExportJobErrorTracking function - can not allocate hGetCountStmt handle\n");
			free(getJobName);
			return FALSE;
		}

		if (OCIStmtPrepare(hGetCountStmt, hOraErr, (const OraText *)getJobName, (ub4)strlen(getJobName), (ub4)OCI_NTV_SYNTAX, (ub4)OCI_DEFAULT) != OCI_SUCCESS)
		{
			printf("Application error, ExecuteExportJobErrorTracking function - unable to prepare getJobName statement.\n");
			OCIHandleFree(hGetCountStmt, OCI_HTYPE_STMT);
			free(getJobName);
			return FALSE;
		}

		OCIDefine *OraCountDefine = NULL;
		if (OCIDefineByPos(hGetCountStmt, &OraCountDefine, hOraErr, 1, (void *)&countFromJobs, (sword)sizeof(countFromJobs), SQLT_INT, (void *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT) != OCI_SUCCESS)
		{
			printf("Application error, ExecuteExportJobErrorTracking function - unable to DefineByPos OraCountDefine.\n");
			OCIHandleFree(hGetCountStmt, OCI_HTYPE_STMT);
			free(getJobName);
			return FALSE;
		}
		status = OCIStmtExecute(hOraSvcCtx, hGetCountStmt, hOraErr, (ub4)1, (ub4)0, (CONST OCISnapshot *) NULL, (OCISnapshot *)NULL, OCI_DEFAULT);
		if (status != OCI_SUCCESS && status != OCI_SUCCESS_WITH_INFO)
		{
			printf("Application error, ExecuteExportJobErrorTracking function - unable to execute \"%s\"\n", getJobName);
			checkerr(hOraErr, status);
			OCIHandleFree(hGetCountStmt, OCI_HTYPE_STMT);
			free(getJobName);
			return FALSE;
		}


		OCIHandleFree(hGetCountStmt, OCI_HTYPE_STMT);
	}

	const char *plsql_export_schema_statement = NULL;

	if (consistent)
	{
		plsql_export_schema_statement = "DECLARE\
                    d1 NUMBER;\
                      errors_count NUMBER;\
                      ind NUMBER;\
                      job_state VARCHAR2(100);\
                      errle ku$_LogEntry;\
                      sts ku$_Status;\
                    BEGIN\
                      d1 := DBMS_DATAPUMP.OPEN('EXPORT', 'SCHEMA', NULL, :job_name, 'COMPATIBLE');\
                      DBMS_DATAPUMP.ADD_FILE(d1, :dumpfile, :oracle_dir, NULL, DBMS_DATAPUMP.KU$_FILE_TYPE_DUMP_FILE, 0);\
                      DBMS_DATAPUMP.ADD_FILE(d1, :logfile, :oracle_dir, NULL, DBMS_DATAPUMP.KU$_FILE_TYPE_LOG_FILE, 1);\
                      DBMS_DATAPUMP.METADATA_FILTER(d1, 'SCHEMA_EXPR', :schema);\
                      DBMS_DATAPUMP.SET_PARAMETER(d1, 'FLASHBACK_TIME', 'TO_TIMESTAMP( TO_CHAR( SYSDATE) )');\
                      DBMS_DATAPUMP.START_JOB(d1);\
                      errors_count := 0;\
		              job_state := 'UNDEFINED';\
	                  while (job_state != 'COMPLETED') and (job_state != 'STOPPED') loop\
		                dbms_datapump.get_status(d1, dbms_datapump.ku$_status_job_error, -1, job_state, sts);\
	                    if (bitand(sts.mask, dbms_datapump.ku$_status_job_error) != 0)\
		                  then\
		                    errle := sts.error;\
	                      else\
		                    errle := null;\
                        end if;\
                        if errle is not null\
                          then\
                            ind := errle.FIRST;\
                            while ind is not null loop\
                              errors_count := errors_count + 1;\
                              ind := errle.NEXT(ind);\
                            end loop;\
                        end if;\
                      end loop;\
                      :errors_count := errors_count;\
                      DBMS_DATAPUMP.DETACH(d1);\
                    END;";
	}
	else
	{
		plsql_export_schema_statement = "DECLARE\
                    d1 NUMBER;\
                      errors_count NUMBER;\
                      ind NUMBER;\
                      job_state VARCHAR2(100);\
                      errle ku$_LogEntry;\
                      sts ku$_Status;\
                    BEGIN\
                      d1 := DBMS_DATAPUMP.OPEN('EXPORT', 'SCHEMA', NULL, :job_name, 'COMPATIBLE');\
                      DBMS_DATAPUMP.ADD_FILE(d1, :dumpfile, :oracle_dir, NULL, DBMS_DATAPUMP.KU$_FILE_TYPE_DUMP_FILE, 0);\
                      DBMS_DATAPUMP.ADD_FILE(d1, :logfile, :oracle_dir, NULL, DBMS_DATAPUMP.KU$_FILE_TYPE_LOG_FILE, 1);\
                      DBMS_DATAPUMP.METADATA_FILTER(d1, 'SCHEMA_EXPR', :schema);\
                      DBMS_DATAPUMP.START_JOB(d1);\
                      errors_count := 0;\
		              job_state := 'UNDEFINED';\
	                  while (job_state != 'COMPLETED') and (job_state != 'STOPPED') loop\
		                dbms_datapump.get_status(d1, dbms_datapump.ku$_status_job_error, -1, job_state, sts);\
	                    if (bitand(sts.mask, dbms_datapump.ku$_status_job_error) != 0)\
		                  then\
		                    errle := sts.error;\
	                      else\
		                    errle := null;\
                        end if;\
                        if errle is not null\
                          then\
                            ind := errle.FIRST;\
                            while ind is not null loop\
                              errors_count := errors_count + 1;\
                              ind := errle.NEXT(ind);\
                            end loop;\
                        end if;\
                      end loop;\
                      :errors_count := errors_count;\
                      DBMS_DATAPUMP.DETACH(d1);\
                    END;";
	}



	OCIStmt *hOraPlsqlExpSchemaStatement = NULL;
	if (OCIHandleAlloc((const void *)hOraEnv, (void **)&hOraPlsqlExpSchemaStatement, OCI_HTYPE_STMT, (size_t)0, (void **)0))
	{
		printf("Application error, ExecuteExportJobErrorTracking function - can not allocate hOraPlsqlExpSchemaStatement handle\n");
		free(getJobName);
		return FALSE;
	}


	if (OCIStmtPrepare(hOraPlsqlExpSchemaStatement, hOraErr, (const OraText *)plsql_export_schema_statement, (ub4)strlen(plsql_export_schema_statement), (ub4)OCI_NTV_SYNTAX, (ub4)OCI_DEFAULT) != OCI_SUCCESS)
	{
		printf("Application error, ExecuteExportJobErrorTracking function - unable to prepare plsql_export_schema_statement statement.\n");
		OCIHandleFree(hOraPlsqlExpSchemaStatement, OCI_HTYPE_STMT);
		free(getJobName);
		return FALSE;
	}

	OCIBind  *bnd1p = NULL;
	OCIBind  *bnd2p = NULL;
	OCIBind  *bnd3p = NULL;
	OCIBind  *bnd4p = NULL;
	OCIBind  *bnd5p = NULL;
	OCIBind  *bnd6p = NULL;
	OCIBindByName(hOraPlsqlExpSchemaStatement, &bnd1p, hOraErr, (text *)":job_name", -1, (void *)jobname, (sb4)(strlen(jobname) + 1), SQLT_STR, (void *)0, (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
	OCIBindByName(hOraPlsqlExpSchemaStatement, &bnd2p, hOraErr, (text *)":dumpfile", -1, (void *)dumpfilenameutf8, (sb4)(strlen(dumpfilenameutf8) + 1), SQLT_STR, (void *)0, (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
	OCIBindByName(hOraPlsqlExpSchemaStatement, &bnd3p, hOraErr, (text *)":logfile", -1, (void *)logfilenameutf8, (sb4)(strlen(logfilenameutf8) + 1), SQLT_STR, (void *)0, (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
	OCIBindByName(hOraPlsqlExpSchemaStatement, &bnd4p, hOraErr, (text *)":oracle_dir", -1, (void *)oradirutf8, (sb4)(strlen(oradirutf8) + 1), SQLT_STR, (void *)0, (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
	OCIBindByName(hOraPlsqlExpSchemaStatement, &bnd5p, hOraErr, (text *)":schema", -1, (void *)schema_expression, (sb4)(strlen(schema_expression) + 1), SQLT_STR, (void *)0, (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
	OCIBindByName(hOraPlsqlExpSchemaStatement, &bnd6p, hOraErr, (text *)":errors_count", -1, (void *)errorCount, (sb4)sizeof(int), SQLT_INT, (void *)0, (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);


	status = OCIStmtExecute(hOraSvcCtx, hOraPlsqlExpSchemaStatement, hOraErr, (ub4)1, (ub4)0, (CONST OCISnapshot *) NULL, (OCISnapshot *)NULL, OCI_DEFAULT);
	if (status != OCI_SUCCESS && status != OCI_SUCCESS_WITH_INFO)
	{
		printf("Application error, ExecuteExportJobErrorTracking function - unable to execute datapump export statement, %s\n", schema_expression);
		checkerr(hOraErr, status);
		OCIHandleFree(hOraPlsqlExpSchemaStatement, OCI_HTYPE_STMT);
		free(getJobName);
		return FALSE;
	}


	OCIHandleFree(hOraPlsqlExpSchemaStatement, OCI_HTYPE_STMT);
	free(getJobName);

	return TRUE;
}


int IsDBLocal(OCISvcCtx *hOraSvcCtx, OCIEnv *hOraEnv, OCIError *hOraErr)
{
	DWORD ClientStringSize = 64;
	DWORD ServerStringSize = 64;
	WCHAR *ClientName = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, ClientStringSize);
	GetComputerNameExW(ComputerNamePhysicalDnsHostname, ClientName, &ClientStringSize);
	char *ClientNameUTF8 = (char *)HeapAlloc(GetProcessHeap(), 0, ClientStringSize + 1);
	WideCharToMultiByte(CP_UTF8, 0, ClientName, -1, ClientNameUTF8, ClientStringSize + 1, NULL, NULL);
	_strupr(ClientNameUTF8);
	char *ServerNameUTF8 = (char *)HeapAlloc(GetProcessHeap(), 0, ServerStringSize);

	sword status = 0;
	const char plsql_get_hostname_statement[] = "select host_name from v$instance";
	OCIStmt *hGetHostnameStmt = NULL;
	if (OCIHandleAlloc((const void *)hOraEnv, (void **)&hGetHostnameStmt, OCI_HTYPE_STMT, (size_t)0, (void **)0))
	{
		printf("Application error, IsDBLocal function - can not allocate hGetHostnameStmt handle\n");
		HeapFree(GetProcessHeap(), 0, ClientName);
		HeapFree(GetProcessHeap(), 0, ClientNameUTF8);
		HeapFree(GetProcessHeap(), 0, ServerNameUTF8);
		return FALSE;
	}

	if (OCIStmtPrepare(hGetHostnameStmt, hOraErr, (const OraText *)plsql_get_hostname_statement, (ub4)strlen(plsql_get_hostname_statement), (ub4)OCI_NTV_SYNTAX, (ub4)OCI_DEFAULT) != OCI_SUCCESS)
	{
		printf("Application error, IsDBLocal function - unable to prepare plsql_get_hostname_statement\n");
		OCIHandleFree(hGetHostnameStmt, OCI_HTYPE_STMT);
		HeapFree(GetProcessHeap(), 0, ClientName);
		HeapFree(GetProcessHeap(), 0, ClientNameUTF8);
		HeapFree(GetProcessHeap(), 0, ServerNameUTF8);
		return FALSE;
	}

	OCIDefine *OraHostnameDefine = NULL;
	if (OCIDefineByPos(hGetHostnameStmt, &OraHostnameDefine, hOraErr, 1, (void *)ServerNameUTF8, (sword)ServerStringSize, SQLT_STR, (void *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT) != OCI_SUCCESS)
	{
		printf("Application error, IsDBLocal function - unable to define OraHostnameDefine\n");
		OCIHandleFree(hGetHostnameStmt, OCI_HTYPE_STMT);
		HeapFree(GetProcessHeap(), 0, ClientName);
		HeapFree(GetProcessHeap(), 0, ClientNameUTF8);
		HeapFree(GetProcessHeap(), 0, ServerNameUTF8);
		return FALSE;
	}
	status = OCIStmtExecute(hOraSvcCtx, hGetHostnameStmt, hOraErr, (ub4)1, (ub4)0, (CONST OCISnapshot *) NULL, (OCISnapshot *)NULL, OCI_DEFAULT);
	if (status != OCI_SUCCESS && status != OCI_SUCCESS_WITH_INFO)
	{
		printf("Application error, IsDBLocal function - unable to execute plsql_get_hostname_statement\n");
		checkerr(hOraErr, status);
		OCIHandleFree(hGetHostnameStmt, OCI_HTYPE_STMT);
		HeapFree(GetProcessHeap(), 0, ClientName);
		HeapFree(GetProcessHeap(), 0, ClientNameUTF8);
		HeapFree(GetProcessHeap(), 0, ServerNameUTF8);
		return FALSE;
	}
	_strupr(ServerNameUTF8);

	if (strcmp(ClientNameUTF8, ServerNameUTF8) == 0)
	{
		OCIHandleFree(hGetHostnameStmt, OCI_HTYPE_STMT);
		HeapFree(GetProcessHeap(), 0, ClientName);
		HeapFree(GetProcessHeap(), 0, ClientNameUTF8);
		HeapFree(GetProcessHeap(), 0, ServerNameUTF8);
		return LOCALDB;
	}
	else
	{
		OCIHandleFree(hGetHostnameStmt, OCI_HTYPE_STMT);
		HeapFree(GetProcessHeap(), 0, ClientName);
		HeapFree(GetProcessHeap(), 0, ClientNameUTF8);
		HeapFree(GetProcessHeap(), 0, ServerNameUTF8);
		return REMOTEDB;
	}

}

//Функция модифицирована ддя сжатия на лету, возвращает просто текущее время
uLong filetime(tm_zip *tmzip, uLong *dt)
{
	FILETIME ft, ftLocal;
	SYSTEMTIME st;

	GetSystemTime(&st);
	SystemTimeToFileTime(&st, &ft);
	FileTimeToLocalFileTime(&ft, &ftLocal);
	FileTimeToDosDateTime(&ftLocal, ((LPWORD)dt) + 1, ((LPWORD)dt) + 0);

	return TRUE;
}

int check_exist_file(const char* filenameutf8)
{
	FILE* ftestexist;
	int ret = 1;
	ftestexist = FOPEN_FUNC(filenameutf8, "rb");
	if (ftestexist == NULL)
		ret = 0;
	else
		fclose(ftestexist);
	return ret;
}

//Функция выполняем получение файла с удаленного сервера оракл со сжатием на лету, первый параметр - полное имя зипфайла, второй - имя Оракл директории, третий - имя файла внутри Оракл
//Отличается от аналогичной функции в datapumpexp тем, что тут нет отображения прогресса скачки файла.
bool GetFileFromDatabaseToZip(OCISvcCtx *hOraSvcCtx, OCIEnv *hOraEnv, OCIError *hOraErr, char *fullzipfilenameutf8, char *oradirutf8, char *filenameutf8)
{
	
	OCILobLocator *LobLocator;
	LobLocator = NULL;
	sword status;

	status = OCIDescriptorAlloc(hOraEnv, (void **)&LobLocator, OCI_DTYPE_FILE, 0, 0);
	if (status != OCI_SUCCESS && status != OCI_SUCCESS_WITH_INFO)
	{
		printf("Application error, GetFileFromDatabaseToZip function - can not allocate LOB file descriptor\n");
		checkerr(hOraErr, status);
		return FALSE;
	}
	
	status = OCILobFileSetName(hOraEnv, hOraErr, &LobLocator, (const text *)oradirutf8, (ub2)strlen(oradirutf8), (const text *)filenameutf8, (ub2)strlen(filenameutf8));
	if (status != OCI_SUCCESS && status != OCI_SUCCESS_WITH_INFO)
	{
		printf("Application error, GetFileFromDatabaseToZip function - can not set filename to LOB file descriptor\n");
		checkerr(hOraErr, status);
		return FALSE;
	}

	status = OCILobFileOpen(hOraSvcCtx, hOraErr, LobLocator, OCI_FILE_READONLY);
	if (status != OCI_SUCCESS && status != OCI_SUCCESS_WITH_INFO)
	{
		printf("Application error, GetFileFromDatabaseToZip function - can not open lob file\n");
		checkerr(hOraErr, status);
		OCIDescriptorFree(LobLocator, OCI_DTYPE_FILE);
		return FALSE;
	}

	boolean isLobFileExists;
	OCILobFileExists(hOraSvcCtx, hOraErr, LobLocator, &isLobFileExists);
	if (!isLobFileExists)
	{
		OCILobFileClose(hOraSvcCtx, hOraErr, LobLocator);
		OCIDescriptorFree(LobLocator, OCI_DTYPE_FILE);
		printf("Application error, GetFileFromDatabaseToZip function - LobFile doesn't exists\n");
		return FALSE;
	}

	oraub8 bfilelenght = 0;
	OCILobGetLength2(hOraSvcCtx, hOraErr, LobLocator, &bfilelenght);

	//Структура для данных о зипфайле
	zipFile zf;
	//Вариант перезаписи
	int opt_overwrite = 0;
	//переменная для хранения ошибок от библиотеки zip
	int zip_err;
	//Структура для данных о записи в зипе
	zip_fileinfo zi;
	unsigned long crcFile = 0;
	int zip64 = 0;
	int opt_compress_level = Z_DEFAULT_COMPRESSION;
	const char* password = NULL;

	/* if the file don't exist, we not append file */
	if (check_exist_file(fullzipfilenameutf8) == 0)
		opt_overwrite = APPEND_STATUS_CREATE;
	else
		opt_overwrite = APPEND_STATUS_ADDINZIP;

	zf = zipOpen64(fullzipfilenameutf8, opt_overwrite);
	if (zf == NULL)
	{
		printf("Application error, GetFileFromDatabaseToZip function - error opening %s\n", fullzipfilenameutf8);
		OCILobFileClose(hOraSvcCtx, hOraErr, LobLocator);
		OCIDescriptorFree(LobLocator, OCI_DTYPE_FILE);
		return FALSE;
	}


	zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
		zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
	zi.dosDate = 0;
	zi.internal_fa = 0;
	zi.external_fa = 0;
	filetime(&zi.tmz_date, &zi.dosDate);

	if (bfilelenght >= 0xffffffff)
	{
		zip64 = 1;
	}

	zip_err = zipOpenNewFileInZip4_64(zf, filenameutf8, &zi, NULL, 0, NULL, 0, NULL,
		(opt_compress_level != 0) ? Z_DEFLATED : 0, opt_compress_level, 0,
		-MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
		password, crcFile, 0, 1 << 11, zip64);
	if (zip_err != ZIP_OK)
	{
		printf("Application error, GetFileFromDatabaseToZip function - error in opening %s in zipfile\n", filenameutf8);
		OCILobFileClose(hOraSvcCtx, hOraErr, LobLocator);
		OCIDescriptorFree(LobLocator, OCI_DTYPE_FILE);
		zipClose(zf, NULL);
		return FALSE;
	}

	oraub8 transeredData = 0;
	oraub8 buflen = 1048576;
	char *buf = (char *)HeapAlloc(GetProcessHeap(), 0, 1048576);
	oraub8 offset = 1;
	
	do
	{
		status = OCILobRead2(hOraSvcCtx, hOraErr, LobLocator, &buflen, 0, offset, (void*)buf, buflen, OCI_ONE_PIECE, NULL, NULL, 0, SQLCS_IMPLICIT);
		if (status == OCI_ERROR)
		{
			break;	
		}

		checkerr(hOraErr, status);

		zip_err = zipWriteInFileInZip(zf, buf, (unsigned int)buflen);
		if (zip_err < 0)
		{
			printf("Application error, GetFileFromDatabaseToZip function - error in writing in the zipfile\n");
			OCILobFileClose(hOraSvcCtx, hOraErr, LobLocator);
			OCIDescriptorFree(LobLocator, OCI_DTYPE_FILE);
			HeapFree(GetProcessHeap(), 0, buf);
			zipCloseFileInZip(zf);
			zipClose(zf, NULL);
			return FALSE;
		}

		
		offset = offset + buflen;
		transeredData = transeredData + buflen;
		//printf("\r");
		//printf("%s transfer progress - %llu%%(%llu/%llu)", filenameutf8, transeredData * 100 / bfilelenght, (unsigned __int64)transeredData, (unsigned __int64)bfilelenght);
	} while ((status == OCI_NEED_DATA || status == OCI_SUCCESS) && (zip_err == ZIP_OK));
	//printf("\r");
	//printf("%s transfer progress - %llu%%(%llu/%llu)\n", filenameutf8, transeredData * 100 / bfilelenght, (unsigned __int64)transeredData, (unsigned __int64)bfilelenght);


	if (bfilelenght != transeredData)
	{
		printf("Application error, GetFileFromDatabaseToZip function - transfered data differ from bfile size.\n");
		OCILobFileClose(hOraSvcCtx, hOraErr, LobLocator);
		OCIDescriptorFree(LobLocator, OCI_DTYPE_FILE);
		HeapFree(GetProcessHeap(), 0, buf);
		zipCloseFileInZip(zf);
		zipClose(zf, NULL);
		return FALSE;
	}

	OCILobFileClose(hOraSvcCtx, hOraErr, LobLocator);
	OCIDescriptorFree(LobLocator, OCI_DTYPE_FILE);
	HeapFree(GetProcessHeap(), 0, buf);
	
	zip_err = zipCloseFileInZip(zf);
	if (zip_err != ZIP_OK)
	{
		printf("Application error, GetFileFromDatabaseToZip function - error in closing entry in the zipfile\n");
		zipClose(zf, NULL);
		return FALSE;
	}

	zip_err = zipClose(zf, NULL);
	if (zip_err != ZIP_OK)
	{
		printf("Application error, GetFileFromDatabaseToZip function - error in closing zipfile\n");
		return FALSE;
	}

	return TRUE;
}


/*Функция выполняем получение файла с локального сервера оракл в зип-фархив, первый параметр - полное имя зипфайла, второй - имя Оракл директории, третий - имя файла внутри Оракл*/
bool LocalGetFileFromDatabaseToZip(OCISvcCtx *hOraSvcCtx, OCIEnv *hOraEnv, OCIError *hOraErr, char *fullzipfilenameutf8, char *oradirutf8, char *filenameutf8)
{
	//Полный локальный путь к файлу в кодировке UTF8
	char *orafileutf8 = (char *)HeapAlloc(GetProcessHeap(), 0, 256);
	//Полный локальный путь к файлу в кодировке UTF16 (UCS2)
	WCHAR *orafile = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, 512);

	sword status;

	const char get_directory_path[] = "select directory_path from all_directories where directory_name LIKE :oradir";

	OCIStmt *hOraGetDirectoryPathStatement = NULL;
	if (OCIHandleAlloc((const void *)hOraEnv, (void **)&hOraGetDirectoryPathStatement, OCI_HTYPE_STMT, (size_t)0, (void **)0))
	{
		HeapFree(GetProcessHeap(), 0, orafileutf8);
		HeapFree(GetProcessHeap(), 0, orafile);
		printf("Application error, LocalGetFileFromDatabaseToZip function - unable to allocate PLSQL Statement handle\n");
		return FALSE;
	}
	

	if (OCIStmtPrepare(hOraGetDirectoryPathStatement, hOraErr, (const OraText *)get_directory_path, (ub4)strlen(get_directory_path), (ub4)OCI_NTV_SYNTAX, (ub4)OCI_DEFAULT) != OCI_SUCCESS)
	{
		HeapFree(GetProcessHeap(), 0, orafileutf8);
		HeapFree(GetProcessHeap(), 0, orafile);
		OCIHandleFree(hOraGetDirectoryPathStatement, OCI_HTYPE_STMT);
		printf("Application error, LocalGetFileFromDatabaseToZip function - unable to prepare statement plsql_statement.\n");
		return FALSE;
	}

	OCIBind  *bnd3p = NULL;
	OCIBindByName(hOraGetDirectoryPathStatement, &bnd3p, hOraErr, (text *)":oradir", -1, (void *)oradirutf8, 32, SQLT_STR, (void *)0, (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);

	OCIDefine *OraDirPath = NULL;
	if (OCIDefineByPos(hOraGetDirectoryPathStatement, &OraDirPath, hOraErr, 1, (void *)orafileutf8, (sword)256, SQLT_STR, (void *)0, (ub2 *)0, (ub2 *)0, OCI_DEFAULT) != OCI_SUCCESS)
	{
		printf("Application error, LocalGetFileFromDatabaseToZip function - unable to DefineByPos.\n");
		HeapFree(GetProcessHeap(), 0, orafileutf8);
		HeapFree(GetProcessHeap(), 0, orafile);
		OCIHandleFree(hOraGetDirectoryPathStatement, OCI_HTYPE_STMT);
		return FALSE;
	}

	status = OCIStmtExecute(hOraSvcCtx, hOraGetDirectoryPathStatement, hOraErr, (ub4)1, (ub4)0, (CONST OCISnapshot *) NULL, (OCISnapshot *)NULL, OCI_DEFAULT);
	if (status != OCI_SUCCESS && status != OCI_SUCCESS_WITH_INFO)
	{
#ifdef DEBUG_LOGGING
		printf("Application error, LocalGetFileFromDatabaseToZip function - unable to execute get_directory_path\n");
#endif
		checkerr(hOraErr, status);
		HeapFree(GetProcessHeap(), 0, orafileutf8);
		HeapFree(GetProcessHeap(), 0, orafile);
		OCIHandleFree(hOraGetDirectoryPathStatement, OCI_HTYPE_STMT);
		printf("Application error, LocalGetFileFromDatabaseToZip function - unable to execute get directory path statement.\n");
		return FALSE;
	}
	OCIHandleFree(hOraGetDirectoryPathStatement, OCI_HTYPE_STMT);

	int i;
	for (i = 0; *(orafileutf8 + i) != 0; i++)
	{
		if (*(orafileutf8 + i) == '/')
		{
			*(orafileutf8 + i) = '\\';
		}
	}

	if (*(orafileutf8 + i - 1) != '\\')
	{
		*(orafileutf8 + i) = '\\';
		*(orafileutf8 + i + 1) = 0;
	}

	strcat(orafileutf8, filenameutf8);
	MultiByteToWideChar(CP_UTF8, 0, orafileutf8, -1, orafile, 256);
	
	HANDLE hIn;
	DWORD nIn;
	const DWORD Bufsize = 1048576;
	oraub8 transferedData = 0;
	char *Buffer = (char *)HeapAlloc(GetProcessHeap(), 0, Bufsize);

	hIn = CreateFileW(orafile, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hIn == INVALID_HANDLE_VALUE)
	{
		HeapFree(GetProcessHeap(), 0, orafileutf8);
		HeapFree(GetProcessHeap(), 0, orafile);
		return FALSE;
	}
	LARGE_INTEGER FileSize;
	if (!GetFileSizeEx(hIn, &FileSize))
	{
		CloseHandle(hIn);
		HeapFree(GetProcessHeap(), 0, orafileutf8);
		HeapFree(GetProcessHeap(), 0, orafile);
		return FALSE;
	}


	//Структура для данных о зипфайле
	zipFile zf;
	//Вариант перезаписи
	int opt_overwrite = 0;
	//переменная для хранения ошибок от библиотеки zip
	int zip_err;
	//Структура для данных о записи в зипе
	zip_fileinfo zi;
	unsigned long crcFile = 0;
	int zip64 = 0;
	int opt_compress_level = Z_DEFAULT_COMPRESSION;
	const char* password = NULL;

	/* if the file don't exist, we not append file */
	if (check_exist_file(fullzipfilenameutf8) == 0)
		opt_overwrite = APPEND_STATUS_CREATE;
	else
		opt_overwrite = APPEND_STATUS_ADDINZIP;

	zf = zipOpen64(fullzipfilenameutf8, opt_overwrite);
	if (zf == NULL)
	{
		CloseHandle(hIn);
		HeapFree(GetProcessHeap(), 0, orafileutf8);
		HeapFree(GetProcessHeap(), 0, orafile);
		return FALSE;
	}

	zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
		zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
	zi.dosDate = 0;
	zi.internal_fa = 0;
	zi.external_fa = 0;
	filetime(&zi.tmz_date, &zi.dosDate);

	if (FileSize.QuadPart >= 0xffffffff)
	{
		zip64 = 1;
	}

	zip_err = zipOpenNewFileInZip4_64(zf, filenameutf8, &zi, NULL, 0, NULL, 0, NULL,
		(opt_compress_level != 0) ? Z_DEFLATED : 0, opt_compress_level, 0,
		-MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
		password, crcFile, 0, 1 << 11, zip64);
	if (zip_err != ZIP_OK)
	{
		CloseHandle(hIn);
		HeapFree(GetProcessHeap(), 0, orafileutf8);
		HeapFree(GetProcessHeap(), 0, orafile);
		zipClose(zf, NULL);
		return FALSE;
	}


	//printf("%s transfer progress - %llu%%(%llu/%llu)", filenameutf8, transferedData * 100 / (unsigned __int64)FileSize.QuadPart, (unsigned __int64)transferedData, (unsigned __int64)FileSize.QuadPart);
	while (ReadFile(hIn, Buffer, Bufsize, &nIn, NULL) && nIn > 0)
	{
		zip_err = zipWriteInFileInZip(zf, Buffer, (unsigned int)nIn);
		if (zip_err < 0)
		{
			printf("Application error, LocalGetFileFromDatabaseToZip function - error in writing in the zipfile\n");
			CloseHandle(hIn);
			HeapFree(GetProcessHeap(), 0, orafileutf8);
			HeapFree(GetProcessHeap(), 0, orafile);
			HeapFree(GetProcessHeap(), 0, Buffer);
			zipCloseFileInZip(zf);
			zipClose(zf, NULL);
			return FALSE;
		}

		
		transferedData = transferedData + nIn;
		//printf("\r");
		//printf("%s transfer progress - %llu%%(%llu/%llu)", filenameutf8, transferedData * 100 / (unsigned __int64)FileSize.QuadPart, (unsigned __int64)transferedData, (unsigned __int64)FileSize.QuadPart);
	}
	//printf("\r");
	//printf("%s transfer progress - %llu%%(%llu/%llu)\n", filenameutf8, transferedData * 100 / (unsigned __int64)FileSize.QuadPart, (unsigned __int64)transferedData, (unsigned __int64)FileSize.QuadPart);



	CloseHandle(hIn);

	HeapFree(GetProcessHeap(), 0, orafileutf8);
	HeapFree(GetProcessHeap(), 0, orafile);
	HeapFree(GetProcessHeap(), 0, Buffer);

	zip_err = zipCloseFileInZip(zf);
	if (zip_err != ZIP_OK)
	{
		zipClose(zf, NULL);
		return FALSE;
	}

	zip_err = zipClose(zf, NULL);
	if (zip_err != ZIP_OK)
	{
		return FALSE;
	}

	if (FileSize.QuadPart != transferedData)
	{
		return FALSE;
	}

	return TRUE;
}

/*Функция удаляет файл из Оракл каталога*/
bool DeleteFileFromOradir(OCISvcCtx *hOraSvcCtx, OCIEnv *hOraEnv, OCIError *hOraErr, char *oradirutf8, char *filenameutf8)
{
	sword status;

	const char plsql_delete_file_statement[] = "BEGIN\
								UTL_FILE.FREMOVE(:oradir, :dumpfile);\
								END;";


	OCIStmt *hOraPlsqlDeleteStatement = NULL;
	if (OCIHandleAlloc((const void *)hOraEnv, (void **)&hOraPlsqlDeleteStatement, OCI_HTYPE_STMT, (size_t)0, (void **)0))
	{
		printf("Application error, DeleteFileFromOradir function - can not allocate hOraPlsqlDeleteStatement handle\n");
		return FALSE;
	}
	

	if (OCIStmtPrepare(hOraPlsqlDeleteStatement, hOraErr, (const OraText *)plsql_delete_file_statement, (ub4)strlen(plsql_delete_file_statement), (ub4)OCI_NTV_SYNTAX, (ub4)OCI_DEFAULT) != OCI_SUCCESS)
	{
		printf("Application error, DeleteFileFromOradir function - unable to prepare statement plsql_delete_file_statement\n");
		OCIHandleFree(hOraPlsqlDeleteStatement, OCI_HTYPE_STMT);
		return FALSE;
	}

	OCIBind  *bnd4p = NULL;
	OCIBind  *bnd5p = NULL;
	OCIBindByName(hOraPlsqlDeleteStatement, &bnd4p, hOraErr, (text *)":dumpfile", -1, (void *)filenameutf8, (sb4)(strlen(filenameutf8) + 1), SQLT_STR, (void *)0, (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);
	OCIBindByName(hOraPlsqlDeleteStatement, &bnd5p, hOraErr, (text *)":oradir", -1, (void *)oradirutf8, 32, SQLT_STR, (void *)0, (ub2 *)0, (ub2 *)0, (ub4)0, (ub4 *)0, OCI_DEFAULT);

	status = OCIStmtExecute(hOraSvcCtx, hOraPlsqlDeleteStatement, hOraErr, (ub4)1, (ub4)0, (CONST OCISnapshot *) NULL, (OCISnapshot *)NULL, OCI_DEFAULT);
	if (status != OCI_SUCCESS && status != OCI_SUCCESS_WITH_INFO)
	{
		printf("Application error, DeleteFileFromOradir function - can not delete file from server\n");
		OCIHandleFree(hOraPlsqlDeleteStatement, OCI_HTYPE_STMT);
		checkerr(hOraErr, status);
		return FALSE;
	}

	OCIHandleFree(hOraPlsqlDeleteStatement, OCI_HTYPE_STMT);
	return TRUE;
}
