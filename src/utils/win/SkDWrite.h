/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDWrite_DEFINED
#define SkDWrite_DEFINED

#include "SkTemplates.h"

#include <dwrite.h>
#include <winsdkver.h>

class SkString;

////////////////////////////////////////////////////////////////////////////////
// Factory

#ifndef SK_HAS_DWRITE_1_H
#define SK_HAS_DWRITE_1_H (WINVER_MAXVER >= 0x0602)
#endif

IDWriteFactory* sk_get_dwrite_factory();

////////////////////////////////////////////////////////////////////////////////
// String conversion

/** Prefer to use this type to prevent template proliferation. */
typedef SkAutoSTMalloc<16, WCHAR> SkSMallocWCHAR;

/** Converts a utf8 string to a WCHAR string. */
HRESULT sk_cstring_to_wchar(const char* skname, SkSMallocWCHAR* name);

/** Converts a WCHAR string to a utf8 string.
 *  @param nameLen the number of WCHARs in the name.
 */
HRESULT sk_wchar_to_skstring(WCHAR* name, int nameLen, SkString* skname);

////////////////////////////////////////////////////////////////////////////////
// Locale

void sk_get_locale_string(IDWriteLocalizedStrings* names, const WCHAR* preferedLocale,
                       SkString* skname);

typedef int (WINAPI *SkGetUserDefaultLocaleNameProc)(LPWSTR, int);
HRESULT SkGetGetUserDefaultLocaleNameProc(SkGetUserDefaultLocaleNameProc* proc);

////////////////////////////////////////////////////////////////////////////////
// Table handling

class AutoDWriteTable {
public:
    AutoDWriteTable(IDWriteFontFace* fontFace, UINT32 beTag) : fFontFace(fontFace), fExists(FALSE) {
        // Any errors are ignored, user must check fExists anyway.
        fontFace->TryGetFontTable(beTag,
            reinterpret_cast<const void **>(&fData), &fSize, &fLock, &fExists);
    }
    ~AutoDWriteTable() {
        if (fExists) {
            fFontFace->ReleaseFontTable(fLock);
        }
    }

    const uint8_t* fData;
    UINT32 fSize;
    BOOL fExists;
private:
    // Borrowed reference, the user must ensure the fontFace stays alive.
    IDWriteFontFace* fFontFace;
    void* fLock;
};
template<typename T> class AutoTDWriteTable : public AutoDWriteTable {
public:
    static const UINT32 tag = DWRITE_MAKE_OPENTYPE_TAG(T::TAG0, T::TAG1, T::TAG2, T::TAG3);
    AutoTDWriteTable(IDWriteFontFace* fontFace) : AutoDWriteTable(fontFace, tag) { }

    const T* get() const { return reinterpret_cast<const T*>(fData); }
    const T* operator->() const { return reinterpret_cast<const T*>(fData); }
};

#endif
