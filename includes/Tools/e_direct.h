
#ifndef E_DIRECT_H
#define E_DIRECT_H

#include <stdio.h>
#ifdef _WIN_BUILD_
#include "windows.h"
#else
#include <unistd.h>
#endif

char* DirectGetCurrectFilePath();
int DirectIsFileExist(const char *path);

char *DirectGetFileExt(const char *name_file);
char *DirectGetFileWithoutExt(const char *name_file);
char *DirectGetFileName(const char *name_file);

#endif // E_DIRECT_H
