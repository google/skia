/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_WIN)

#include "include/core/SkString.h"
#include "include/private/SkOnce.h"
#include "src/utils/win/SkDWrite.h"
#include "src/utils/win/SkHRESULT.h"

#include <dwrite.h>

#if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wcast-function-type"
#endif

static IDWriteFactory* gDWriteFactory = nullptr;

static void release_dwrite_factory() {
    if (gDWriteFactory) {
        gDWriteFactory->Release();
    }
}

static void create_dwrite_factory(IDWriteFactory** factory) {
    typedef decltype(DWriteCreateFactory)* DWriteCreateFactoryProc;
    DWriteCreateFactoryProc dWriteCreateFactoryProc = reinterpret_cast<DWriteCreateFactoryProc>(
        GetProcAddress(LoadLibraryW(L"dwrite.dll"), "DWriteCreateFactory"));

    if (!dWriteCreateFactoryProc) {
        HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
        if (!IS_ERROR(hr)) {
            hr = ERROR_PROC_NOT_FOUND;
        }
        HRVM(hr, "Could not get DWriteCreateFactory proc.");
    }

    HRVM(dWriteCreateFactoryProc(DWRITE_FACTORY_TYPE_SHARED,
                                 __uuidof(IDWriteFactory),
                                 reinterpret_cast<IUnknown**>(factory)),
         "Could not create DirectWrite factory.");
    atexit(release_dwrite_factory);
}


IDWriteFactory* sk_get_dwrite_factory() {
    static SkOnce once;
    once(create_dwrite_factory, &gDWriteFactory);
    return gDWriteFactory;
}

////////////////////////////////////////////////////////////////////////////////
// String conversion

/** Converts a utf8 string to a WCHAR string. */
HRESULT sk_cstring_to_wchar(const char* skname, SkSMallocWCHAR* name) {
    int wlen = MultiByteToWideChar(CP_UTF8, 0, skname, -1, nullptr, 0);
    if (0 == wlen) {
        HRM(HRESULT_FROM_WIN32(GetLastError()),
            "Could not get length for wchar to utf-8 conversion.");
    }
    name->reset(wlen);
    wlen = MultiByteToWideChar(CP_UTF8, 0, skname, -1, name->get(), wlen);
    if (0 == wlen) {
        HRM(HRESULT_FROM_WIN32(GetLastError()), "Could not convert wchar to utf-8.");
    }
    return S_OK;
}

/** Converts a WCHAR string to a utf8 string. */
HRESULT sk_wchar_to_skstring(WCHAR* name, int nameLen, SkString* skname) {
    int len = WideCharToMultiByte(CP_UTF8, 0, name, nameLen, nullptr, 0, nullptr, nullptr);
    if (0 == len) {
        if (nameLen <= 0) {
            skname->reset();
            return S_OK;
        }
        HRM(HRESULT_FROM_WIN32(GetLastError()),
            "Could not get length for utf-8 to wchar conversion.");
    }
    skname->resize(len);

    len = WideCharToMultiByte(CP_UTF8, 0, name, nameLen, skname->writable_str(), len, nullptr, nullptr);
    if (0 == len) {
        HRM(HRESULT_FROM_WIN32(GetLastError()), "Could not convert utf-8 to wchar.");
    }
    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
// Locale

HRESULT sk_get_locale_string(IDWriteLocalizedStrings* names, const WCHAR* preferedLocale,
                             SkString* skname) {
    UINT32 nameIndex = 0;
    if (preferedLocale) {
        // Ignore any errors and continue with index 0 if there is a problem.
        BOOL nameExists = FALSE;
        (void)names->FindLocaleName(preferedLocale, &nameIndex, &nameExists);
        if (!nameExists) {
            nameIndex = 0;
        }
    }

    UINT32 nameLen;
    HRM(names->GetStringLength(nameIndex, &nameLen), "Could not get name length.");

    SkSMallocWCHAR name(nameLen + 1);
    HRM(names->GetString(nameIndex, name.get(), nameLen + 1), "Could not get string.");

    HR(sk_wchar_to_skstring(name.get(), nameLen, skname));
    return S_OK;
}

HRESULT SkGetGetUserDefaultLocaleNameProc(SkGetUserDefaultLocaleNameProc* proc) {
    *proc = reinterpret_cast<SkGetUserDefaultLocaleNameProc>(
        GetProcAddress(LoadLibraryW(L"Kernel32.dll"), "GetUserDefaultLocaleName")
    );
    if (!*proc) {
        HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
        if (!IS_ERROR(hr)) {
            hr = ERROR_PROC_NOT_FOUND;
        }
        return hr;
    }
    return S_OK;
}

#if defined(__clang__)
    #pragma clang diagnostic pop
#endif

#endif//defined(SK_BUILD_FOR_WIN)
