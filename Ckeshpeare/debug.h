#pragma once
#include <cstdio>
#ifdef _DEBUG
#define DP(fmt, ...) fprintf(stderr, "#%4d[%s]: " fmt, __LINE__, __FILE__, ##__VA_ARGS__)
#else
#define DP(...) 
#endif