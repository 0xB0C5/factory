#ifdef WEB
#include "fs.h"
#include <emscripten.h>


File::operator bool() const {
    return pos >= 0;
}

File File::openNextFile() {
    return File { .path = "", .pos = -1 };
}

const char *File::name() {
    return path;
}

void File::close() {
    pos = -1;
}

size_t File::read(void *dest, size_t size) {
    if (pos < 0) return 0;

    size_t count = EM_ASM_INT({
        return fileRead($0, $1, $2, $3);
    }, path, pos, dest, size);

    pos += count;

    return count;
}

size_t File::write(void *src, size_t size) {
    if (pos < 0) return 0;

    size_t count = EM_ASM_INT({
        return fileWrite($0, $1, $2, $3);
    }, path, pos, src, size);

    pos += count;

    return count;
}

void File::flush() {
    EM_ASM({
        fileFlush($0);
    }, path);
}

bool LittleFS_Program::begin(unsigned long size) {
    return true;
}
File LittleFS_Program::open(const char *path, int options) {
    int pos = 0;
    if (options & FILE_READ) {
        int exists = EM_ASM_INT({
            return fileExists($0);
        }, path);

        if (!exists) pos = -1;
    }

    return File { .path = path, .pos = pos };
}
void LittleFS_Program::remove(const char *path) {
    EM_ASM({
        fileRemove($0);
    }, path);
}
void LittleFS_Program::rename(const char *src, const char *dest) {
    EM_ASM({
        fileRename($0, $1);
    }, src, dest);
}

#endif
