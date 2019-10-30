/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTFitsIn.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkOSFile.h"

#include <dirent.h>
#include <new>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef SK_BUILD_FOR_IOS
#include "src/ports/SkOSFile_ios.h"
#endif

bool sk_exists(const char *path, SkFILE_Flags flags) {
    int mode = F_OK;
    if (flags & kRead_SkFILE_Flag) {
        mode |= R_OK;
    }
    if (flags & kWrite_SkFILE_Flag) {
        mode |= W_OK;
    }
#ifdef SK_BUILD_FOR_IOS
    // if the default path fails, check the bundle (but only if read-only)
    if (0 == access(path, mode)) {
        return true;
    } else {
        return (kRead_SkFILE_Flag == flags && ios_get_path_in_bundle(path, nullptr));
    }
#else
    return (0 == access(path, mode));
#endif
}

typedef struct {
    dev_t dev;
    ino_t ino;
} SkFILEID;

static bool sk_ino(FILE* a, SkFILEID* id) {
    int fd = fileno(a);
    if (fd < 0) {
        return 0;
    }
    struct stat status;
    if (0 != fstat(fd, &status)) {
        return 0;
    }
    id->dev = status.st_dev;
    id->ino = status.st_ino;
    return true;
}

bool sk_fidentical(FILE* a, FILE* b) {
    SkFILEID aID, bID;
    return sk_ino(a, &aID) && sk_ino(b, &bID)
           && aID.ino == bID.ino
           && aID.dev == bID.dev;
}

void sk_fmunmap(const void* addr, size_t length) {
    munmap(const_cast<void*>(addr), length);
}

void* sk_fdmmap(int fd, size_t* size) {
    struct stat status;
    if (0 != fstat(fd, &status)) {
        return nullptr;
    }
    if (!S_ISREG(status.st_mode)) {
        return nullptr;
    }
    if (!SkTFitsIn<size_t>(status.st_size)) {
        return nullptr;
    }
    size_t fileSize = static_cast<size_t>(status.st_size);

    void* addr = mmap(nullptr, fileSize, PROT_READ, MAP_PRIVATE, fd, 0);
    if (MAP_FAILED == addr) {
        return nullptr;
    }

    *size = fileSize;
    return addr;
}

int sk_fileno(FILE* f) {
    return fileno(f);
}

void* sk_fmmap(FILE* f, size_t* size) {
    int fd = sk_fileno(f);
    if (fd < 0) {
        return nullptr;
    }

    return sk_fdmmap(fd, size);
}

size_t sk_qread(FILE* file, void* buffer, size_t count, size_t offset) {
    int fd = sk_fileno(file);
    if (fd < 0) {
        return SIZE_MAX;
    }
    ssize_t bytesRead = pread(fd, buffer, count, offset);
    if (bytesRead < 0) {
        return SIZE_MAX;
    }
    return bytesRead;
}

////////////////////////////////////////////////////////////////////////////

struct SkOSFileIterData {
    SkOSFileIterData() : fDIR(nullptr) { }
    DIR* fDIR;
    SkString fPath, fSuffix;
};
static_assert(sizeof(SkOSFileIterData) <= SkOSFile::Iter::kStorageSize, "not_enough_space");

SkOSFile::Iter::Iter() { new (fSelf.get()) SkOSFileIterData; }

SkOSFile::Iter::Iter(const char path[], const char suffix[]) {
    new (fSelf.get()) SkOSFileIterData;
    this->reset(path, suffix);
}

SkOSFile::Iter::~Iter() {
    SkOSFileIterData& self = *static_cast<SkOSFileIterData*>(fSelf.get());
    if (self.fDIR) {
        ::closedir(self.fDIR);
    }
    self.~SkOSFileIterData();
}

void SkOSFile::Iter::reset(const char path[], const char suffix[]) {
    SkOSFileIterData& self = *static_cast<SkOSFileIterData*>(fSelf.get());
    if (self.fDIR) {
        ::closedir(self.fDIR);
        self.fDIR = nullptr;
    }
    self.fPath.set(path);

    if (path) {
        self.fDIR = ::opendir(path);
#ifdef SK_BUILD_FOR_IOS
        // check bundle for directory
        if (!self.fDIR && ios_get_path_in_bundle(path, &self.fPath)) {
            self.fDIR = ::opendir(self.fPath.c_str());
        }
#endif
        self.fSuffix.set(suffix);
    } else {
        self.fSuffix.reset();
    }
}

// returns true if suffix is empty, or if str ends with suffix
static bool issuffixfor(const SkString& suffix, const char str[]) {
    size_t  suffixLen = suffix.size();
    size_t  strLen = strlen(str);

    return  strLen >= suffixLen &&
            memcmp(suffix.c_str(), str + strLen - suffixLen, suffixLen) == 0;
}

bool SkOSFile::Iter::next(SkString* name, bool getDir) {
    SkOSFileIterData& self = *static_cast<SkOSFileIterData*>(fSelf.get());
    if (self.fDIR) {
        dirent* entry;

        while ((entry = ::readdir(self.fDIR)) != nullptr) {
            struct stat s;
            SkString str(self.fPath);

            if (!str.endsWith("/") && !str.endsWith("\\")) {
                str.append("/");
            }
            str.append(entry->d_name);

            if (0 == stat(str.c_str(), &s)) {
                if (getDir) {
                    if (s.st_mode & S_IFDIR) {
                        break;
                    }
                } else {
                    if (!(s.st_mode & S_IFDIR) && issuffixfor(self.fSuffix, entry->d_name)) {
                        break;
                    }
                }
            }
        }
        if (entry) { // we broke out with a file
            if (name) {
                name->set(entry->d_name);
            }
            return true;
        }
    }
    return false;
}
