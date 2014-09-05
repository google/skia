/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOSFile.h"

#include "SkTFitsIn.h"

#include <io.h>
#include <stdio.h>
#include <sys/stat.h>

bool sk_exists(const char *path, SkFILE_Flags flags) {
    int mode = 0; // existence
    if (flags & kRead_SkFILE_Flag) {
        mode |= 4; // read
    }
    if (flags & kWrite_SkFILE_Flag) {
        mode |= 2; // write
    }
    return (0 == _access(path, mode));
}

typedef struct {
    ULONGLONG fVolume;
    ULONGLONG fLsbSize;
    ULONGLONG fMsbSize;
} SkFILEID;

static bool sk_ino(SkFILE* f, SkFILEID* id) {
    int fileno = _fileno((FILE*)f);
    if (fileno < 0) {
        return false;
    }

    HANDLE file = (HANDLE)_get_osfhandle(fileno);
    if (INVALID_HANDLE_VALUE == file) {
        return false;
    }

    //TODO: call GetFileInformationByHandleEx on Vista and later with FileIdInfo.
    BY_HANDLE_FILE_INFORMATION info;
    if (0 == GetFileInformationByHandle(file, &info)) {
        return false;
    }
    id->fVolume = info.dwVolumeSerialNumber;
    id->fLsbSize = info.nFileIndexLow + (((ULONGLONG)info.nFileIndexHigh) << 32);
    id->fMsbSize = 0;

    return true;
}

bool sk_fidentical(SkFILE* a, SkFILE* b) {
    SkFILEID aID, bID;
    return sk_ino(a, &aID) && sk_ino(b, &bID)
           && aID.fLsbSize == bID.fLsbSize
           && aID.fMsbSize == bID.fMsbSize
           && aID.fVolume == bID.fVolume;
}

class SkAutoNullKernelHandle : SkNoncopyable {
public:
    SkAutoNullKernelHandle(const HANDLE handle) : fHandle(handle) { }
    ~SkAutoNullKernelHandle() { CloseHandle(fHandle); }
    operator HANDLE() const { return fHandle; }
    bool isValid() const { return SkToBool(fHandle); }
private:
    HANDLE fHandle;
};
typedef SkAutoNullKernelHandle SkAutoWinMMap;

void sk_fmunmap(const void* addr, size_t) {
    UnmapViewOfFile(addr);
}

void* sk_fdmmap(int fileno, size_t* length) {
    HANDLE file = (HANDLE)_get_osfhandle(fileno);
    if (INVALID_HANDLE_VALUE == file) {
        return NULL;
    }

    LARGE_INTEGER fileSize;
    if (0 == GetFileSizeEx(file, &fileSize)) {
        //TODO: use SK_TRACEHR(GetLastError(), "Could not get file size.") to report.
        return NULL;
    }
    if (!SkTFitsIn<size_t>(fileSize.QuadPart)) {
        return NULL;
    }

    SkAutoWinMMap mmap(CreateFileMapping(file, NULL, PAGE_READONLY, 0, 0, NULL));
    if (!mmap.isValid()) {
        //TODO: use SK_TRACEHR(GetLastError(), "Could not create file mapping.") to report.
        return NULL;
    }

    // Eventually call UnmapViewOfFile
    void* addr = MapViewOfFile(mmap, FILE_MAP_READ, 0, 0, 0);
    if (NULL == addr) {
        //TODO: use SK_TRACEHR(GetLastError(), "Could not map view of file.") to report.
        return NULL;
    }

    *length = static_cast<size_t>(fileSize.QuadPart);
    return addr;
}

int sk_fileno(SkFILE* f) {
    return _fileno((FILE*)f);
}

void* sk_fmmap(SkFILE* f, size_t* length) {
    int fileno = sk_fileno(f);
    if (fileno < 0) {
        return NULL;
    }

    return sk_fdmmap(fileno, length);
}
