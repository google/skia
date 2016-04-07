/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontDescriptor_DEFINED
#define SkFontDescriptor_DEFINED

#include "SkFixed.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkTypeface.h"

class SkFontData {
public:
    /** This takes ownership of 'stream'. Makes a copy of the data in 'axis'. */
    SkFontData(SkStreamAsset* stream, int index, const SkFixed axis[], int axisCount)
        : fStream(stream), fIndex(index), fAxisCount(axisCount), fAxis(axisCount)
    {
        for (int i = 0; i < axisCount; ++i) {
            fAxis[i] = axis[i];
        }
    }
    SkFontData(const SkFontData& that)
        : fStream(that.fStream->duplicate())
        , fIndex(that.fIndex)
        , fAxisCount(that.fAxisCount)
        , fAxis(fAxisCount)
    {
        for (int i = 0; i < fAxisCount; ++i) {
            fAxis[i] = that.fAxis[i];
        }
    }
    bool hasStream() const { return fStream.get() != nullptr; }
    SkStreamAsset* duplicateStream() const { return fStream->duplicate(); }
    SkStreamAsset* detachStream() { return fStream.release(); }
    SkStreamAsset* getStream() { return fStream.get(); }
    int getIndex() const { return fIndex; }
    int getAxisCount() const { return fAxisCount; }
    const SkFixed* getAxis() const { return fAxis.get(); }

private:
    SkAutoTDelete<SkStreamAsset> fStream;
    int fIndex;
    int fAxisCount;
    SkAutoSTMalloc<4, SkFixed> fAxis;
};

class SkFontDescriptor : SkNoncopyable {
public:
    SkFontDescriptor(SkTypeface::Style = SkTypeface::kNormal);
    // Does not affect ownership of SkStream.
    static bool Deserialize(SkStream*, SkFontDescriptor* result);

    void serialize(SkWStream*);

    SkTypeface::Style getStyle() { return fStyle; }
    void setStyle(SkTypeface::Style style) { fStyle = style; }

    const char* getFamilyName() const { return fFamilyName.c_str(); }
    const char* getFullName() const { return fFullName.c_str(); }
    const char* getPostscriptName() const { return fPostscriptName.c_str(); }
    bool hasFontData() const { return fFontData.get() != nullptr; }
    SkFontData* detachFontData() { return fFontData.release(); }

    void setFamilyName(const char* name) { fFamilyName.set(name); }
    void setFullName(const char* name) { fFullName.set(name); }
    void setPostscriptName(const char* name) { fPostscriptName.set(name); }
    /** Set the font data only if it is necessary for serialization.
     *  This method takes ownership of the font data. */
    void setFontData(SkFontData* data) { fFontData.reset(data); }

private:
    SkString fFamilyName;
    SkString fFullName;
    SkString fPostscriptName;
    SkAutoTDelete<SkFontData> fFontData;

    SkTypeface::Style fStyle;
};

#endif // SkFontDescriptor_DEFINED
