/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontDescriptor_DEFINED
#define SkFontDescriptor_DEFINED

#include "SkString.h"
#include "SkTypeface.h"

class SkStream;
class SkWStream;

class SkFontDescriptor {
public:
    SkFontDescriptor(SkTypeface::Style = SkTypeface::kNormal);
    SkFontDescriptor(SkStream*);

    void serialize(SkWStream*);

    SkTypeface::Style getStyle() { return fStyle; }
    void setStyle(SkTypeface::Style style) { fStyle = style; }

    const char* getFamilyName() { return fFamilyName.c_str(); }
    const char* getFullName() { return fFullName.c_str(); }
    const char* getPostscriptName() { return fPostscriptName.c_str(); }
    const char* getFontFileName() { return fFontFileName.c_str(); }
    
    void setFamilyName(const char* name) { fFamilyName.set(name); }
    void setFullName(const char* name) { fFullName.set(name); }
    void setPostscriptName(const char* name) { fPostscriptName.set(name); }
    void setFontFileName(const char* name) { fFontFileName.set(name); }

    // legacy naming
    SkTypeface::Style getFontStyle() { return this->getStyle(); }
    void setFontStyle(SkTypeface::Style style) { this->setStyle(style); }
    const char* getFontFamilyName() { return this->getFamilyName(); }
    void setFontFamilyName(const char* name) { this->setFamilyName(name); }

private:
    SkString fFamilyName;
    SkString fFullName;
    SkString fPostscriptName;
    SkString fFontFileName;

    SkTypeface::Style fStyle;
};

#endif // SkFontDescriptor_DEFINED
