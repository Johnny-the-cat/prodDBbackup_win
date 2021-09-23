#pragma once

#include <windows.h>
#include "oratypes.h"
#include "oci.h"

#define USER_EXISTS 1
#define USER_NOT_EXISTS 2
#define NOT_PROCESSED 0
#define EXPORT_COMPLETE 1
#define EXPORT_ERROR 3

#define LOCALDB 1
#define REMOTEDB 2

typedef struct export_schema_row
{
	//Имя схемы
	char schema[32];
	//Имя файла без расширения.
	char filename [256];
	//Флаг состояния схемы - 0 - необработана, EXPORT_COMPLETE - Экспорт завершен, USER_NOT_EXISTS - пользователя не существует, пропускаем.
	int status;

} SCHEMA_ROW_STUCTURE, *P_SCHEMA_ROW_STUCTURE;

typedef struct export_schema_list
{
	int schemaCount;
	P_SCHEMA_ROW_STUCTURE pSchemaRows;

} SCHEMA_LIST_STUCTURE, *P_SCHEMA_LIST_STUCTURE;

BOOL LoadOciFunctions(HMODULE hOCIDll);

void checkerr(OCIError *errhp, sword status);

BOOL InitOraEnvironment(OCIEnv **hOraEnv, OCIError **hOraErr);
void CloseOraEnvironment(OCIEnv *hOraEnv, OCIError *hOraErr);

BOOL CreateSession(OCIEnv *hOraEnv, OCIError *hOraErr, OCIServer **hOraServer, OCISvcCtx **hOraSvcCtx, OCISession **hOraSession, char *usernameutf8, char *passwordutf8, char *dbnameutf8, BOOL assysdba);
void CloseSession(OCIError *hOraErr, OCIServer *hOraServer, OCISvcCtx *hOraSvcCtx, OCISession *hOraSession);

//В функцию передаем запрос, результатом которого должен быть набор строк (один столбец), strResult - указатель на буфер приема, sizeStrResult - размер буфера.
//Возвращается указатель на хендл выражения OCIStmt. При ошибке возращается NULL
OCIStmt *GetStringsSetPrepare(OCISvcCtx *hOraSvcCtx, OCIEnv *hOraEnv, OCIError *hOraErr, const char * strSelect, char * strResult, int sizeStrResult);
//Функция получает следующую строку в буфер.
sword GetStringsSetFetch(OCIStmt *hOraPlsqlUserToListStatement, OCIError *hOraErr);
//Освобождаем хендл, после использования.
void FreeSqlHandle(OCIStmt *hOraPlsqlUserToListStatement);

//Функция для получения из базы одного целочисленного числа
BOOL GetNumberFromSelect(OCISvcCtx *hOraSvcCtx, OCIEnv *hOraEnv, OCIError *hOraErr, const char * strSelect, long long *Result);
//Функция проверки существования пользователя,  возвращает USER_EXISTS либо USER_NOT_EXISTS
int IsUserExists(OCISvcCtx *hOraSvcCtx, OCIEnv *hOraEnv, OCIError *hOraErr, const char *schema);

//Эта версия функции отличается от подобной в datapumpexp тем, что здесь не запускается в отдельном потоке функция отслеживания прогресса экспорта.
//мы просто запускаем экспорт и ждем его завершения
int ExecuteExportJob(OCISvcCtx *hOraSvcCtx, OCIEnv *hOraEnv, OCIError *hOraErr, const char *schemautf8, const char *oradirutf8, const char *dumpfilenameutf8, const char *logfilenameutf8);

//Проверяет, расположен ли сервер на этом же компьютере, возвращает LOCALDB или REMOTEDB
int IsDBLocal(OCISvcCtx *hOraSvcCtx, OCIEnv *hOraEnv, OCIError *hOraErr);

//Функция выполняем получение файла с удаленного сервера оракл со сжатием на лету, четвертый параметр - полное имя зипфайла, пятый - имя Оракл директории, шестой - имя файла внутри Оракл
//Отличается от аналогичной функции в datapumpexp тем, что тут нет отображения прогресса скачки файла.
BOOL GetFileFromDatabaseToZip(OCISvcCtx *hOraSvcCtx, OCIEnv *hOraEnv, OCIError *hOraErr, char *fullzipfilenameutf8, char *oradirutf8, char *filenameutf8);

/*Функция выполняем получение файла с локального сервера оракл в зип-фархив, четвертый параметр - полное имя зипфайла, пятый - имя Оракл директории, шестой - имя файла внутри Оракл*/
//Отличается от аналогичной функции в datapumpexp тем, что тут нет отображения прогресса скачки файла.
BOOL LocalGetFileFromDatabaseToZip(OCISvcCtx *hOraSvcCtx, OCIEnv *hOraEnv, OCIError *hOraErr, char *fullzipfilenameutf8, char *oradirutf8, char *filenameutf8);

/*Функция удаляет файл из Оракл каталога*/
BOOL DeleteFileFromOradir(OCISvcCtx *hOraSvcCtx, OCIEnv *hOraEnv, OCIError *hOraErr, char *oradirutf8, char *filenameutf8);