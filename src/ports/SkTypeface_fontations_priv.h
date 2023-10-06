/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTypeface_Fontations_priv_DEFINED
#define SkTypeface_Fontations_priv_DEFINED

#include "include/core/SkFontParameters.h"
#include "include/core/SkPath.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "include/private/base/SkOnce.h"
#include "src/core/SkAdvancedTypefaceMetrics.h"
#include "src/core/SkScalerContext.h"
#include "src/ports/fontations/src/ffi.rs.h"

#include <memory>

class SkStreamAsset;

namespace sk_fontations {

/** Implementation of PathWrapper FFI C++ interface which allows Rust to call back
 * into C++ without exposing Skia types on the interface, see skpath_bridge.h. */
class PathGeometrySink : public fontations_ffi::PathWrapper {
public:
    /* From fontations_ffi::PathWrapper. */
    void move_to(float x, float y) override;
    void line_to(float x, float y) override;
    void quad_to(float cx0, float cy0, float x, float y) override;
    void curve_to(float cx0, float cy0, float cx1, float cy1, float x, float y) override;
    void close() override;

    SkPath into_inner() &&;

private:
    void going_to(SkPoint point);
    bool current_is_not(SkPoint);

    SkPath fPath;
    bool fStarted;
    SkPoint fCurrent;
};

/** Implementation of AxisWrapper FFI C++ interface, allowing Rust to call back into
 * C++ for populating variable axis availability information, see skpath_bridge.h. */
class AxisWrapper : public fontations_ffi::AxisWrapper {
public:
    AxisWrapper(SkFontParameters::Variation::Axis axisArray[], size_t axisCount);
    AxisWrapper() = delete;
    /* From fontations_ffi::AxisWrapper. */
    bool populate_axis(
            size_t i, uint32_t axisTag, float min, float def, float max, bool hidden) override;
    size_t size() const override;

private:
    SkFontParameters::Variation::Axis* fAxisArray;
    size_t fAxisCount;
};

}  // namespace sk_fontations

/** SkTypeface implementation based on Google Fonts Fontations Rust libraries. */
class SkTypeface_Fontations : public SkTypeface {
public:
    SkTypeface_Fontations(sk_sp<SkData> fontData, const SkFontArguments&);

    bool hasValidBridgeFontRef() const;
    const fontations_ffi::BridgeFontRef& getBridgeFontRef() { return *fBridgeFontRef; }
    const fontations_ffi::BridgeNormalizedCoords& getBridgeNormalizedCoords() {
        return *fBridgeNormalizedCoords;
    }

    static constexpr SkTypeface::FactoryId FactoryId = SkSetFourByteTag('f', 'n', 't', 'a');

    static sk_sp<SkTypeface> MakeFromData(sk_sp<SkData> fontData, const SkFontArguments&);
    static sk_sp<SkTypeface> MakeFromStream(std::unique_ptr<SkStreamAsset>, const SkFontArguments&);

protected:
    std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override;
    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override;
    std::unique_ptr<SkScalerContext> onCreateScalerContext(const SkScalerContextEffects& effects,
                                                           const SkDescriptor* desc) const override;
    void onFilterRec(SkScalerContextRec*) const override;
    std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const override {
        return nullptr;
    }
    void onGetFontDescriptor(SkFontDescriptor*, bool*) const override;
    void onCharsToGlyphs(const SkUnichar* chars, int count, SkGlyphID glyphs[]) const override;
    int onCountGlyphs() const override;
    void getPostScriptGlyphNames(SkString*) const override {}
    void getGlyphToUnicodeMap(SkUnichar*) const override {}
    int onGetUPEM() const override;
    void onGetFamilyName(SkString* familyName) const override;
    bool onGetPostScriptName(SkString*) const override;
    SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const override;
    bool onGlyphMaskNeedsCurrentColor() const override;
    int onGetVariationDesignPosition(SkFontArguments::VariationPosition::Coordinate coordinates[],
                                     int coordinateCount) const override;
    int onGetVariationDesignParameters(SkFontParameters::Variation::Axis parameters[],
                                       int parameterCount) const override;
    int onGetTableTags(SkFontTableTag tags[]) const override;
    size_t onGetTableData(SkFontTableTag, size_t, size_t, void*) const override;

private:
    sk_sp<SkData> fFontData;
    // Incoming ttc index requested when this typeface was instantiated from data.
    uint32_t fTtcIndex = 0;
    // fBridgeFontRef accesses the data in fFontData. fFontData needs to be kept around for the
    // lifetime of fBridgeFontRef to safely request parsed data.
    rust::Box<fontations_ffi::BridgeFontRef> fBridgeFontRef;
    rust::Box<fontations_ffi::BridgeNormalizedCoords> fBridgeNormalizedCoords;

    mutable SkOnce fGlyphMasksMayNeedCurrentColorOnce;
    mutable bool fGlyphMasksMayNeedCurrentColor;
};

#endif  // SkTypeface_Fontations_DEFINED
