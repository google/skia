/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontDescriptor_DEFINED
#define SkFontDescriptor_DEFINED

#include "include/core/SkFontArguments.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/private/base/SkFixed.h"
#include "include/private/base/SkNoncopyable.h"
#include "include/private/base/SkTemplates.h"

#include <memory>
#include <utility>

class SkFontData {
public:
    /** Makes a copy of the data in 'axis'. */
    SkFontData(std::unique_ptr<SkStreamAsset> stream, int index, int paletteIndex,
               const SkFixed* axis, int axisCount,
               const SkFontArguments::Palette::Override* paletteOverrides, int paletteOverrideCount)
        : fStream(std::move(stream))
        , fIndex(index)
        , fPaletteIndex(paletteIndex)
        , fAxisCount(axisCount)
        , fPaletteOverrideCount(paletteOverrideCount)
        , fAxis(fAxisCount)
        , fPaletteOverrides(fPaletteOverrideCount)
    {
        for (int i = 0; i < fAxisCount; ++i) {
            fAxis[i] = axis[i];
        }
        for (int i = 0; i < fPaletteOverrideCount; ++i) {
            fPaletteOverrides[i] = paletteOverrides[i];
        }
    }

    SkFontData(const SkFontData& that)
        : fStream(that.fStream->duplicate())
        , fIndex(that.fIndex)
        , fPaletteIndex(that.fPaletteIndex)
        , fAxisCount(that.fAxisCount)
        , fPaletteOverrideCount(that.fPaletteOverrideCount)
        , fAxis(fAxisCount)
        , fPaletteOverrides(fPaletteOverrideCount)
    {
        for (int i = 0; i < fAxisCount; ++i) {
            fAxis[i] = that.fAxis[i];
        }
        for (int i = 0; i < fPaletteOverrideCount; ++i) {
            fPaletteOverrides[i] = that.fPaletteOverrides[i];
        }
    }
    bool hasStream() const { return fStream != nullptr; }
    std::unique_ptr<SkStreamAsset> detachStream() { return std::move(fStream); }
    SkStreamAsset* getStream() { return fStream.get(); }
    SkStreamAsset const* getStream() const { return fStream.get(); }
    int getIndex() const { return fIndex; }
    int getAxisCount() const { return fAxisCount; }
    const SkFixed* getAxis() const { return fAxis.get(); }
    int getPaletteIndex() const { return fPaletteIndex; }
    int getPaletteOverrideCount() const { return fPaletteOverrideCount; }
    const SkFontArguments::Palette::Override* getPaletteOverrides() const {
        return fPaletteOverrides.get();
    }

private:
    std::unique_ptr<SkStreamAsset> fStream;
    int fIndex;
    int fPaletteIndex;
    int fAxisCount;
    int fPaletteOverrideCount;
    skia_private::AutoSTMalloc<4, SkFixed> fAxis;
    skia_private::AutoSTMalloc<4, SkFontArguments::Palette::Override> fPaletteOverrides;
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
    int getPaletteIndex() const { return fPaletteIndex; }
    int getVariationCoordinateCount() const { return fCoordinateCount; }
    const SkFontArguments::VariationPosition::Coordinate* getVariation() const {
        return fVariation.get();
    }
    int getPaletteEntryOverrideCount() const { return fPaletteEntryOverrideCount; }
    const SkFontArguments::Palette::Override* getPaletteEntryOverrides() const {
        return fPaletteEntryOverrides.get();
    }
    SkTypeface::FactoryId getFactoryId() {
        return fFactoryId;
    }

    std::unique_ptr<SkStreamAsset> detachStream() { return std::move(fStream); }
    void setStream(std::unique_ptr<SkStreamAsset> stream) { fStream = std::move(stream); }
    void setCollectionIndex(int collectionIndex) { fCollectionIndex = collectionIndex; }
    void setPaletteIndex(int paletteIndex) { fPaletteIndex = paletteIndex; }
    SkFontArguments::VariationPosition::Coordinate* setVariationCoordinates(int coordinateCount) {
        fCoordinateCount = coordinateCount;
        return fVariation.reset(coordinateCount);
    }
    SkFontArguments::Palette::Override* setPaletteEntryOverrides(int paletteEntryOverrideCount) {
        fPaletteEntryOverrideCount = paletteEntryOverrideCount;
        return fPaletteEntryOverrides.reset(paletteEntryOverrideCount);
    }
    void setFactoryId(SkTypeface::FactoryId factoryId) {
        fFactoryId = factoryId;
    }

    SkFontArguments getFontArguments() const {
        return SkFontArguments()
            .setCollectionIndex(this->getCollectionIndex())
            .setVariationDesignPosition({this->getVariation(),this->getVariationCoordinateCount()})
            .setPalette({this->getPaletteIndex(),
                         this->getPaletteEntryOverrides(),
                         this->getPaletteEntryOverrideCount()});
    }
    static SkFontStyle::Width SkFontStyleWidthForWidthAxisValue(SkScalar width);

private:
    SkString fFamilyName;
    SkString fFullName;
    SkString fPostscriptName;
    SkFontStyle fStyle;

    std::unique_ptr<SkStreamAsset> fStream;
    int fCollectionIndex = 0;
    using Coordinates =
            skia_private::AutoSTMalloc<4, SkFontArguments::VariationPosition::Coordinate>;
    int fCoordinateCount = 0;
    Coordinates fVariation;
    int fPaletteIndex = 0;
    int fPaletteEntryOverrideCount = 0;
    skia_private::AutoTMalloc<SkFontArguments::Palette::Override> fPaletteEntryOverrides;
    SkTypeface::FactoryId fFactoryId = 0;
};

#endif // SkFontDescriptor_DEFINED
