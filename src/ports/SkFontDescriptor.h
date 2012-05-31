/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

///////////////////////////////////////////////////////////////////////////////

#ifndef SkFontDescriptor_DEFINED
#define SkFontDescriptor_DEFINED

#include "SkString.h"
#include "SkTypeface.h"

class SkStream;
class SkWStream;

class SkFontDescriptor {
public:
    SkFontDescriptor();
    SkFontDescriptor(SkStream*);

    void serialize(SkWStream*);

    void setFontFamilyName(const char* name) { fFontFamilyName.set(name); }
    void setFontStyle(SkTypeface::Style style) { fFontStyle = style; }
    void setFontFileName(const char* name) { fFontFileName.set(name); }

    const char* getFontFamilyName() { return fFontFamilyName.c_str(); }
    SkTypeface::Style getFontStyle() { return fFontStyle; }
    const char* getFontFileName() { return fFontFileName.c_str(); }

private:
    SkString fFontFamilyName;
    SkTypeface::Style fFontStyle;
    SkString fFontFileName;
};

#endif // SkFontDescriptor_DEFINED
