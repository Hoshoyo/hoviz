#pragma once
#include <stdlib.h>

char* os_file_read(const char* path, int* file_length, void *(*allocator)(size_t));