/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOSFile.h"
#include "SkString.h"
#include "SkTFitsIn.h"
#include "SkTemplates.h"
#include "SkTypes.h"

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

bool sk_exists(const char *path, SkFILE_Flags flags) {
    int mode = F_OK;
    if (flags & kRead_SkFILE_Flag) {
        mode |= R_OK;
    }
    if (flags & kWrite_SkFILE_Flag) {
        mode |= W_OK;
    }
    return (0 == access(path, mode));
}

typedef struct {
    dev_t dev;
    ino_t ino;
} SkFILEID;

static bool sk_ino(SkFILE* a, SkFILEID* id) {
    int fd = fileno((FILE*)a);
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

bool sk_fidentical(SkFILE* a, SkFILE* b) {
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
        return NULL;
    }
    if (!S_ISREG(status.st_mode)) {
        return NULL;
    }
    if (!SkTFitsIn<size_t>(status.st_size)) {
        return NULL;
    }
    size_t fileSize = static_cast<size_t>(status.st_size);

    void* addr = mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, fd, 0);
    if (MAP_FAILED == addr) {
        return NULL;
    }

    *size = fileSize;
    return addr;
}

int sk_fileno(SkFILE* f) {
    return fileno((FILE*)f);
}

void* sk_fmmap(SkFILE* f, size_t* size) {
    int fd = sk_fileno(f);
    if (fd < 0) {
        return NULL;
    }

    return sk_fdmmap(fd, size);
}

////////////////////////////////////////////////////////////////////////////

struct SkOSFileIterData {
    SkOSFileIterData() : fDIR(0) { }
    DIR* fDIR;
    SkString fPath, fSuffix;
};
SK_COMPILE_ASSERT(sizeof(SkOSFileIterData) <= SkOSFile::Iter::kStorageSize, not_enough_space);

SkOSFile::Iter::Iter() {
    SkNEW_PLACEMENT(fSelf.get(), SkOSFileIterData);
}

SkOSFile::Iter::Iter(const char path[], const char suffix[]) {
    SkNEW_PLACEMENT(fSelf.get(), SkOSFileIterData);
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
        self.fDIR = 0;
    }

    self.fPath.set(path);
    if (path) {
        self.fDIR = ::opendir(path);
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

        while ((entry = ::readdir(self.fDIR)) != NULL) {
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
