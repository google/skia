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
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "include/private/base/SkOnce.h"
#include "include/private/base/SkTArray.h"
#include "src/core/SkAdvancedTypefaceMetrics.h"
#include "src/core/SkScalerContext.h"
#include "src/ports/fontations/src/ffi.rs.h"

#include <memory>

class SkStreamAsset;
class SkFontationsScalerContext;

namespace sk_fontations {

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
                 SkSpan<const SkColor> palette,
                 SkColor foregroundColor,
                 bool antialias,
                 uint16_t upem);

    // fontations_ffi::ColorPainter interface.
    virtual bool is_bounds_mode() override { return false; }
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
    SkSpan<const SkColor> fPalette;
    SkColor fForegroundColor;
    bool fAntialias;
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
    virtual bool is_bounds_mode() override { return true; }
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
    skia_private::STArray<4, SkMatrix> fMatrixStack;

    uint16_t fUpem;
    SkRect fBounds;
};

}  // namespace sk_fontations

/** SkTypeface implementation based on Google Fonts Fontations Rust libraries. */
class SkTypeface_Fontations : public SkTypeface {
private:
    SkTypeface_Fontations(sk_sp<SkData> fontData,
                          const SkFontStyle& style,
                          uint32_t ttcIndex,
                          rust::Box<fontations_ffi::BridgeFontRef>&& fontRef,
                          rust::Box<fontations_ffi::BridgeMappingIndex>&& mappingIndex,
                          rust::Box<fontations_ffi::BridgeNormalizedCoords>&& normalizedCoords,
                          rust::Box<fontations_ffi::BridgeOutlineCollection>&& outlines,
                          rust::Box<fontations_ffi::BridgeGlyphStyles>&& glyph_styles,
                          rust::Vec<uint32_t>&& palette);

public:
    const fontations_ffi::BridgeFontRef& getBridgeFontRef() const { return *fBridgeFontRef; }
    const fontations_ffi::BridgeNormalizedCoords& getBridgeNormalizedCoords() const {
        return *fBridgeNormalizedCoords;
    }
    const fontations_ffi::BridgeOutlineCollection& getOutlines() const { return *fOutlines; }
    const fontations_ffi::BridgeGlyphStyles& getGlyphStyles() const { return *fGlyphStyles; }
    const fontations_ffi::BridgeMappingIndex& getMappingIndex() const { return *fMappingIndex; }
    SkSpan<const SkColor> getPalette() const {
        return SkSpan(reinterpret_cast<const SkColor*>(fPalette.data()), fPalette.size());
    }

    static constexpr SkTypeface::FactoryId FactoryId = SkSetFourByteTag('f', 'n', 't', 'a');

    static sk_sp<SkTypeface> MakeFromData(sk_sp<SkData> fontData, const SkFontArguments&);
    static sk_sp<SkTypeface> MakeFromStream(std::unique_ptr<SkStreamAsset>, const SkFontArguments&);

protected:
    std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override;
    sk_sp<SkTypeface> onMakeClone(const SkFontArguments& args) const override;
    std::unique_ptr<SkScalerContext> onCreateScalerContext(const SkScalerContextEffects& effects,
                                                           const SkDescriptor* desc) const override;
    std::unique_ptr<SkScalerContext> onCreateScalerContextAsProxyTypeface(
            const SkScalerContextEffects&,
            const SkDescriptor*,
            SkTypeface* proxyTypeface) const override;
    void onFilterRec(SkScalerContextRec*) const override;
    std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const override;
    void onGetFontDescriptor(SkFontDescriptor*, bool*) const override;
    void onCharsToGlyphs(SkSpan<const SkUnichar>, SkSpan<SkGlyphID>) const override;
    int onCountGlyphs() const override;
    void getPostScriptGlyphNames(SkString*) const override {}
    void getGlyphToUnicodeMap(SkSpan<SkUnichar>) const override;
    int onGetUPEM() const override;
    void onGetFamilyName(SkString* familyName) const override;
    bool onGetPostScriptName(SkString*) const override;
    SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const override;
    bool onGlyphMaskNeedsCurrentColor() const override;
    int onGetVariationDesignPosition(
                             SkSpan<SkFontArguments::VariationPosition::Coordinate>) const override;
    int onGetVariationDesignParameters(SkSpan<SkFontParameters::Variation::Axis>) const override;
    int onGetTableTags(SkSpan<SkFontTableTag>) const override;
    size_t onGetTableData(SkFontTableTag, size_t, size_t, void*) const override;

private:
    sk_sp<SkData> fFontData;
    // Incoming ttc index requested when this typeface was instantiated from data.
    uint32_t fTtcIndex = 0;
    // fBridgeFontRef accesses the data in fFontData. fFontData needs to be kept around for the
    // lifetime of fBridgeFontRef to safely request parsed data.
    rust::Box<fontations_ffi::BridgeFontRef> fBridgeFontRef;
    rust::Box<fontations_ffi::BridgeMappingIndex> fMappingIndex;
    rust::Box<fontations_ffi::BridgeNormalizedCoords> fBridgeNormalizedCoords;
    rust::Box<fontations_ffi::BridgeOutlineCollection> fOutlines;
    rust::Box<fontations_ffi::BridgeGlyphStyles> fGlyphStyles;
    rust::Vec<uint32_t> fPalette;

    mutable SkOnce fGlyphMasksMayNeedCurrentColorOnce;
    mutable bool fGlyphMasksMayNeedCurrentColor;
};

#endif  // SkTypeface_Fontations_DEFINED
