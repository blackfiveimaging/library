/*
 * util.h
 * Provides miscellaneous utility functions
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef UTIL_H
#define UTIL_H

void CreateDirIfNeeded(const char *dirname);
bool CheckSettingsDir(const char *dirname);

char *BuildFilename(const char *root,const char *suffix,const char *fileext);
char *BuildAbsoluteFilename(const char *fname);
char *SerialiseFilename(const char *fname,int serialno,int max=0);

char *SafeStrdup(const char *src);
char *SafeStrcat(const char *str1,const char *str2);
int StrcasecmpIgnoreSpaces(const char *str1,const char *str2);

int TestHostName(char *str,char **hostname,int *port);

bool CompareFiles(const char *fn1,const char *fn2);

#endif
