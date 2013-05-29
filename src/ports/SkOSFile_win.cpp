/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOSFile.h"

#include <io.h>
#include <stdio.h>
#include <sys/stat.h>

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

template <typename HandleType, HandleType InvalidValue, BOOL (WINAPI * Close)(HandleType)>
class SkAutoTHandle : SkNoncopyable {
public:
    SkAutoTHandle(HandleType handle) : fHandle(handle) { }
    ~SkAutoTHandle() { Close(fHandle); }
    operator HandleType() { return fHandle; }
    bool isValid() { return InvalidValue != fHandle; }
private:
    HandleType fHandle;
};
typedef SkAutoTHandle<HANDLE, INVALID_HANDLE_VALUE, CloseHandle> SkAutoWinFile;
typedef SkAutoTHandle<HANDLE, NULL, CloseHandle> SkAutoWinMMap;

void sk_fmunmap(const void* addr, size_t) {
    UnmapViewOfFile(addr);
}

void* sk_fmmap(SkFILE* f, size_t* length) {
    size_t fileSize = sk_fgetsize(f);
    if (0 == fileSize) {
        return NULL;
    }

    int fileno = _fileno((FILE*)f);
    if (fileno < 0) {
        return NULL;
    }

    HANDLE file = (HANDLE)_get_osfhandle(fileno);
    if (INVALID_HANDLE_VALUE == file) {
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

    *length = fileSize;
    return addr;
}
