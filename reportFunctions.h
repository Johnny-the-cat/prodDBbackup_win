#pragma once

#include <stdio.h>
#include <windows.h>

/*Формат отчета JSON для ZABBIX
{
  startTime:1632326404770,
  startTimeStr:2021-09-22 19:00:04,
  finishTime:1632336257615,
  finishTimeStr:2021-09-22 21:44:17,
  success:false,
  schemaTotalCount:30,
  backupSuccessCount:27,
  exportWarnings:2,
  exportErrors:1,
  receiveErrors:1,
  deletedWhileBackup:1
}
*/

//Функция для генерации JSON для Zabbix по завершении экспорта
bool generateJSON(time_t startCTime, SYSTEMTIME startSystemTime, P_SCHEMA_LIST_STUCTURE ExportList, const char *filename);