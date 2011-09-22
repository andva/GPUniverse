#ifndef FILELOADER_H
#define FILELOADER_H

#ifdef _WIN32
#include "windows.h"
#endif
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <assert.h>
using namespace std;
class Fileloader
{
public:
	static char *load_source(const char *filename);
};
#endif