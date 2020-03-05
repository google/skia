/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "src/core/SkOSFile.h"

#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>

#ifdef SK_BUILD_FOR_UNIX
#include <unistd.h>
#endif

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <vector>
#include "src/utils/SkUTF.h"
#endif

#ifdef SK_BUILD_FOR_IOS
#include "src/ports/SkOSFile_ios.h"
#endif

#ifdef _WIN32
static bool is_ascii(const char* s) {
    while (char v = *s++) {
        if ((v & 0x80) != 0) {
            return false;
        }
    }
    return true;
}

static FILE* fopen_win(const char* utf8path, const char* perm) {
    if (is_ascii(utf8path)) {
        return fopen(utf8path, perm);
    }

    const char* ptr = utf8path;
    const char* end = utf8path + strlen(utf8path);
    size_t n = 0;
    while (ptr < end) {
        SkUnichar u = SkUTF::NextUTF8(&ptr, end);
        if (u < 0) {
            return nullptr;  // malformed UTF-8
        }
        n += SkUTF::ToUTF16(u);
    }
    std::vector<uint16_t> wchars(n + 1);
    uint16_t* out = wchars.data();
    for (const char* ptr = utf8path; ptr < end;) {
        out += SkUTF::ToUTF16(SkUTF::NextUTF8(&ptr, end), out);
    }
    SkASSERT(out == &wchars[n]);
    *out = 0; // final null
    wchar_t wperms[4] = {(wchar_t)perm[0], (wchar_t)perm[1], (wchar_t)perm[2], (wchar_t)perm[3]};
    return _wfopen((wchar_t*)wchars.data(), wperms);
}
#endif

FILE* sk_fopen(const char path[], SkFILE_Flags flags) {
    char    perm[4] = {0, 0, 0, 0};
    char*   p = perm;

    if (flags & kRead_SkFILE_Flag) {
        *p++ = 'r';
    }
    if (flags & kWrite_SkFILE_Flag) {
        SkASSERT(!(flags & kAppend_SkFILE_Flag));
        *p++ = 'w';
    } else if (flags & kAppend_SkFILE_Flag) {
        *p++ = 'a';
    }
    *p = 'b';

    FILE* file = nullptr;
#ifdef _WIN32
    file = fopen_win(path, perm);
#else
    file = fopen(path, perm);
#endif
#ifdef SK_BUILD_FOR_IOS
    // if not found in default path and read-only, try to open from bundle
    if (!file && kRead_SkFILE_Flag == flags) {
        SkString bundlePath;
        if (ios_get_path_in_bundle(path, &bundlePath)) {
            file = fopen(bundlePath.c_str(), perm);
        }
    }
#endif

    if (nullptr == file && (flags & kWrite_SkFILE_Flag)) {
        SkDEBUGF("sk_fopen: fopen(\"%s\", \"%s\") returned nullptr (errno:%d): %s\n",
                 path, perm, errno, strerror(errno));
    }
    return file;
}

size_t sk_fgetsize(FILE* f) {
    SkASSERT(f);

    long curr = ftell(f); // remember where we are
    if (curr < 0) {
        return 0;
    }

    fseek(f, 0, SEEK_END); // go to the end
    long size = ftell(f); // record the size
    if (size < 0) {
        size = 0;
    }

    fseek(f, curr, SEEK_SET); // go back to our prev location
    return size;
}

size_t sk_fwrite(const void* buffer, size_t byteCount, FILE* f) {
    SkASSERT(f);
    return fwrite(buffer, 1, byteCount, f);
}

void sk_fflush(FILE* f) {
    SkASSERT(f);
    fflush(f);
}

void sk_fsync(FILE* f) {
#if !defined(_WIN32) && !defined(SK_BUILD_FOR_ANDROID) && !defined(__UCLIBC__) \
        && !defined(_NEWLIB_VERSION)
    int fd = fileno(f);
    fsync(fd);
#endif
}

size_t sk_ftell(FILE* f) {
    long curr = ftell(f);
    if (curr < 0) {
        return 0;
    }
    return curr;
}

void sk_fclose(FILE* f) {
    if (f) {
        fclose(f);
    }
}

bool sk_isdir(const char *path) {
    struct stat status;
    if (0 != stat(path, &status)) {
#ifdef SK_BUILD_FOR_IOS
        // check the bundle directory if not in default path
        SkString bundlePath;
        if (ios_get_path_in_bundle(path, &bundlePath)) {
            if (0 != stat(bundlePath.c_str(), &status)) {
                return false;
            }
        }
#else
        return false;
#endif
    }
    return SkToBool(status.st_mode & S_IFDIR);
}

bool sk_mkdir(const char* path) {
    if (sk_isdir(path)) {
        return true;
    }
    if (sk_exists(path)) {
        fprintf(stderr,
                "sk_mkdir: path '%s' already exists but is not a directory\n",
                path);
        return false;
    }

    int retval;
#ifdef _WIN32
    retval = _mkdir(path);
#else
    retval = mkdir(path, 0777);
    if (retval) {
      perror("mkdir() failed with error: ");
    }
#endif
    return 0 == retval;
}
