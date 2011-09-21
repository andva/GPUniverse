#ifdef _WIN32
#include "windows.h"
#endif
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <assert.h>
using namespace std;
class fileloader
{
public:
	static char *load_source(const char *filename);
};