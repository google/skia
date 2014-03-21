/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTemplates.h"

#include <dwrite.h>

class SkString;

////////////////////////////////////////////////////////////////////////////////
// Factory

IDWriteFactory* sk_get_dwrite_factory();

////////////////////////////////////////////////////////////////////////////////
// String conversion

/** Prefer to use this type to prevent template proliferation. */
typedef SkAutoSTMalloc<16, WCHAR> SkSMallocWCHAR;

/** Converts a utf8 string to a WCHAR string. */
HRESULT sk_cstring_to_wchar(const char* skname, SkSMallocWCHAR* name);

/** Converts a WCHAR string to a utf8 string. */
HRESULT sk_wchar_to_skstring(WCHAR* name, SkString* skname);

////////////////////////////////////////////////////////////////////////////////
// Locale

void sk_get_locale_string(IDWriteLocalizedStrings* names, const WCHAR* preferedLocale,
                       SkString* skname);

typedef decltype(GetUserDefaultLocaleName)* SkGetUserDefaultLocaleNameProc;
HRESULT SkGetGetUserDefaultLocaleNameProc(SkGetUserDefaultLocaleNameProc* proc);
