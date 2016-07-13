// which.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Shlobj.h>


void usage(void);
void error_exit(char *);
char ** tokens(char * string, char *seps);
char * value_of(char * env[], char * name);
char * lower(char * string, int length);
char ** possiblefiles(char *name, char **ext);
char **directories(char ** paths);
char *canonical_dir(char * dir);
void display_time(char * label, FILETIME * ftTime);


char EXTENTIONS[4][5] = {".EXE", ".COM", ".BAT", NULL};
const int ATTRCOUNT = 8;
int ATTR[ATTRCOUNT] = {FILE_ATTRIBUTE_ARCHIVE, FILE_ATTRIBUTE_COMPRESSED,
				FILE_ATTRIBUTE_ENCRYPTED, FILE_ATTRIBUTE_HIDDEN,
				FILE_ATTRIBUTE_NORMAL, FILE_ATTRIBUTE_READONLY,
				FILE_ATTRIBUTE_SYSTEM, FILE_ATTRIBUTE_TEMPORARY };
char ATTRNAME[ATTRCOUNT][256] = {" archive", " compressed", " encrypted", " hidden", " normal", " readonly", " system", " temporary"};

char PATH_SEP[] =  ";";

int main(int argc, char* argv[], char* env[])
{
	BOOL details = FALSE;
	int index = 1;
	if(argc < 2 || argc > 3)
		usage();
	if(argc == 3){
		index = 2;
		details = TRUE;
		char * opt = lower(argv[1], strlen(argv[1]));
		if(strcmp(opt, "/d") != 0)
		if(strcmp(opt, "-d") != 0)
		if(strcmp(opt, "--details") != 0)
			usage();
	}
	else
		if(argv[1][0] == '/')
			usage();
	
	char * file = lower(argv[index], strlen(argv[index]));

	char **ext = NULL;
	char ** path = NULL;
	
	char * pathextvalue = value_of(env, "PATHEXT");

	
	if(pathextvalue)
		ext = tokens(pathextvalue, PATH_SEP);
	else
		ext = (char**)EXTENTIONS;

	char **possible = possiblefiles(file, ext);
	
	
	char *pathvalue = value_of(env, "Path");


	path = tokens(pathvalue, PATH_SEP);
	path = directories(path);

	int i=0;
	int j=0;
	while(path[i]){
		int pathlength = strlen(path[i]);
		j=0;
		while(possible[j]){
			int filelength = strlen(possible[j]);
			char * full = new char[pathlength+filelength+1];
			memcpy((void*)full, (void*)path[i], pathlength);
			memcpy((void*)&full[pathlength], possible[j], filelength);
			full[pathlength+filelength] = NULL;
			WIN32_FIND_DATA file_data;
			HANDLE hFind;
			hFind = FindFirstFile(full, &file_data);
			while(hFind != INVALID_HANDLE_VALUE){
				printf("%s%s\n", path[i],file_data.cFileName);
				if(details){
					printf("\tAttributes:");
					for(int k=0; k<ATTRCOUNT; k++)
						if(file_data.dwFileAttributes & ATTR[k])
							printf("%s", ATTRNAME[k]);
					printf("\n\tSize: %d bytes\n", (file_data.nFileSizeHigh * (MAXDWORD+1)) + file_data.nFileSizeLow);
					display_time("\tCreated: ", &file_data.ftCreationTime);
					display_time("\tLast writed: ", &file_data.ftLastWriteTime);
					display_time("\tLast access: ", &file_data.ftLastAccessTime);
					if(strlen(file_data.cAlternateFileName ) != 0)
						printf("\tAlternate file name: %s\n", file_data.cAlternateFileName);
				}
				BOOL result = FindNextFile(hFind, &file_data);
				if(!result)
				{
					FindClose(hFind);
					break;
				}
			}
			j++;
		}

		i++;
	}
	return 0;
}

void display_time(char * label, FILETIME * ftTime){
	SYSTEMTIME time;
	FILETIME ftLocalTime;
	FileTimeToLocalFileTime(ftTime, &ftLocalTime);
	FileTimeToSystemTime(&ftLocalTime, &time);
	printf("%s %d-%d-%d %d:%d:%d.%d\n", label, 
			time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds );     
}

char **directories(char ** paths){
	char **dirs = NULL;
	char current[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, current);
	
	if(!paths){
		dirs = (char**)new char[2];
		dirs[0] = canonical_dir(current);
		dirs[1] = NULL;
		return dirs;
	}
	int i=0;
	while(paths[i]){
		i++;
	}
	int length = i;
	dirs = (char **)new char[length+2];
	dirs[0] = canonical_dir(current);

	for(i=1; i<=length; i++){
		dirs[i] = canonical_dir(paths[i-1]);
	}
	dirs[length+1] = NULL;
	return dirs;
}

char * canonical_dir(char *srcdir){
	char * dir = NULL;
	
	int length = strlen(srcdir);
	
	if(strcmp("\\", &srcdir[length-1]) == 0)
		return _strdup(srcdir);
	
	dir = new char[length+2];
	strcpy(dir, srcdir);
	dir[length] = '\\';
	dir[length+1] = NULL;
	return dir;
}

char ** possiblefiles(char *name, char **ext){
	char * dot = strrchr(name, '.');
	
	
	char ** possible = NULL;
	int i=0;
	
	if(dot){
		possible = (char**)new char[2];
		possible[0] = _strdup(name);
		possible[1] = NULL;
		return possible;
	}
	while(ext[i]){
		i++;
	}
	
	int length = i+1;
	possible = (char **)new char[length];
	i=0;
	int fnlength = strlen(name);
	while(ext[i]){
		possible[i] = new char[fnlength + strlen(ext[i])+1];
		possible[i] = strcpy(possible[i], name);
		possible[i] = strcat(possible[i], ext[i]);
		i++;
	}
	possible[length-1] = NULL;
	return possible;
}

char * lower(char * string, int length){
	char * low = new char[length+1];
	
	if(!low)
		return NULL;
	for(int i=0; i<length; i++){
		if(string[i] >= 'A' && string[i] <= 'Z')
			low[i] = string[i] + 32;
		else
			low[i] = string[i];
	}
	low[length] = NULL;
	return low;
}

char * value_of(char * env[], char * name){
	int i = 0;
	int length = strlen(name);
	
	char * lowername = lower(name, length);
	
	while(env[i]){
		char * current = lower(env[i], length);
		if(strncmp(lowername, current, length) == 0) {
			if(env[i][length] == '=' && strlen(env[i])>length+1)
			{
				delete lowername;
				delete current;
				return &env[i][length+1];
			}
		}
		i++;
		delete current;
	}
	delete lowername;
	return NULL;
}

char ** tokens(char * string, char *seps){
	char * tmp = new char[strlen(string)];
	tmp = strcpy(tmp, string);
	
	char ** result = NULL;
	char * token = strtok(tmp, seps);
	
	int count = 0;
	while(token)
	{
		count++;
		token = strtok(NULL, seps);
	}
	
	if(count ==0)
		return NULL;

	result =  (char**)new char[count+1];
	
	token = strtok(string, seps);
	result[0] = token;
	for(int i=1; i<count; i++){
		result[i] = strtok(NULL, seps);

	}
	result[count] = NULL;
	return result;
}

void error_exit(char * message){
	printf("%s\n", message);
	exit(1);
}

void usage(void){
	printf("which for Windows.\nUsage: which [-d] executable_name[.extension]\n\t -d display details for finded files.");
	exit(1);
}
