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
class SkFontationsScalerContext;

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

class ColorPainter : public fontations_ffi::ColorPainterWrapper {
public:
    ColorPainter() = delete;
    ColorPainter(SkFontationsScalerContext& scaler_context,
                 SkCanvas& canvas,
                 SkSpan<SkColor> palette,
                 SkColor foregroundColor,
                 uint16_t upem);

    // fontations_ffi::ColorPainter interface.
    virtual void push_transform(const fontations_ffi::Transform& transform) override;
    virtual void pop_transform() override;
    virtual void push_clip_glyph(uint16_t glyph_id) override;
    virtual void push_clip_rectangle(float x_min, float y_min, float x_max, float y_max) override;
    virtual void pop_clip() override;

    // Paint*Gradient equivalents:
    virtual void fill_solid(uint16_t palette_index, float alpha) override;
    virtual void fill_radial(const fontations_ffi::FillRadialParams& fill_radial_params,
                             fontations_ffi::BridgeColorStops&,
                             uint8_t extend_mode) override;
    virtual void fill_linear(const fontations_ffi::FillLinearParams& fill_linear_params,
                             fontations_ffi::BridgeColorStops&,
                             uint8_t extend_mode) override;
    virtual void fill_sweep(const fontations_ffi::FillSweepParams& fill_sweep_params,
                            fontations_ffi::BridgeColorStops&,
                            uint8_t extend_mode) override;

    // Optimized calls that allow a SkCanvas::drawPath() call.
    virtual void fill_glyph_solid(uint16_t glyph_id, uint16_t palette_index, float alpha) override;
    virtual void fill_glyph_radial(uint16_t glyph_id,
                                   const fontations_ffi::Transform& transform,
                                   const fontations_ffi::FillRadialParams& fill_radial_params,
                                   fontations_ffi::BridgeColorStops& stops,
                                   uint8_t) override;
    virtual void fill_glyph_linear(uint16_t glyph_id,
                                   const fontations_ffi::Transform& transform,
                                   const fontations_ffi::FillLinearParams& fill_linear_params,
                                   fontations_ffi::BridgeColorStops& stops,
                                   uint8_t) override;
    virtual void fill_glyph_sweep(uint16_t glyph_id,
                                  const fontations_ffi::Transform& transform,
                                  const fontations_ffi::FillSweepParams& fill_sweep_params,
                                  fontations_ffi::BridgeColorStops& stops,
                                  uint8_t) override;

    // compositeMode arg matches composite mode values from the OpenType COLR table spec.
    virtual void push_layer(uint8_t compositeMode) override;
    virtual void pop_layer() override;

private:
    void configure_solid_paint(uint16_t palette_index, float alpha, SkPaint& paint);
    void configure_linear_paint(const fontations_ffi::FillLinearParams& fill_linear_params,
                                fontations_ffi::BridgeColorStops& bridge_stops,
                                uint8_t extend_mode,
                                SkPaint& paint,
                                SkMatrix* = nullptr);
    void configure_radial_paint(const fontations_ffi::FillRadialParams& fill_radial_params,
                                fontations_ffi::BridgeColorStops& bridge_stops,
                                uint8_t extend_mode,
                                SkPaint& paint,
                                SkMatrix* = nullptr);
    void configure_sweep_paint(const fontations_ffi::FillSweepParams& sweep_params,
                               fontations_ffi::BridgeColorStops& bridge_stops,
                               uint8_t extend_mode,
                               SkPaint& paint,
                               SkMatrix* = nullptr);
    SkFontationsScalerContext& fScalerContext;
    SkCanvas& fCanvas;
    SkSpan<SkColor> fPalette;
    SkColor fForegroundColor;
    uint16_t fUpem;
};

/** Tracks transforms and clips to compute a bounding box without drawing pixels. */
class BoundsPainter : public fontations_ffi::ColorPainterWrapper {
public:
    BoundsPainter() = delete;
    BoundsPainter(SkFontationsScalerContext& scaler_context,
                  SkMatrix initialTransfom,
                  uint16_t upem);

    SkRect getBoundingBox();

    // fontations_ffi::ColorPainter interface.
    virtual void push_transform(const fontations_ffi::Transform& transform) override;
    virtual void pop_transform() override;
    virtual void push_clip_glyph(uint16_t glyph_id) override;
    virtual void push_clip_rectangle(float x_min, float y_min, float x_max, float y_max) override;
    virtual void pop_clip() override {}

    // Paint*Gradient equivalents:
    virtual void fill_solid(uint16_t palette_index, float alpha) override {}
    virtual void fill_radial(const fontations_ffi::FillRadialParams& fill_radial_params,
                             fontations_ffi::BridgeColorStops& stops,
                             uint8_t) override {}
    virtual void fill_linear(const fontations_ffi::FillLinearParams& fill_linear_params,
                             fontations_ffi::BridgeColorStops& stops,
                             uint8_t) override {}
    virtual void fill_sweep(const fontations_ffi::FillSweepParams& fill_sweep_params,
                            fontations_ffi::BridgeColorStops& stops,
                            uint8_t extend_mode) override {}

    virtual void push_layer(uint8_t) override {}
    virtual void pop_layer() override {}

    // Stubs for optimized calls. We're only interested in the glyph bounds, so we forward this to
    // push_clip_glyph()
    virtual void fill_glyph_solid(uint16_t glyph_id, uint16_t, float) override;
    virtual void fill_glyph_radial(uint16_t glyph_id,
                                   const fontations_ffi::Transform&,
                                   const fontations_ffi::FillRadialParams&,
                                   fontations_ffi::BridgeColorStops&,
                                   uint8_t) override;
    virtual void fill_glyph_linear(uint16_t glyph_id,
                                   const fontations_ffi::Transform&,
                                   const fontations_ffi::FillLinearParams&,
                                   fontations_ffi::BridgeColorStops&,
                                   uint8_t) override;
    virtual void fill_glyph_sweep(uint16_t glyph_id,
                                  const fontations_ffi::Transform&,
                                  const fontations_ffi::FillSweepParams&,
                                  fontations_ffi::BridgeColorStops&,
                                  uint8_t) override;

private:
    SkFontationsScalerContext& fScalerContext;
    SkMatrix fCurrentTransform;
    SkMatrix fStackTopTransformInverse;

    uint16_t fUpem;
    SkRect fBounds;
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
    const fontations_ffi::BridgeOutlineCollection& getOutlines() {
        return *fOutlines;
    }
    SkSpan<SkColor> getPalette() {
        return SkSpan<SkColor>(reinterpret_cast<SkColor*>(fPalette.data()), fPalette.size());
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
    rust::Box<fontations_ffi::BridgeOutlineCollection> fOutlines;
    rust::Vec<uint32_t> fPalette;

    mutable SkOnce fGlyphMasksMayNeedCurrentColorOnce;
    mutable bool fGlyphMasksMayNeedCurrentColor;
};

#endif  // SkTypeface_Fontations_DEFINED
