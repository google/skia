/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkStream.h"
#include "include/pathops/SkPathOps.h"
#include "include/ports/SkTypeface_fontations.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkFontPriv.h"
#include "src/ports/SkTypeface_fontations_priv.h"

#include "src/ports/fontations/src/skpath_bridge.h"

namespace {

sk_sp<SkData> streamToData(const std::unique_ptr<SkStreamAsset>& font_data) {
    // TODO(drott): From a stream this causes a full read/copy. Make sure
    // we can instantiate this directly from the decompressed buffer that
    // Blink has after OTS and woff2 decompression.
    font_data->rewind();
    return SkData::MakeFromStream(font_data.get(), font_data->getLength());
}

rust::Box<::fontations_ffi::BridgeFontRef> make_bridge_font_ref(sk_sp<SkData> fontData,
                                                                uint32_t index) {
    rust::Slice<const uint8_t> slice{fontData->bytes(), fontData->size()};
    return fontations_ffi::make_font_ref(slice, index);
}

static_assert(sizeof(fontations_ffi::SkiaDesignCoordinate) ==
                      sizeof(SkFontArguments::VariationPosition::Coordinate) &&
              sizeof(fontations_ffi::SkiaDesignCoordinate::axis) ==
                      sizeof(SkFontArguments::VariationPosition::Coordinate::axis) &&
              sizeof(fontations_ffi::SkiaDesignCoordinate::value) ==
                      sizeof(SkFontArguments::VariationPosition::Coordinate::value) &&
              offsetof(fontations_ffi::SkiaDesignCoordinate, axis) ==
                      offsetof(SkFontArguments::VariationPosition::Coordinate, axis) &&
              offsetof(fontations_ffi::SkiaDesignCoordinate, value) ==
                      offsetof(SkFontArguments::VariationPosition::Coordinate, value) &&
              "Struct fontations_ffi::SkiaDesignCoordinate must match "
              "SkFontArguments::VariationPosition::Coordinate.");

rust::Box<fontations_ffi::BridgeNormalizedCoords> make_normalized_coords(
        fontations_ffi::BridgeFontRef const& bridgeFontRef, const SkFontArguments& args) {
    SkFontArguments::VariationPosition variationPosition = args.getVariationDesignPosition();
    // Cast is safe because of static_assert matching the structs above.
    rust::Slice<const fontations_ffi::SkiaDesignCoordinate> coordinates(
            reinterpret_cast<const fontations_ffi::SkiaDesignCoordinate*>(
                    variationPosition.coordinates),
            variationPosition.coordinateCount);
    return resolve_into_normalized_coords(bridgeFontRef, coordinates);
}
}  // namespace

SK_API sk_sp<SkTypeface> SkTypeface_Make_Fontations(std::unique_ptr<SkStreamAsset> fontData,
                                                    const SkFontArguments& args) {
    return SkTypeface_Fontations::MakeFromStream(std::move(fontData), args);
}

SkTypeface_Fontations::SkTypeface_Fontations(sk_sp<SkData> fontData, const SkFontArguments& args)
        : SkTypeface(SkFontStyle(), true)
        , fFontData(fontData)
        , fTtcIndex(args.getCollectionIndex())
        , fBridgeFontRef(make_bridge_font_ref(fFontData, fTtcIndex))
        , fBridgeNormalizedCoords(make_normalized_coords(*fBridgeFontRef, args)) {}

sk_sp<SkTypeface> SkTypeface_Fontations::MakeFromStream(std::unique_ptr<SkStreamAsset> stream,
                                                        const SkFontArguments& args) {
    return MakeFromData(streamToData(stream), args);
}

sk_sp<SkTypeface> SkTypeface_Fontations::MakeFromData(sk_sp<SkData> data,
                                                      const SkFontArguments& args) {
    sk_sp<SkTypeface_Fontations> probeTypeface(new SkTypeface_Fontations(data, args));
    return probeTypeface->hasValidBridgeFontRef() ? probeTypeface : nullptr;
}

