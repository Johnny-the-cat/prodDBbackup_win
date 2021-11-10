#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>

//Выделяет необходимый буфер и переводит строку из UTF8 в UTF16 (UCS32)
wchar_t *str2wstr(const char *str) 
{
	int len = strlen(str) + 1;
	wchar_t *wstr = (wchar_t *)malloc(len * sizeof(wchar_t));
	if (MultiByteToWideChar(CP_UTF8, 0, str, len * sizeof(char), wstr, len) == 0)
	{
		return NULL;
	}
	return wstr;
}

//для виндовс - конвертирует строки в WCHAR и делает _wfopen
FILE *utf8_fopen(const char *pFilename, const char *pMode) 
{
	wchar_t *wFilename = str2wstr(pFilename);
	if (wFilename == NULL)
	{
		return NULL;
	}
	wchar_t *wMode = str2wstr(pMode);
	if (wMode == NULL)
	{
		free(wFilename);
		return NULL;
	}

	FILE *pFile = _wfopen(wFilename, wMode);

	free(wFilename);
	free(wMode);

	return pFile;
}

//для виндовс - конвертирует строки в WCHAR и делает DeleteFileW
int utf8_fdelete(const char *pFilename)
{
	wchar_t *wFilename = str2wstr(pFilename);
	if (wFilename == NULL)
	{
		return FALSE;
	}

	if (DeleteFileW(wFilename) == FALSE)
	{
		free(wFilename);
		return FALSE;
	}

	free(wFilename);

	return TRUE;
}

//Конвертация строки 1251 в UTF8
char *win1251str2utf8str(char *str)
{
	int len = strlen(str) + 1;
	wchar_t *wstr = (wchar_t *)malloc(len * sizeof(wchar_t));
	if (wstr == NULL)
	{
		return NULL;
	}

	if (MultiByteToWideChar(1251, 0, str, len * sizeof(char), wstr, len) == 0)
	{
		free(wstr);
		return NULL;
	}
	
	if (WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len * sizeof(wchar_t), NULL, NULL) == 0)
	{
		free(wstr);
		return NULL;
	}
	
	free(wstr);

	return str;
}

//Конвертация строки Windows ANSI Code Page (кодировка по умолчанию для русского языка - Win1251) в UTF8
char *winACPstr2utf8str(char *str)
{
	int len = strlen(str) + 1;
	wchar_t *wstr = (wchar_t *)malloc(len * sizeof(wchar_t));
	if (wstr == NULL)
	{
		return NULL;
	}

	if (MultiByteToWideChar(CP_ACP, 0, str, len * sizeof(char), wstr, len) == 0)
	{
		free(wstr);
		return NULL;
	}

	if (WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len * sizeof(wchar_t), NULL, NULL) == 0)
	{
		free(wstr);
		return NULL;
	}

	free(wstr);

	return str;
}

char *winACPstr2utf8strBufAlloc(const char *str)
{
	int len = strlen(str) + 1;
	wchar_t *wstr = (wchar_t *)malloc(len * sizeof(wchar_t));
	if (wstr == NULL)
	{
		return NULL;
	}
	//Выделяем строку с учетом максимального количества байт на символ
	char *utf8str = (char *)malloc(len * 4);
	if (utf8str == NULL)
	{
		free(wstr);
		return NULL;
	}

	if (MultiByteToWideChar(CP_ACP, 0, str, -1, wstr, len * sizeof(wchar_t)) == 0)
	{
		free(wstr);
		free(utf8str);
		return NULL;
	}
	if (WideCharToMultiByte(CP_UTF8, 0, wstr, -1, utf8str, len * 4, NULL, NULL) == 0)
	{
		free(wstr);
		free(utf8str);
		return NULL;
	}
	free(wstr);

	return utf8str;
}