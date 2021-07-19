/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontDescriptor_DEFINED
#define SkFontDescriptor_DEFINED

#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/private/SkFixed.h"
#include "include/private/SkNoncopyable.h"
#include "include/private/SkTemplates.h"

class SkFontData {
public:
    /** Makes a copy of the data in 'axis'. */
    SkFontData(std::unique_ptr<SkStreamAsset> stream, int index, const SkFixed* axis, int axisCount)
        : fStream(std::move(stream)), fIndex(index), fAxisCount(axisCount), fAxis(axisCount)
    {
        for (int i = 0; i < axisCount; ++i) {
            fAxis[i] = axis[i];
        }
    }
    SkFontData(std::unique_ptr<SkStreamAsset> stream, SkFontArguments args)
        : fStream(std::move(stream)), fIndex(args.getCollectionIndex())
        , fAxisCount(args.getVariationDesignPosition().coordinateCount)
        , fAxis(args.getVariationDesignPosition().coordinateCount)
    {
        for (int i = 0; i < fAxisCount; ++i) {
            fAxis[i] = SkFloatToFixed(args.getVariationDesignPosition().coordinates[i].value);
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
    bool hasStream() const { return fStream != nullptr; }
    std::unique_ptr<SkStreamAsset> detachStream() { return std::move(fStream); }
    SkStreamAsset* getStream() { return fStream.get(); }
    SkStreamAsset const* getStream() const { return fStream.get(); }
    int getIndex() const { return fIndex; }
    int getAxisCount() const { return fAxisCount; }
    const SkFixed* getAxis() const { return fAxis.get(); }

private:
    std::unique_ptr<SkStreamAsset> fStream;
    int fIndex;
    int fAxisCount;
    SkAutoSTMalloc<4, SkFixed> fAxis;
};

class SkFontDescriptor : SkNoncopyable {
public:
    SkFontDescriptor();
    // Does not affect ownership of SkStream.
    static bool Deserialize(SkStream*, SkFontDescriptor* result);

    void serialize(SkWStream*) const;

    SkFontStyle getStyle() const { return fStyle; }
    void setStyle(SkFontStyle style) { fStyle = style; }

    const char* getFamilyName() const { return fFamilyName.c_str(); }
    const char* getFullName() const { return fFullName.c_str(); }
    const char* getPostscriptName() const { return fPostscriptName.c_str(); }

    void setFamilyName(const char* name) { fFamilyName.set(name); }
    void setFullName(const char* name) { fFullName.set(name); }
    void setPostscriptName(const char* name) { fPostscriptName.set(name); }

    bool hasStream() const { return bool(fStream); }
    std::unique_ptr<SkStreamAsset> dupStream() const { return fStream->duplicate(); }
    int getCollectionIndex() const { return fCollectionIndex; }
    int getVariationCoordinateCount() const { return fCoordinateCount; }
    const SkFontArguments::VariationPosition::Coordinate* getVariation() const {
        return fVariation.get();
    }

    std::unique_ptr<SkStreamAsset> detachStream() { return std::move(fStream); }
    void setStream(std::unique_ptr<SkStreamAsset> stream) { fStream = std::move(stream); }
    void setCollectionIndex(int collectionIndex) { fCollectionIndex = collectionIndex; }
    SkFontArguments::VariationPosition::Coordinate* setVariationCoordinates(int coordinateCount) {
        fCoordinateCount = coordinateCount;
        return fVariation.reset(coordinateCount);
    }

private:
    SkString fFamilyName;
    SkString fFullName;
    SkString fPostscriptName;
    SkFontStyle fStyle;

    std::unique_ptr<SkStreamAsset> fStream;
    int fCollectionIndex = 0;
    using Coordinates = SkAutoSTMalloc<4, SkFontArguments::VariationPosition::Coordinate>;
    int fCoordinateCount = 0;
    Coordinates fVariation;
};

#endif // SkFontDescriptor_DEFINED
