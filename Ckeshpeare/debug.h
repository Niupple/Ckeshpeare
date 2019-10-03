#pragma once
#include <cstdio>
#ifdef _DEBUG
#define DP(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#else
#define DP(...) 0
#endif