namespace sk_fontations {
// Path sanitization ported from SkFTGeometrySink.
void PathGeometrySink::going_to(SkPoint point) {
    if (!fStarted) {
        fStarted = true;
        fPath.moveTo(fCurrent);
    }
    fCurrent = point;
}

bool PathGeometrySink::current_is_not(SkPoint point) { return fCurrent != point; }

void PathGeometrySink::move_to(float x, float y) {
    if (fStarted) {
        fPath.close();
        fStarted = false;
    }
    fCurrent = SkPoint::Make(SkFloatToScalar(x), SkFloatToScalar(y));
}

void PathGeometrySink::line_to(float x, float y) {
    SkPoint pt0 = SkPoint::Make(SkFloatToScalar(x), SkFloatToScalar(y));
    if (current_is_not(pt0)) {
        going_to(pt0);
        fPath.lineTo(pt0);
    }
}

void PathGeometrySink::quad_to(float cx0, float cy0, float x, float y) {
    SkPoint pt0 = SkPoint::Make(SkFloatToScalar(cx0), SkFloatToScalar(cy0));
    SkPoint pt1 = SkPoint::Make(SkFloatToScalar(x), SkFloatToScalar(y));
    if (current_is_not(pt0) || current_is_not(pt1)) {
        going_to(pt1);
        fPath.quadTo(pt0, pt1);
    }
}
void PathGeometrySink::curve_to(float cx0, float cy0, float cx1, float cy1, float x, float y) {
    SkPoint pt0 = SkPoint::Make(SkFloatToScalar(cx0), SkFloatToScalar(cy0));
    SkPoint pt1 = SkPoint::Make(SkFloatToScalar(cx1), SkFloatToScalar(cy1));
    SkPoint pt2 = SkPoint::Make(SkFloatToScalar(x), SkFloatToScalar(y));
    if (current_is_not(pt0) || current_is_not(pt1) || current_is_not(pt2)) {
        going_to(pt2);
        fPath.cubicTo(pt0, pt1, pt2);
    }
}

void PathGeometrySink::close() { fPath.close(); }

SkPath PathGeometrySink::into_inner() && { return std::move(fPath); }

AxisWrapper::AxisWrapper(SkFontParameters::Variation::Axis axisArray[], size_t axisCount)
        : fAxisArray(axisArray), fAxisCount(axisCount) {}

bool AxisWrapper::populate_axis(
        size_t i, uint32_t axisTag, float min, float def, float max, bool hidden) {
    if (i >= fAxisCount) {
        return false;
    }
    SkFontParameters::Variation::Axis& axis = fAxisArray[i];
    axis.tag = axisTag;
    axis.min = min;
    axis.def = def;
    axis.max = max;
    axis.setHidden(hidden);
    return true;
}

size_t AxisWrapper::size() const { return fAxisCount; }
}  // namespace sk_fontations

int SkTypeface_Fontations::onGetUPEM() const {
    return fontations_ffi::units_per_em_or_zero(*fBridgeFontRef);
}

void SkTypeface_Fontations::onGetFamilyName(SkString* familyName) const {
    rust::String readFamilyName = fontations_ffi::family_name(*fBridgeFontRef);
    *familyName = SkString(readFamilyName.data(), readFamilyName.size());
}

bool SkTypeface_Fontations::onGetPostScriptName(SkString* postscriptName) const {
    rust::String readPsName;
    if (fontations_ffi::postscript_name(*fBridgeFontRef, readPsName)) {
        *postscriptName = SkString(readPsName.data(), readPsName.size());
        return true;
    }

    return false;
}

void SkTypeface_Fontations::onCharsToGlyphs(const SkUnichar* chars,
                                            int count,
                                            SkGlyphID glyphs[]) const {
    sk_bzero(glyphs, count * sizeof(glyphs[0]));

    for (int i = 0; i < count; ++i) {
        glyphs[i] = fontations_ffi::lookup_glyph_or_zero(*fBridgeFontRef, chars[i]);
    }
}
int SkTypeface_Fontations::onCountGlyphs() const {
    return fontations_ffi::num_glyphs(*fBridgeFontRef);
}

bool SkTypeface_Fontations::hasValidBridgeFontRef() const {
    return fontations_ffi::font_ref_is_valid(*fBridgeFontRef);
}

