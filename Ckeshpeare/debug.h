#pragma once
#include <cstdio>
#ifdef _DEBUG
#define DP(fmt, ...) fprintf(stderr, "#%4d: " fmt, __LINE__, ##__VA_ARGS__)
#else
#define DP(...) 
#endif