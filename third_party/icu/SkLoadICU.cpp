// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "SkLoadICU.h"

#if defined(_WIN32) && defined(SK_USING_THIRD_PARTY_ICU)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <io.h>

#include <cstdio>
#include <cstring>
#include <mutex>
#include <string>

#include "unicode/udata.h"

static void* win_mmap(const wchar_t* dataFile) {
    if (!dataFile) {
        return nullptr;
    }
    struct FCloseWrapper { void operator()(FILE* f) { fclose(f); } };
    std::unique_ptr<FILE, FCloseWrapper> stream(_wfopen(dataFile, L"rb"));
    if (!stream) {
        fprintf(stderr, "SkIcuLoader: datafile missing: %ls.\n", dataFile);
        return nullptr;
    }
    int fileno = _fileno(stream.get());
    if (fileno < 0) {
        fprintf(stderr, "SkIcuLoader: datafile fileno error.\n");
        return nullptr;
    }
    HANDLE file = (HANDLE)_get_osfhandle(fileno);
    if ((HANDLE)INVALID_HANDLE_VALUE == file) {
        fprintf(stderr, "SkIcuLoader: datafile handle error.\n");
        return nullptr;
    }
    struct CloseHandleWrapper { void operator()(HANDLE h) { CloseHandle(h); } };
    std::unique_ptr<void, CloseHandleWrapper> mmapHandle(
        CreateFileMapping(file, nullptr, PAGE_READONLY, 0, 0, nullptr));
    if (!mmapHandle) {
        fprintf(stderr, "SkIcuLoader: datafile mmap error.\n");
        return nullptr;
    }
    void* addr = MapViewOfFile(mmapHandle.get(), FILE_MAP_READ, 0, 0, 0);
    if (nullptr == addr) {
        fprintf(stderr, "SkIcuLoader: datafile view error.\n");
        return nullptr;
    }
    return addr;
}

static bool init_icu(void* addr) {
    UErrorCode err = U_ZERO_ERROR;
    udata_setCommonData(addr, &err);
    if (err != U_ZERO_ERROR) {
        fprintf(stderr, "udata_setCommonData() returned %d.\n", (int)err);
        return false;
    }
    udata_setFileAccess(UDATA_ONLY_PACKAGES, &err);
    if (err != U_ZERO_ERROR) {
        fprintf(stderr, "udata_setFileAccess() returned %d.\n", (int)err);
        return false;
    }
    return true;
}

static std::wstring get_module_path(HMODULE module) {
    DWORD len;
    std::wstring path;
    path.resize(MAX_PATH);

    len = GetModuleFileNameW(module, (LPWSTR)path.data(), (DWORD)path.size());
    if (len > path.size()) {
        path.resize(len);
        len = GetModuleFileNameW(module, (LPWSTR)path.data(), (DWORD)path.size());
    }
    path.resize(len);
    std::size_t end = path.rfind('\\');
    if (end == std::wstring::npos) {
        return std::wstring();
    }
    path.resize(end);
    return path;
}

static std::wstring library_directory() {
    HMODULE hModule = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
        reinterpret_cast<LPCSTR>(&library_directory), &hModule);
    return get_module_path(hModule);
}

static std::wstring executable_directory() {
    HMODULE hModule = GetModuleHandleA(NULL);
    return get_module_path(hModule);
}

static bool load_from(const std::wstring& dir) {
    auto sPath = dir + L"\\icudtl.dat";
    if (void* addr = win_mmap(sPath.c_str())) {
        if (init_icu(addr)) {
            return true;
        }
    }
    return false;
}

bool SkLoadICU() {
    static bool good = false;
    static std::once_flag flag;
    std::call_once(flag, []() {
        good = load_from(executable_directory()) || load_from(library_directory());
    });
    return good;
}

#endif  // defined(_WIN32) && defined(SK_USING_THIRD_PARTY_ICU)
