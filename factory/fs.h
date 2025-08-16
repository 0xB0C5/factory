#ifdef WEB

#include <cstddef>

#define FILE_READ 1
#define FILE_WRITE 2


struct File {
    const char *path;
    int pos;

    operator bool() const;

    File openNextFile();
    const char *name();
    void close();
    size_t read(void *dest, size_t size);
    size_t write(void *src, size_t size);
    void flush();
};

struct LittleFS_Program {
    bool begin(unsigned long size);
    File open(const char *path, int options);
    void remove(const char *path);
    void rename(const char *src, const char *dest);
};

#else

#include "LittleFS.h"

#endif