void SkTypeface_Fontations::onFilterRec(SkScalerContextRec* rec) const {
    rec->setHinting(SkFontHinting::kNone);
}

class SkrifaLocalizedStrings : public SkTypeface::LocalizedStrings {
public:
    SkrifaLocalizedStrings(
            rust::Box<::fontations_ffi::BridgeLocalizedStrings> bridge_localized_strings)
            : fBridgeLocalizedStrings(std::move(bridge_localized_strings)) {}
    bool next(SkTypeface::LocalizedString* localized_string) override {
        fontations_ffi::BridgeLocalizedName localizedName;
        if (!fontations_ffi::localized_name_next(*fBridgeLocalizedStrings, localizedName)) {
            return false;
        }
        localized_string->fString =
                SkString(localizedName.string.data(), localizedName.string.size());
        localized_string->fLanguage =
                SkString(localizedName.language.data(), localizedName.language.size());
        return true;
    }

private:
    rust::Box<::fontations_ffi::BridgeLocalizedStrings> fBridgeLocalizedStrings;
};

SkTypeface::LocalizedStrings* SkTypeface_Fontations::onCreateFamilyNameIterator() const {
    return new SkrifaLocalizedStrings(fontations_ffi::get_localized_strings(*fBridgeFontRef));
}

class SkFontationsScalerContext : public SkScalerContext {
public:
    SkFontationsScalerContext(sk_sp<SkTypeface_Fontations> face,
                              const SkScalerContextEffects& effects,
                              const SkDescriptor* desc)
            : SkScalerContext(face, effects, desc)
            , fBridgeFontRef(
                      static_cast<SkTypeface_Fontations*>(this->getTypeface())->getBridgeFontRef())
            , fBridgeNormalizedCoords(static_cast<SkTypeface_Fontations*>(this->getTypeface())
                                              ->getBridgeNormalizedCoords()) {
        fRec.getSingleMatrix(&fMatrix);
        this->forceGenerateImageFromPath();
    }

protected:
    GlyphMetrics generateMetrics(const SkGlyph& glyph, SkArenaAlloc*) override {
        GlyphMetrics mx(fRec.fMaskFormat);

        SkVector scale;
        SkMatrix remainingMatrix;
        if (!fRec.computeMatrices(
                    SkScalerContextRec::PreMatrixScale::kVertical, &scale, &remainingMatrix)) {
            return mx;
        }
        float x_advance = 0.0f;
        x_advance = fontations_ffi::advance_width_or_zero(
                fBridgeFontRef, scale.y(), fBridgeNormalizedCoords, glyph.getGlyphID());
        // TODO(drott): y-advance?
        mx.advance = remainingMatrix.mapXY(x_advance, SkFloatToScalar(0.f));
        mx.computeFromPath = true;
        return mx;
    }

    void generateImage(const SkGlyph&, void*) override {
        SK_ABORT("Should have generated from path.");
    }

    bool generatePath(const SkGlyph& glyph, SkPath* path) override {
        SkVector scale;
        SkMatrix remainingMatrix;
        if (!fRec.computeMatrices(
                    SkScalerContextRec::PreMatrixScale::kVertical, &scale, &remainingMatrix)) {
            return false;
        }
        sk_fontations::PathGeometrySink pathWrapper;
        fontations_ffi::BridgeScalerMetrics scalerMetrics;

        if (!fontations_ffi::get_path(fBridgeFontRef,
                                      glyph.getGlyphID(),
                                      scale.y(),
                                      fBridgeNormalizedCoords,
                                      pathWrapper,
                                      scalerMetrics)) {
            return false;
        }
        *path = std::move(pathWrapper).into_inner();
        if (scalerMetrics.has_overlaps) {
            // See SkScalerContext_FreeType_Base::generateGlyphPath.
            Simplify(*path, path);
            AsWinding(*path, path);
        }
        *path = path->makeTransform(remainingMatrix);
        return true;
    }

