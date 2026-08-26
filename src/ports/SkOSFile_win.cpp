/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_WIN)

#include "include/private/SkMalloc.h"
#include "include/private/SkNoncopyable.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTFitsIn.h"
#include "src/core/SkLeanWindows.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkStringUtils.h"

#include <io.h>
#include <new>
#include <stdio.h>
#include <sys/stat.h>

void sk_fsync(FILE* f) {
    _commit(sk_fileno(f));
}

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

static bool sk_ino(FILE* f, SkFILEID* id) {
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

bool sk_fidentical(FILE* a, FILE* b) {
    SkFILEID aID, bID;
    return sk_ino(a, &aID) && sk_ino(b, &bID)
           && aID.fLsbSize == bID.fLsbSize
           && aID.fMsbSize == bID.fMsbSize
           && aID.fVolume == bID.fVolume;
}

class [[nodiscard]] SkAutoNullKernelHandle : SkNoncopyable {
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
        return nullptr;
    }

    LARGE_INTEGER fileSize;
    if (0 == GetFileSizeEx(file, &fileSize)) {
        //TODO: use SK_TRACEHR(GetLastError(), "Could not get file size.") to report.
        return nullptr;
    }
    if (!SkTFitsIn<size_t>(fileSize.QuadPart)) {
        return nullptr;
    }

    SkAutoWinMMap mmap(CreateFileMapping(file, nullptr, PAGE_READONLY, 0, 0, nullptr));
    if (!mmap.isValid()) {
        //TODO: use SK_TRACEHR(GetLastError(), "Could not create file mapping.") to report.
        return nullptr;
    }

    // Eventually call UnmapViewOfFile
    void* addr = MapViewOfFile(mmap, FILE_MAP_READ, 0, 0, 0);
    if (nullptr == addr) {
        //TODO: use SK_TRACEHR(GetLastError(), "Could not map view of file.") to report.
        return nullptr;
    }

    *length = static_cast<size_t>(fileSize.QuadPart);
    return addr;
}

int sk_fileno(FILE* f) {
    return _fileno((FILE*)f);
}

void* sk_fmmap(FILE* f, size_t* length) {
    int fileno = sk_fileno(f);
    if (fileno < 0) {
        return nullptr;
    }

    return sk_fdmmap(fileno, length);
}

size_t sk_qread(FILE* file, void* buffer, size_t count, size_t offset) {
    int fileno = sk_fileno(file);
    HANDLE fileHandle = (HANDLE)_get_osfhandle(fileno);
    if (INVALID_HANDLE_VALUE == file) {
        return SIZE_MAX;
    }

    OVERLAPPED overlapped;
    memset(&overlapped, 0, sizeof(overlapped));
    ULARGE_INTEGER winOffset;
    winOffset.QuadPart = offset;
    overlapped.Offset = winOffset.LowPart;
    overlapped.OffsetHigh = winOffset.HighPart;

    if (!SkTFitsIn<DWORD>(count)) {
        count = std::numeric_limits<DWORD>::max();
    }

    DWORD bytesRead;
    if (ReadFile(fileHandle, buffer, static_cast<DWORD>(count), &bytesRead, &overlapped)) {
        return bytesRead;
    }
    if (GetLastError() == ERROR_HANDLE_EOF) {
        return 0;
    }
    return SIZE_MAX;
}

////////////////////////////////////////////////////////////////////////////

struct SkOSFileIterData {
    HANDLE fHandle = 0;
    skia_private::TArray<uint16_t> fPath16;
};
static_assert(sizeof(SkOSFileIterData) <= SkOSFile::Iter::kStorageSize, "not_enough_space");

// Builds a UTF-16 wildcard search pattern for FindFirstFileW.
// e.g. path = "C:/my_dir", suffix = ".png" -> "C:/my_dir/*.png"
static skia_private::TArray<uint16_t> make_utf16_search_pattern(const char path[],
                                                                const char suffix[]) {
    if (!path) {
        path = "";
    }
    if (!suffix) {
        suffix = "";
    }

    skia_private::TArray<uint16_t> dst;
    while (*path) {
        dst.push_back(static_cast<uint16_t>(*path++));
    }
    if (!dst.empty() && dst.back() != '/') {
        dst.push_back('/');
    }
    dst.push_back('*');
    while (*suffix) {
        dst.push_back(static_cast<uint16_t>(*suffix++));
    }
    dst.push_back(0);
    return dst;
}

SkOSFile::Iter::Iter() { new (fSelf) SkOSFileIterData; }

SkOSFile::Iter::Iter(const char path[], const char suffix[]) {
    new (fSelf) SkOSFileIterData;
    this->reset(path, suffix);
}

SkOSFile::Iter::~Iter() {
    SkOSFileIterData& self = *reinterpret_cast<SkOSFileIterData*>(fSelf);
    if (self.fHandle) {
        ::FindClose(self.fHandle);
    }
    self.~SkOSFileIterData();
}

void SkOSFile::Iter::reset(const char path[], const char suffix[]) {
    SkOSFileIterData& self = *reinterpret_cast<SkOSFileIterData*>(fSelf);
    if (self.fHandle) {
        ::FindClose(self.fHandle);
        self.fHandle = 0;
    }
    self.fPath16 = make_utf16_search_pattern(path, suffix);
}

static bool is_magic_dir(const uint16_t dir[]) {
    // return true for "." and ".."
    return dir[0] == '.' && (dir[1] == 0 || (dir[1] == '.' && dir[2] == 0));
}

static bool get_the_file(HANDLE handle, SkString* name, WIN32_FIND_DATAW* dataPtr, bool getDir) {
    WIN32_FIND_DATAW    data;

    if (nullptr == dataPtr) {
        if (::FindNextFileW(handle, &data))
            dataPtr = &data;
        else
            return false;
    }

    for (;;) {
        if (getDir) {
            if ((dataPtr->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                !is_magic_dir((uint16_t*)dataPtr->cFileName))
            {
                break;
            }
        } else {
            if (!(dataPtr->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                break;
            }
        }
        if (!::FindNextFileW(handle, dataPtr)) {
            return false;
        }
    }
    // if we get here, we've found a file/dir
    if (name) {
        const uint16_t* utf16name = (const uint16_t*)dataPtr->cFileName;
        const uint16_t* ptr = utf16name;
        while (*ptr != 0) { ++ptr; }
        *name = SkStringFromUTF16(utf16name, ptr - utf16name);
    }
    return true;
}

bool SkOSFile::Iter::next(SkString* name, bool getDir) {
    SkOSFileIterData& self = *reinterpret_cast<SkOSFileIterData*>(fSelf);
    WIN32_FIND_DATAW    data;
    WIN32_FIND_DATAW*   dataPtr = nullptr;

    if (self.fHandle == 0) {  // our first time
        if (self.fPath16.empty()) {  // check for no path
            return false;
        }

        self.fHandle = ::FindFirstFileW((LPCWSTR)self.fPath16.data(), &data);
        if (self.fHandle != 0 && self.fHandle != (HANDLE)~0) {
            dataPtr = &data;
        }
    }
    return self.fHandle != (HANDLE)~0 && get_the_file(self.fHandle, name, dataPtr, getDir);
}

#endif//defined(SK_BUILD_FOR_WIN)
