#pragma once

#include <windows.h>
#include <stdbool.h>
#include "oratypes.h"
#include "oci.h"

#define USER_EXISTS 1
#define USER_NOT_EXISTS 2

#define NOT_PROCESSED 0

#define EXPORT_COMPLETE 1
#define EXPORT_ERROR 3
#define EXPORT_RUNNING 4
#define EXPORT_WARNING 5

#define RECEIVE_COMPLETE 1
#define RECEIVE_ERROR 3
#define RECEIVE_RUNNING 4
#define RECEIVE_SKIP 5

#define LOCALDB 1
#define REMOTEDB 2

typedef struct datapump_dir_row
{
	//порядковый номер каталога
	int serialNumber;
	//Имя датапамп директории для дампа.
	char datapumpDirName[32];

} DATAPUMP_DIR_ROW_STRUCTURE, *P_DATAPUMP_DIR_ROW_STRUCTURE;

typedef struct export_schema_row
{
	//Имя схемы
	char schema[32];
	//Имя файла без расширения.
	char filename [256];
	//Имя датапамп директории для дампа.
	char datapumpDirName[32];
	//Флаг состояния схемы - 0 - необработана (NOT_PROCESSED), EXPORT_COMPLETE - Экспорт завершен, USER_NOT_EXISTS - пользователя не существует, пропускаем, 
	//EXPORT_RUNNING - идет в настоящий момент, EXPORT_ERROR - ошибка экспорта, EXPORT_WARNING - предупреждения во время экспорта
	int exportStatus;
	//Флаг состояния получения - 0 - необработана, RECEIVE_RUNNING - идет получение, RECEIVE_COMPLETE - файлы получены, RECEIVE_ERROR - ошибка получения, RECEIVE_SKIP - пропущен
	int receiveStatus;

} SCHEMA_ROW_STUCTURE, *P_SCHEMA_ROW_STUCTURE;

typedef struct export_schema_list
{
	int schemaCount;
	P_SCHEMA_ROW_STUCTURE pSchemaRows;

} SCHEMA_LIST_STUCTURE, *P_SCHEMA_LIST_STUCTURE;

bool LoadOciFunctions(HMODULE hOCIDll);

void checkerr(OCIError *errhp, sword status);

bool InitOraEnvironment(OCIEnv **hOraEnv, OCIError **hOraErr);
void CloseOraEnvironment(OCIEnv *hOraEnv, OCIError *hOraErr);

bool CreateSession(OCIEnv *hOraEnv, OCIError *hOraErr, OCIServer **hOraServer, OCISvcCtx **hOraSvcCtx, OCISession **hOraSession, char *usernameutf8, char *passwordutf8, char *dbnameutf8, bool assysdba);
void CloseSession(OCIError *hOraErr, OCIServer *hOraServer, OCISvcCtx *hOraSvcCtx, OCISession *hOraSession);

//В функцию передаем запрос, результатом которого должен быть набор строк (один столбец), strResult - указатель на буфер приема, sizeStrResult - размер буфера.
//Возвращается указатель на хендл выражения OCIStmt. При ошибке возращается NULL
OCIStmt *GetStringsSetPrepare(OCISvcCtx *hOraSvcCtx, OCIEnv *hOraEnv, OCIError *hOraErr, const char * strSelect, char * strResult, int sizeStrResult);
//Функция получает следующую строку в буфер.
sword GetStringsSetFetch(OCIStmt *hOraPlsqlUserToListStatement, OCIError *hOraErr);
//Освобождаем хендл, после использования.
void FreeSqlHandle(OCIStmt *hOraPlsqlUserToListStatement);

//Функция для получения из базы одного целочисленного числа
bool GetNumberFromSelect(OCISvcCtx *hOraSvcCtx, OCIEnv *hOraEnv, OCIError *hOraErr, const char * strSelect, long long *Result);
//Функция проверки существования пользователя,  возвращает USER_EXISTS либо USER_NOT_EXISTS
int IsUserExists(OCISvcCtx *hOraSvcCtx, OCIEnv *hOraEnv, OCIError *hOraErr, const char *schema);

//Эта версия функции отличается от подобной в datapumpexp тем, что здесь не запускается в отдельном потоке функция отслеживания прогресса экспорта.
//мы просто запускаем экспорт и ждем его завершения
int ExecuteExportJob(OCISvcCtx *hOraSvcCtx, OCIEnv *hOraEnv, OCIError *hOraErr, const char *schemautf8, const char *oradirutf8, const char *dumpfilenameutf8, const char *logfilenameutf8);

//Эта версия функции ExecuteExportJob, в которой мы отслеживаем сообщения об ошибках.
//Отдельный поток для отслеживания прогресса на запускается, мы просто запускаем экспорт и ждем его завершения. Флаг consistent для создания консистентного дампа
int ExecuteExportJobErrorTracking(OCISvcCtx *hOraSvcCtx, OCIEnv *hOraEnv, OCIError *hOraErr, const char *schemautf8, const char *oradirutf8, const char *dumpfilenameutf8, const char *logfilenameutf8,
	int *errorCount, bool consistent);

//Проверяет, расположен ли сервер на этом же компьютере, возвращает LOCALDB или REMOTEDB
int IsDBLocal(OCISvcCtx *hOraSvcCtx, OCIEnv *hOraEnv, OCIError *hOraErr);

//Функция выполняем получение файла с удаленного сервера оракл со сжатием на лету, четвертый параметр - полное имя зипфайла, пятый - имя Оракл директории, шестой - имя файла внутри Оракл
//Отличается от аналогичной функции в datapumpexp тем, что тут нет отображения прогресса скачки файла.
bool GetFileFromDatabaseToZip(OCISvcCtx *hOraSvcCtx, OCIEnv *hOraEnv, OCIError *hOraErr, char *fullzipfilenameutf8, char *oradirutf8, char *filenameutf8);

/*Функция выполняем получение файла с локального сервера оракл в зип-фархив, четвертый параметр - полное имя зипфайла, пятый - имя Оракл директории, шестой - имя файла внутри Оракл*/
//Отличается от аналогичной функции в datapumpexp тем, что тут нет отображения прогресса скачки файла.
bool LocalGetFileFromDatabaseToZip(OCISvcCtx *hOraSvcCtx, OCIEnv *hOraEnv, OCIError *hOraErr, char *fullzipfilenameutf8, char *oradirutf8, char *filenameutf8);

/*Функция удаляет файл из Оракл каталога*/
bool DeleteFileFromOradir(OCISvcCtx *hOraSvcCtx, OCIEnv *hOraEnv, OCIError *hOraErr, char *oradirutf8, char *filenameutf8);