    void generateFontMetrics(SkFontMetrics* out_metrics) override {
        fontations_ffi::Metrics metrics = fontations_ffi::get_skia_metrics(
                fBridgeFontRef, fMatrix.getScaleY(), fBridgeNormalizedCoords);
        out_metrics->fTop = -metrics.top;
        out_metrics->fAscent = -metrics.ascent;
        out_metrics->fDescent = -metrics.descent;
        out_metrics->fBottom = -metrics.bottom;
        out_metrics->fLeading = -metrics.leading;
        out_metrics->fAvgCharWidth = metrics.avg_char_width;
        out_metrics->fMaxCharWidth = metrics.max_char_width;
        out_metrics->fXMin = metrics.x_min;
        out_metrics->fXMax = metrics.x_max;
        out_metrics->fXHeight = -metrics.x_height;
        out_metrics->fCapHeight = -metrics.cap_height;
        out_metrics->fFlags = 0;
        // TODO(drott): Is it necessary to transform metrics with remaining parts of matrix?
    }

private:
    SkMatrix fMatrix;
    sk_sp<SkData> fFontData = nullptr;
    const fontations_ffi::BridgeFontRef& fBridgeFontRef;
    const fontations_ffi::BridgeNormalizedCoords& fBridgeNormalizedCoords;
};

std::unique_ptr<SkStreamAsset> SkTypeface_Fontations::onOpenStream(int* ttcIndex) const {
    *ttcIndex = fTtcIndex;
    return std::make_unique<SkMemoryStream>(fFontData);
}

sk_sp<SkTypeface> SkTypeface_Fontations::onMakeClone(const SkFontArguments& args) const {
    return MakeFromData(fFontData, args);
}

std::unique_ptr<SkScalerContext> SkTypeface_Fontations::onCreateScalerContext(
        const SkScalerContextEffects& effects, const SkDescriptor* desc) const {
    return std::make_unique<SkFontationsScalerContext>(
            sk_ref_sp(const_cast<SkTypeface_Fontations*>(this)), effects, desc);
}

void SkTypeface_Fontations::onGetFontDescriptor(SkFontDescriptor* desc, bool* serialize) const {
    SkString familyName;
    onGetFamilyName(&familyName);
    desc->setFamilyName(familyName.c_str());
    desc->setStyle(this->fontStyle());
    desc->setFactoryId(FactoryId);
    *serialize = true;
}

size_t SkTypeface_Fontations::onGetTableData(SkFontTableTag tag,
                                             size_t offset,
                                             size_t length,
                                             void* data) const {
    rust::Slice<uint8_t> dataSlice;
    if (data) {
        dataSlice = rust::Slice<uint8_t>(reinterpret_cast<uint8_t*>(data), length);
    }
    size_t copied = fontations_ffi::table_data(*fBridgeFontRef, tag, offset, dataSlice);
    // If data is nullptr, the Rust side doesn't see a length limit.
    return std::min(copied, length);
}

int SkTypeface_Fontations::onGetTableTags(SkFontTableTag tags[]) const {
    uint16_t numTables = fontations_ffi::table_tags(*fBridgeFontRef, rust::Slice<uint32_t>());
    if (!tags) {
      return numTables;
    }
    rust::Slice<uint32_t> copyToTags(tags, numTables);
    return fontations_ffi::table_tags(*fBridgeFontRef, copyToTags);
}

int SkTypeface_Fontations::onGetVariationDesignPosition(
        SkFontArguments::VariationPosition::Coordinate coordinates[], int coordinateCount) const {
    rust::Slice<fontations_ffi::SkiaDesignCoordinate> copyToCoordinates;
    if (coordinates) {
      copyToCoordinates = rust::Slice<fontations_ffi::SkiaDesignCoordinate>(
              reinterpret_cast<fontations_ffi::SkiaDesignCoordinate*>(coordinates),
              coordinateCount);
    }
    return fontations_ffi::variation_position(*fBridgeNormalizedCoords, copyToCoordinates);
}

int SkTypeface_Fontations::onGetVariationDesignParameters(
        SkFontParameters::Variation::Axis parameters[], int parameterCount) const {
    sk_fontations::AxisWrapper axisWrapper(parameters, parameterCount);
    return fontations_ffi::populate_axes(*fBridgeFontRef, axisWrapper);
}
