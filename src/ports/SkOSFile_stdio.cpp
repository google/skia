/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOSFile.h"
#include "SkTypes.h"

#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#endif

#ifdef SK_BUILD_FOR_IOS
#import <CoreFoundation/CoreFoundation.h>

static FILE* ios_open_from_bundle(const char path[], const char* perm) {
    // Get a reference to the main bundle
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    
    // Get a reference to the file's URL
    CFStringRef pathRef = CFStringCreateWithCString(NULL, path, kCFStringEncodingUTF8);
    CFURLRef imageURL = CFBundleCopyResourceURL(mainBundle, pathRef, NULL, NULL);

    CFRelease(pathRef);

    if (!imageURL) {
        return nullptr;
    }
    
    // Convert the URL reference into a string reference
    CFStringRef imagePath = CFURLCopyFileSystemPath(imageURL, kCFURLPOSIXPathStyle);
    
    // Get the system encoding method
    CFStringEncoding encodingMethod = CFStringGetSystemEncoding();
    
    // Convert the string reference into a C string
    const char *finalPath = CFStringGetCStringPtr(imagePath, encodingMethod);

    CFRelease(imageURL);
    CFRelease(imagePath);

    return fopen(finalPath, perm);
}
#endif


SkFILE* sk_fopen(const char path[], SkFILE_Flags flags) {
    char    perm[4];
    char*   p = perm;

    if (flags & kRead_SkFILE_Flag) {
        *p++ = 'r';
    }
    if (flags & kWrite_SkFILE_Flag) {
        *p++ = 'w';
    }
    *p++ = 'b';
    *p = 0;
    
    //TODO: on Windows fopen is just ASCII or the current code page,
    //convert to utf16 and use _wfopen
    SkFILE* file = nullptr;
#ifdef SK_BUILD_FOR_IOS
    // if read-only, try to open from bundle first
    if (kRead_SkFILE_Flag == flags) {
        file = (SkFILE*)ios_open_from_bundle(path, perm);
    }
    // otherwise just read from the Documents directory (default)
    if (!file) {
#endif
        file = (SkFILE*)::fopen(path, perm);
#ifdef SK_BUILD_FOR_IOS
    }
#endif
    if (nullptr == file && (flags & kWrite_SkFILE_Flag)) {
        SkDEBUGF(("sk_fopen: fopen(\"%s\", \"%s\") returned NULL (errno:%d): %s\n",
                  path, perm, errno, strerror(errno)));
    }
    return file;
}

char* sk_fgets(char* str, int size, SkFILE* f) {
    return ::fgets(str, size, (FILE *)f);
}

int sk_feof(SkFILE *f) {
    // no :: namespace qualifier because it breaks android
    return feof((FILE *)f);
}

size_t sk_fgetsize(SkFILE* f) {
    SkASSERT(f);

    long curr = ::ftell((FILE*)f); // remember where we are
    if (curr < 0) {
        return 0;
    }

    ::fseek((FILE*)f, 0, SEEK_END); // go to the end
    long size = ::ftell((FILE*)f); // record the size
    if (size < 0) {
        size = 0;
    }

    ::fseek((FILE*)f, curr, SEEK_SET); // go back to our prev location
    return size;
}

bool sk_frewind(SkFILE* f) {
    SkASSERT(f);
    ::rewind((FILE*)f);
    return true;
}

size_t sk_fread(void* buffer, size_t byteCount, SkFILE* f) {
    SkASSERT(f);
    if (buffer == nullptr) {
        size_t curr = ::ftell((FILE*)f);
        if ((long)curr == -1) {
            SkDEBUGF(("sk_fread: ftell(%p) returned -1 feof:%d ferror:%d\n", f, feof((FILE*)f), ferror((FILE*)f)));
            return 0;
        }
        int err = ::fseek((FILE*)f, (long)byteCount, SEEK_CUR);
        if (err != 0) {
            SkDEBUGF(("sk_fread: fseek(%d) tell:%d failed with feof:%d ferror:%d returned:%d\n",
                        byteCount, curr, feof((FILE*)f), ferror((FILE*)f), err));
            return 0;
        }
        return byteCount;
    }
    else
        return ::fread(buffer, 1, byteCount, (FILE*)f);
}

size_t sk_fwrite(const void* buffer, size_t byteCount, SkFILE* f) {
    SkASSERT(f);
    return ::fwrite(buffer, 1, byteCount, (FILE*)f);
}

void sk_fflush(SkFILE* f) {
    SkASSERT(f);
    ::fflush((FILE*)f);
}

bool sk_fseek(SkFILE* f, size_t byteCount) {
    int err = ::fseek((FILE*)f, (long)byteCount, SEEK_SET);
    return err == 0;
}

bool sk_fmove(SkFILE* f, long byteCount) {
    int err = ::fseek((FILE*)f, byteCount, SEEK_CUR);
    return err == 0;
}

size_t sk_ftell(SkFILE* f) {
    long curr = ::ftell((FILE*)f);
    if (curr < 0) {
        return 0;
    }
    return curr;
}

void sk_fclose(SkFILE* f) {
    SkASSERT(f);
    ::fclose((FILE*)f);
}

bool sk_isdir(const char *path) {
    struct stat status;
    if (0 != stat(path, &status)) {
        return false;
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
#endif
    if (0 == retval) {
        return true;
    } else {
        fprintf(stderr, "sk_mkdir: error %d creating dir '%s'\n", errno, path);
        return false;
    }
}
