/* iowin32.h -- IO base function header for compress/uncompress .zip
     Version 1.1, February 14h, 2010
     part of the MiniZip project - ( http://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2010 Gilles Vollant (minizip) ( http://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( http://result42.com )

         For more info read MiniZip_info.txt

*/

#include <windows.h>
#include "StringConvert.h"


#ifdef __cplusplus
extern "C" {
#endif

void fill_win32_filefunc OF((zlib_filefunc_def* pzlib_filefunc_def));
void fill_win32_filefunc64 OF((zlib_filefunc64_def* pzlib_filefunc_def));
void fill_win32_filefunc64A OF((zlib_filefunc64_def* pzlib_filefunc_def));
void fill_win32_filefunc64W OF((zlib_filefunc64_def* pzlib_filefunc_def));

//Выделяет необходимый буфер и переводит строку из UTF8 в UTF16 (UCS32)
//wchar_t *str2wstr(const char *str);

//FILE *mz_fopen(const char *pFilename, const char *pMode);

//Конвертация строки 1251 в UTF8
//const char *win1251str2utf8str(char *str);

#ifdef __cplusplus
}
#endif