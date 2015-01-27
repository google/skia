/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontDescriptor_DEFINED
#define SkFontDescriptor_DEFINED

#include "SkStream.h"
#include "SkString.h"
#include "SkTypeface.h"

class SkFontDescriptor {
public:
    SkFontDescriptor(SkTypeface::Style = SkTypeface::kNormal);
    // Does not affect ownership of SkStream.
    SkFontDescriptor(SkStream*);

    void serialize(SkWStream*);

    SkTypeface::Style getStyle() { return fStyle; }
    void setStyle(SkTypeface::Style style) { fStyle = style; }

    const char* getFamilyName() const { return fFamilyName.c_str(); }
    const char* getFullName() const { return fFullName.c_str(); }
    const char* getPostscriptName() const { return fPostscriptName.c_str(); }
    const char* getFontFileName() const { return fFontFileName.c_str(); }
    bool hasFontData() const { return fFontData.get() != NULL; }
    // Transfers ownership to the caller.
    SkStreamAsset* transferFontData() { return fFontData.detach(); }
    int getFontIndex() const { return fFontIndex; }

    void setFamilyName(const char* name) { fFamilyName.set(name); }
    void setFullName(const char* name) { fFullName.set(name); }
    void setPostscriptName(const char* name) { fPostscriptName.set(name); }
    void setFontFileName(const char* name) { fFontFileName.set(name); }
    /** Set the font data only if it is necessary for serialization.
     *  This method takes ownership of the stream (both reference and cursor).
     */
    void setFontData(SkStreamAsset* stream) { fFontData.reset(stream); }
    void setFontIndex(int index) { fFontIndex = index; }

private:
    SkString fFamilyName;
    SkString fFullName;
    SkString fPostscriptName;
    SkString fFontFileName;
    SkAutoTDelete<SkStreamAsset> fFontData;
    int fFontIndex;

    SkTypeface::Style fStyle;
};

#endif // SkFontDescriptor_DEFINED
