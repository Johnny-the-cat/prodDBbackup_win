#pragma once
#include <stdio.h>
#include <windows.h>

//Выделяет необходимый буфер и переводит строку из UTF8 в UTF16 (UCS32)
wchar_t *str2wstr(const char *str);

//для виндовс - конвертирует строки в WCHAR и делает _wfopen
FILE *utf8_fopen(const char *pFilename, const char *pMode);

//Конвертация строки 1251 в UTF8. Строка конвертирется в исходный буфер, это нужно учитывать,чтобы не было переполнения.
char *win1251str2utf8str(char *str);

//Конвертация строки Windows ANSI Code Page (кодировка по умолчанию для русского языка - Win1251) в UTF8
//Строка конвертирется в исходный буфер, это нужно учитывать, чтобы не было переполнения.
char *winACPstr2utf8str(char *str);

//Конвертация строки Windows ANSI Code Page (кодировка по умолчанию для русского языка - Win1251) в UTF8
//Под строку выделяется буфер, после использования его нужно освободить.
char *winACPstr2utf8strBufAlloc(char *str);

//для виндовс - конвертирует строки в WCHAR и делает DeleteFileW. Возвращает TRUE или FALSE
int utf8_fdelete(const char *pFilename);