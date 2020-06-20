#include <stdio.h>
#include <stdlib.h>

char*
os_file_read(const char* path, int* file_length, void *(*allocator)(size_t))
{
  FILE* file;
  char* buffer;
  int len;

  file = fopen(path, "rb");
  if (file == NULL)
  {
    printf("os_file_read: could not open file %s\n", path);
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  len = ftell(file);
  rewind(file);

  buffer = (char *) allocator((len + 1) * sizeof(char));
  if (fread(buffer, 1, len, file) != len)
  {
    printf("os_file_read: could not read file %s\n", path);
    fclose(file);
    free(buffer);
    return NULL;
  }

  fclose(file);

  buffer[len] = '\0';

  if (file_length)
    *file_length = len;

  return buffer;
}
