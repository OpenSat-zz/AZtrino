#include <stdio.h>

#include "init_cs.h"

static const char * FILENAME = "init_cs.cpp";

void init_cs_api() { printf("%s:%s\n", FILENAME, __FUNCTION__); }
void shutdown_cs_api() { printf("%s:%s\n", FILENAME, __FUNCTION__); }
