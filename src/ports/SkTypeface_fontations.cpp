/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/ports/SkTypeface_fontations.h"

#include "include/codec/SkCodec.h"
#include "include/codec/SkPngDecoder.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkImage.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkStream.h"
#include "include/effects/SkGradientShader.h"
#include "include/pathops/SkPathOps.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkFontPriv.h"
#include "src/ports/SkTypeface_fontations_priv.h"
#include "src/ports/fontations/src/skpath_bridge.h"

namespace {

[[maybe_unused]] static inline const constexpr bool kSkShowTextBlitCoverage = false;

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

SkMatrix SkMatrixFromFontationsTransform(const fontations_ffi::Transform& transformArg) {
    return SkMatrix::MakeAll(transformArg.xx,
                             -transformArg.xy,
                             transformArg.dx,
                             -transformArg.yx,
                             transformArg.yy,
                             -transformArg.dy,
                             0.f,
                             0.f,
                             1.0f);
}

}  // namespace

sk_sp<SkTypeface> SkTypeface_Make_Fontations(std::unique_ptr<SkStreamAsset> fontData,
                                             const SkFontArguments& args) {
    return SkTypeface_Fontations::MakeFromStream(std::move(fontData), args);
}

SkTypeface_Fontations::SkTypeface_Fontations(sk_sp<SkData> fontData, const SkFontArguments& args)
        : SkTypeface(SkFontStyle(), true)
        , fFontData(fontData)
        , fTtcIndex(args.getCollectionIndex())
        , fBridgeFontRef(make_bridge_font_ref(fFontData, fTtcIndex))
        , fBridgeNormalizedCoords(make_normalized_coords(*fBridgeFontRef, args))
        , fOutlines(fontations_ffi::get_outline_collection(*fBridgeFontRef))
        , fPalette(resolve_palette(
                  *fBridgeFontRef,
                  args.getPalette().index,
                  rust::Slice<const fontations_ffi::PaletteOverride>(
                          reinterpret_cast<const ::fontations_ffi::PaletteOverride*>(
                                  args.getPalette().overrides),
                          args.getPalette().overrideCount))) {}

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
        if (postscriptName) {
            *postscriptName = SkString(readPsName.data(), readPsName.size());
        }
        return true;
    }

    return false;
}

bool SkTypeface_Fontations::onGlyphMaskNeedsCurrentColor() const {
    fGlyphMasksMayNeedCurrentColorOnce([this] {
        static constexpr SkFourByteTag COLRTag = SkSetFourByteTag('C', 'O', 'L', 'R');
        fGlyphMasksMayNeedCurrentColor = this->getTableSize(COLRTag) > 0;
    });
    return fGlyphMasksMayNeedCurrentColor;
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
                                              ->getBridgeNormalizedCoords())
            , fOutlines(static_cast<SkTypeface_Fontations*>(this->getTypeface())->getOutlines())
            , fPalette(static_cast<SkTypeface_Fontations*>(this->getTypeface())->getPalette()) {
        fRec.getSingleMatrix(&fMatrix);
    }

    // TODO(drott): Add parameter/control for hinting here once that is available from Fontations.
    bool generateYScalePathForGlyphId(uint16_t glyphId, SkPath* path, float yScale) {
        sk_fontations::PathGeometrySink pathWrapper;
        fontations_ffi::BridgeScalerMetrics scalerMetrics;

        if (!fontations_ffi::get_path(fOutlines,
                                      glyphId,
                                      yScale,
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
        return true;
    }

protected:
    struct ScalerContextBits {
        using value_type = uint16_t;
        static const constexpr value_type PATH = 1;
        static const constexpr value_type COLRv0 = 2;
        static const constexpr value_type COLRv1 = 3;
        static const constexpr value_type BITMAP = 4;
    };

    GlyphMetrics generateMetrics(const SkGlyph& glyph, SkArenaAlloc*) override {
        GlyphMetrics mx(glyph.maskFormat());

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

        // The FreeType backend has a big switch here:
        // Scalable or bitmap, monochromatic or color, subpixel shifting bounds if needed.
        // For now: check if COLRv1, get clipbox, else -
        // get bounds from Path.
        // TODO(drott): Later move bounds retrieval for monochromatic glyphs to retrieving
        // them from Skrifa scaler, taking hinting into account.

        bool has_colrv1_glyph =
                fontations_ffi::has_colrv1_glyph(fBridgeFontRef, glyph.getGlyphID());
        bool has_colrv0_glyph =
                fontations_ffi::has_colrv0_glyph(fBridgeFontRef, glyph.getGlyphID());
        bool has_bitmap_glyph =
                fontations_ffi::has_bitmap_glyph(fBridgeFontRef, glyph.getGlyphID());

        if (has_colrv1_glyph || has_colrv0_glyph) {
            mx.extraBits = has_colrv1_glyph ? ScalerContextBits::COLRv1 : ScalerContextBits::COLRv0;
            mx.maskFormat = SkMask::kARGB32_Format;
            mx.neverRequestPath = true;

            fontations_ffi::ClipBox clipBox;
            if (has_colrv1_glyph && fontations_ffi::get_colrv1_clip_box(fBridgeFontRef,
                                                                        fBridgeNormalizedCoords,
                                                                        glyph.getGlyphID(),
                                                                        scale.y(),
                                                                        clipBox)) {
                // Flip y.
                SkRect boundsRect = SkRect::MakeLTRB(
                        clipBox.x_min, -clipBox.y_max, clipBox.x_max, -clipBox.y_min);

                if (!remainingMatrix.isIdentity()) {
                    SkPath boundsPath = SkPath::Rect(boundsRect);
                    boundsPath.transform(remainingMatrix);
                    boundsRect = boundsPath.getBounds();
                }

                boundsRect.roundOut(&mx.bounds);

            } else {
                uint16_t upem = fontations_ffi::units_per_em_or_zero(fBridgeFontRef);
                if (upem == 0) {
                    mx.bounds = SkRect::MakeEmpty();
                } else {
                    SkMatrix fullTransform;
                    fRec.getSingleMatrix(&fullTransform);
                    fullTransform.preScale(1.f / upem, 1.f / upem);

                    sk_fontations::BoundsPainter boundsPainter(*this, fullTransform, upem);
                    bool result = fontations_ffi::draw_colr_glyph(fBridgeFontRef,
                                                                  fBridgeNormalizedCoords,
                                                                  glyph.getGlyphID(),
                                                                  boundsPainter);
                    if (result) {
                        boundsPainter.getBoundingBox().roundOut(&mx.bounds);
                    } else {
                        mx.bounds = SkRect::MakeEmpty();
                    }
                }
            }
        } else if (has_bitmap_glyph) {
            mx.maskFormat = SkMask::kARGB32_Format;
            mx.neverRequestPath = true;
            mx.extraBits = ScalerContextBits::BITMAP;

            rust::cxxbridge1::Box<fontations_ffi::BridgeBitmapGlyph> bitmap_glyph =
                    fontations_ffi::bitmap_glyph(fBridgeFontRef, glyph.getGlyphID(), scale.fY);
            rust::cxxbridge1::Slice<const uint8_t> png_data =
                    fontations_ffi::png_data(*bitmap_glyph);
            SkASSERT(png_data.size());

            const fontations_ffi::BitmapMetrics bitmap_metrics =
                    fontations_ffi::bitmap_metrics(*bitmap_glyph);

            const bool& placement_origin_bottom_left = bitmap_metrics.placement_origin_bottom_left;

            std::unique_ptr<SkCodec> codec = SkPngDecoder::Decode(
                    SkData::MakeWithoutCopy(png_data.data(), png_data.size()), nullptr);
            if (!codec) {
                return mx;
            }

            SkRect bounds = SkRect::MakeEmpty();
            SkImageInfo info = codec->getInfo();
            float height_adjustment = placement_origin_bottom_left ? info.height() : 0;

            bounds = SkRect::Make(info.bounds());
            SkMatrix matrix = remainingMatrix;
            SkScalar ratio = scale.fY / bitmap_metrics.ppem_y;
            matrix.preScale(ratio, ratio);
            matrix.preTranslate(bitmap_metrics.bearing_x,
                                -bitmap_metrics.bearing_y - height_adjustment);
            if (this->isSubpixel()) {
                matrix.postTranslate(SkFixedToScalar(glyph.getSubXFixed()),
                                     SkFixedToScalar(glyph.getSubYFixed()));
            }
            matrix.mapRect(&bounds);
            mx.bounds = SkRect::Make(bounds.roundOut());
        } else {
            // TODO: Retrieve from read_fonts and Skrifa - TrueType bbox or from path with
            // hinting?
            mx.extraBits = ScalerContextBits::PATH;
            mx.computeFromPath = true;
        }
        return mx;
    }

    void generatePngImage(const SkGlyph& glyph, void* imageBuffer) {
        SkASSERT(glyph.maskFormat() == SkMask::kARGB32_Format);
        SkBitmap dstBitmap;
        dstBitmap.setInfo(
                SkImageInfo::Make(
                        glyph.width(), glyph.height(), kN32_SkColorType, kPremul_SkAlphaType),
                glyph.rowBytes());
        dstBitmap.setPixels(imageBuffer);

        SkCanvas canvas(dstBitmap);

        canvas.translate(-glyph.left(), -glyph.top());

        SkVector scale;
        SkMatrix remainingMatrix;
        if (!fRec.computeMatrices(
                    SkScalerContextRec::PreMatrixScale::kVertical, &scale, &remainingMatrix)) {
            return;
        }

        rust::cxxbridge1::Box<fontations_ffi::BridgeBitmapGlyph> bitmap_glyph =
                fontations_ffi::bitmap_glyph(fBridgeFontRef, glyph.getGlyphID(), scale.fY);
        rust::cxxbridge1::Slice<const uint8_t> png_data = fontations_ffi::png_data(*bitmap_glyph);
        SkASSERT(png_data.size());

        std::unique_ptr<SkCodec> codec = SkPngDecoder::Decode(
                SkData::MakeWithoutCopy(png_data.data(), png_data.size()), nullptr);

        if (!codec) {
            return;
        }

        auto [glyph_image, result] = codec->getImage();
        if (result != SkCodec::Result::kSuccess) {
            return;
        }

        canvas.clear(SK_ColorTRANSPARENT);
        canvas.concat(remainingMatrix);

        if (this->isSubpixel()) {
            canvas.translate(SkFixedToScalar(glyph.getSubXFixed()),
                             SkFixedToScalar(glyph.getSubYFixed()));
        }
        const fontations_ffi::BitmapMetrics bitmapMetrics =
                fontations_ffi::bitmap_metrics(*bitmap_glyph);
        SkScalar ratio = scale.fY / bitmapMetrics.ppem_y;
        canvas.scale(ratio, ratio);

        float height_adjustment =
                bitmapMetrics.placement_origin_bottom_left ? glyph_image->height() : 0;
        canvas.translate(bitmapMetrics.bearing_x, -bitmapMetrics.bearing_y - height_adjustment);

        SkSamplingOptions sampling(SkFilterMode::kLinear, SkMipmapMode::kNearest);
        canvas.drawImage(glyph_image, 0, 0, sampling);
    }

    void generateImage(const SkGlyph& glyph, void* imageBuffer) override {
        ScalerContextBits::value_type format = glyph.extraBits();
        if (format == ScalerContextBits::PATH) {
            const SkPath* devPath = glyph.path();
            SkASSERT_RELEASE(devPath);
            SkMaskBuilder mask(static_cast<uint8_t*>(imageBuffer),
                               glyph.iRect(),
                               glyph.rowBytes(),
                               glyph.maskFormat());
            SkASSERT(SkMask::kARGB32_Format != mask.fFormat);
            const bool doBGR = SkToBool(fRec.fFlags & SkScalerContext::kLCD_BGROrder_Flag);
            const bool doVert = SkToBool(fRec.fFlags & SkScalerContext::kLCD_Vertical_Flag);
            const bool a8LCD = SkToBool(fRec.fFlags & SkScalerContext::kGenA8FromLCD_Flag);
            const bool hairline = glyph.pathIsHairline();
            GenerateImageFromPath(mask, *devPath, fPreBlend, doBGR, doVert, a8LCD, hairline);

        } else if (format == ScalerContextBits::COLRv1 || format == ScalerContextBits::COLRv0) {
            SkASSERT(glyph.maskFormat() == SkMask::kARGB32_Format);
            SkBitmap dstBitmap;
            dstBitmap.setInfo(
                    SkImageInfo::Make(
                            glyph.width(), glyph.height(), kN32_SkColorType, kPremul_SkAlphaType),
                    glyph.rowBytes());
            dstBitmap.setPixels(imageBuffer);

            SkCanvas canvas(dstBitmap);
            if constexpr (kSkShowTextBlitCoverage) {
                canvas.clear(0x33FF0000);
            } else {
                canvas.clear(SK_ColorTRANSPARENT);
            }
            canvas.translate(-glyph.left(), -glyph.top());

            drawCOLRGlyph(glyph, fRec.fForegroundColor, &canvas);
        } else if (format == ScalerContextBits::BITMAP) {
            generatePngImage(glyph, imageBuffer);
        } else {
            SK_ABORT("Bad format");
        }
    }

    bool generatePath(const SkGlyph& glyph, SkPath* path) override {
        SkASSERT(glyph.extraBits() == ScalerContextBits::PATH);

        SkVector scale;
        SkMatrix remainingMatrix;
        if (!fRec.computeMatrices(
                    SkScalerContextRec::PreMatrixScale::kVertical, &scale, &remainingMatrix)) {
            return false;
        }
        bool result = generateYScalePathForGlyphId(glyph.getGlyphID(), path, scale.y());
        if (!result) {
            return false;
        }

        *path = path->makeTransform(remainingMatrix);
        return true;
    }

    bool drawCOLRGlyph(const SkGlyph& glyph, SkColor foregroundColor, SkCanvas* canvas) {
        uint16_t upem = fontations_ffi::units_per_em_or_zero(fBridgeFontRef);
        if (upem == 0) {
            return false;
        }

        SkMatrix scalerMatrix;
        fRec.getSingleMatrix(&scalerMatrix);
        SkAutoCanvasRestore autoRestore(canvas, true /* doSave */);

        // Scale down so that COLR operations can happen in glyph coordinates.
        SkMatrix upemToPpem = SkMatrix::Scale(1.f / upem, 1.f / upem);
        scalerMatrix.preConcat(upemToPpem);
        canvas->concat(scalerMatrix);
        SkPaint defaultPaint;
        defaultPaint.setColor(SK_ColorRED);
        sk_fontations::ColorPainter colorPainter(*this, *canvas, fPalette, foregroundColor, upem);
        bool result = fontations_ffi::draw_colr_glyph(
                fBridgeFontRef, fBridgeNormalizedCoords, glyph.getGlyphID(), colorPainter);
        return result;
    }

    sk_sp<SkDrawable> generateDrawable(const SkGlyph& glyph) override {
        struct GlyphDrawable : public SkDrawable {
            SkFontationsScalerContext* fSelf;
            SkGlyph fGlyph;
            GlyphDrawable(SkFontationsScalerContext* self, const SkGlyph& glyph)
                    : fSelf(self), fGlyph(glyph) {}
            SkRect onGetBounds() override { return fGlyph.rect(); }
            size_t onApproximateBytesUsed() override { return sizeof(GlyphDrawable); }
            void maybeShowTextBlitCoverage(SkCanvas* canvas) {
                if constexpr (kSkShowTextBlitCoverage) {
                    SkPaint paint;
                    paint.setColor(0x3300FF00);
                    paint.setStyle(SkPaint::kFill_Style);
                    canvas->drawRect(this->onGetBounds(), paint);
                }
            }
        };
        struct ColrGlyphDrawable : public GlyphDrawable {
            using GlyphDrawable::GlyphDrawable;
            void onDraw(SkCanvas* canvas) override {
                this->maybeShowTextBlitCoverage(canvas);
                fSelf->drawCOLRGlyph(fGlyph, fSelf->fRec.fForegroundColor, canvas);
            }
        };
        ScalerContextBits::value_type format = glyph.extraBits();
        if (format == ScalerContextBits::COLRv1 || format == ScalerContextBits::COLRv0) {
            return sk_sp<SkDrawable>(new ColrGlyphDrawable(this, glyph));
        }
        return nullptr;
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
    const fontations_ffi::BridgeOutlineCollection& fOutlines;
    const SkSpan<SkColor> fPalette;
    friend class sk_fontations::ColorPainter;
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

namespace sk_fontations {

namespace {

const uint16_t kForegroundColorPaletteIndex = 0xFFFF;

void populateStopsAndColors(std::vector<SkScalar>& dest_stops,
                            std::vector<SkColor4f>& dest_colors,
                            const SkSpan<SkColor>& palette,
                            SkColor foregroundColor,
                            fontations_ffi::BridgeColorStops& color_stops) {
    SkASSERT(dest_stops.size() == 0);
    SkASSERT(dest_colors.size() == 0);
    size_t num_color_stops = fontations_ffi::num_color_stops(color_stops);
    dest_stops.reserve(num_color_stops);
    dest_colors.reserve(num_color_stops);

    fontations_ffi::ColorStop color_stop;
    while (fontations_ffi::next_color_stop(color_stops, color_stop)) {
        dest_stops.push_back(color_stop.stop);
        SkColor4f dest_color;
        if (color_stop.palette_index == kForegroundColorPaletteIndex) {
            dest_color = SkColor4f::FromColor(foregroundColor);
        } else {
            dest_color = SkColor4f::FromColor(palette[color_stop.palette_index]);
        }
        dest_color.fA *= color_stop.alpha;
        dest_colors.push_back(dest_color);
    }
}

SkColor4f lerpSkColor(SkColor4f c0, SkColor4f c1, float t) {
    // Due to the floating point calculation in the caller, when interpolating between very
    // narrow stops, we may get values outside the interpolation range, guard against these.
    if (t < 0) {
        return c0;
    }
    if (t > 1) {
        return c1;
    }

    const auto c0_4f = skvx::float4::Load(c0.vec());
    const auto c1_4f = skvx::float4::Load(c1.vec());
    const auto c_4f = c0_4f + (c1_4f - c0_4f) * t;

    SkColor4f l;
    c_4f.store(l.vec());
    return l;
}

enum TruncateStops { TruncateStart, TruncateEnd };

// Truncate a vector of color stops at a previously computed stop position and insert at that
// position the color interpolated between the surrounding stops.
void truncateToStopInterpolating(SkScalar zeroRadiusStop,
                                 std::vector<SkColor4f>& colors,
                                 std::vector<SkScalar>& stops,
                                 TruncateStops truncateStops) {
    if (stops.size() <= 1u || zeroRadiusStop < stops.front() || stops.back() < zeroRadiusStop) {
        return;
    }

    size_t afterIndex =
            (truncateStops == TruncateStart)
                    ? std::lower_bound(stops.begin(), stops.end(), zeroRadiusStop) - stops.begin()
                    : std::upper_bound(stops.begin(), stops.end(), zeroRadiusStop) - stops.begin();

    const float t =
            (zeroRadiusStop - stops[afterIndex - 1]) / (stops[afterIndex] - stops[afterIndex - 1]);
    SkColor4f lerpColor = lerpSkColor(colors[afterIndex - 1], colors[afterIndex], t);

    if (truncateStops == TruncateStart) {
        stops.erase(stops.begin(), stops.begin() + afterIndex);
        colors.erase(colors.begin(), colors.begin() + afterIndex);
        stops.insert(stops.begin(), 0);
        colors.insert(colors.begin(), lerpColor);
    } else {
        stops.erase(stops.begin() + afterIndex, stops.end());
        colors.erase(colors.begin() + afterIndex, colors.end());
        stops.insert(stops.end(), 1);
        colors.insert(colors.end(), lerpColor);
    }
}

// https://learn.microsoft.com/en-us/typography/opentype/spec/colr#format-32-paintcomposite
inline SkBlendMode ToSkBlendMode(uint16_t colrV1CompositeMode) {
    switch (colrV1CompositeMode) {
        case 0:
            return SkBlendMode::kClear;
        case 1:
            return SkBlendMode::kSrc;
        case 2:
            return SkBlendMode::kDst;
        case 3:
            return SkBlendMode::kSrcOver;
        case 4:
            return SkBlendMode::kDstOver;
        case 5:
            return SkBlendMode::kSrcIn;
        case 6:
            return SkBlendMode::kDstIn;
        case 7:
            return SkBlendMode::kSrcOut;
        case 8:
            return SkBlendMode::kDstOut;
        case 9:
            return SkBlendMode::kSrcATop;
        case 10:
            return SkBlendMode::kDstATop;
        case 11:
            return SkBlendMode::kXor;
        case 12:
            return SkBlendMode::kPlus;
        case 13:
            return SkBlendMode::kScreen;
        case 14:
            return SkBlendMode::kOverlay;
        case 15:
            return SkBlendMode::kDarken;
        case 16:
            return SkBlendMode::kLighten;
        case 17:
            return SkBlendMode::kColorDodge;
        case 18:
            return SkBlendMode::kColorBurn;
        case 19:
            return SkBlendMode::kHardLight;
        case 20:
            return SkBlendMode::kSoftLight;
        case 21:
            return SkBlendMode::kDifference;
        case 22:
            return SkBlendMode::kExclusion;
        case 23:
            return SkBlendMode::kMultiply;
        case 24:
            return SkBlendMode::kHue;
        case 25:
            return SkBlendMode::kSaturation;
        case 26:
            return SkBlendMode::kColor;
        case 27:
            return SkBlendMode::kLuminosity;
        default:
            return SkBlendMode::kDst;
    }
}

inline SkTileMode ToSkTileMode(uint8_t extendMode) {
    switch (extendMode) {
        case 1:
            return SkTileMode::kRepeat;
        case 2:
            return SkTileMode::kMirror;
        default:
            return SkTileMode::kClamp;
    }
}
}  // namespace

ColorPainter::ColorPainter(SkFontationsScalerContext& scaler_context,
                           SkCanvas& canvas,
                           SkSpan<SkColor> palette,
                           SkColor foregroundColor,
                           uint16_t upem)
        : fScalerContext(scaler_context)
        , fCanvas(canvas)
        , fPalette(palette)
        , fForegroundColor(foregroundColor)
        , fUpem(upem) {}

void ColorPainter::push_transform(const fontations_ffi::Transform& transform_arg) {
    fCanvas.save();
    fCanvas.concat(SkMatrixFromFontationsTransform(transform_arg));
}

void ColorPainter::pop_transform() { fCanvas.restore(); }

void ColorPainter::push_clip_glyph(uint16_t glyph_id) {
    fCanvas.save();
    SkPath path;
    fScalerContext.generateYScalePathForGlyphId(glyph_id, &path, fUpem);
    fCanvas.clipPath(path, true /* doAntialias */);
}

void ColorPainter::push_clip_rectangle(float x_min, float y_min, float x_max, float y_max) {
    fCanvas.save();
    SkRect clipRect = SkRect::MakeLTRB(x_min, -y_min, x_max, -y_max);
    fCanvas.clipRect(clipRect, true);
}

void ColorPainter::pop_clip() { fCanvas.restore(); }

void ColorPainter::configure_solid_paint(uint16_t palette_index, float alpha, SkPaint& paint) {
    SkColor4f color;
    if (palette_index == kForegroundColorPaletteIndex) {
        color = SkColor4f::FromColor(fForegroundColor);
    } else {
        color = SkColor4f::FromColor(fPalette[palette_index]);
    }
    color.fA *= alpha;
    paint.setShader(nullptr);
    paint.setColor(color);
}

void ColorPainter::fill_solid(uint16_t palette_index, float alpha) {
    SkPaint paint;
    configure_solid_paint(palette_index, alpha, paint);
    fCanvas.drawPaint(paint);
}

void ColorPainter::fill_glyph_solid(uint16_t glyph_id, uint16_t palette_index, float alpha) {
    SkPath path;
    fScalerContext.generateYScalePathForGlyphId(glyph_id, &path, fUpem);

    SkPaint paint;
    configure_solid_paint(palette_index, alpha, paint);
    fCanvas.drawPath(path, paint);
}

void ColorPainter::configure_linear_paint(const fontations_ffi::FillLinearParams& linear_params,
                                          fontations_ffi::BridgeColorStops& bridge_stops,
                                          uint8_t extend_mode,
                                          SkPaint& paint,
                                          SkMatrix* paintTransform) {
    std::vector<SkScalar> stops;
    std::vector<SkColor4f> colors;

    populateStopsAndColors(stops, colors, fPalette, fForegroundColor, bridge_stops);

    if (stops.size() == 1) {
        paint.setColor(colors[0]);
        return;
    }

    SkPoint linePositions[2] = {
            SkPoint::Make(SkFloatToScalar(linear_params.x0), -SkFloatToScalar(linear_params.y0)),
            SkPoint::Make(SkFloatToScalar(linear_params.x1), -SkFloatToScalar(linear_params.y1))};
    SkTileMode tileMode = ToSkTileMode(extend_mode);

    sk_sp<SkShader> shader(SkGradientShader::MakeLinear(
            linePositions,
            colors.data(),
            SkColorSpace::MakeSRGB(),
            stops.data(),
            stops.size(),
            tileMode,
            SkGradientShader::Interpolation{SkGradientShader::Interpolation::InPremul::kNo,
                                            SkGradientShader::Interpolation::ColorSpace::kSRGB,
                                            SkGradientShader::Interpolation::HueMethod::kShorter},
            paintTransform));

    SkASSERT(shader);
    // An opaque color is needed to ensure the gradient is not modulated by alpha.
    paint.setColor(SK_ColorBLACK);
    paint.setShader(shader);
}

void ColorPainter::fill_linear(const fontations_ffi::FillLinearParams& linear_params,
                               fontations_ffi::BridgeColorStops& bridge_stops,
                               uint8_t extend_mode) {
    SkPaint paint;

    configure_linear_paint(linear_params, bridge_stops, extend_mode, paint);

    fCanvas.drawPaint(paint);
}

void ColorPainter::fill_glyph_linear(uint16_t glyph_id,
                                     const fontations_ffi::Transform& transform,
                                     const fontations_ffi::FillLinearParams& linear_params,
                                     fontations_ffi::BridgeColorStops& bridge_stops,
                                     uint8_t extend_mode) {
    SkPath path;
    fScalerContext.generateYScalePathForGlyphId(glyph_id, &path, fUpem);

    SkPaint paint;
    SkMatrix paintTransform = SkMatrixFromFontationsTransform(transform);
    configure_linear_paint(linear_params, bridge_stops, extend_mode, paint, &paintTransform);
    fCanvas.drawPath(path, paint);
}

void ColorPainter::configure_radial_paint(
        const fontations_ffi::FillRadialParams& fill_radial_params,
        fontations_ffi::BridgeColorStops& bridge_stops,
        uint8_t extend_mode,
        SkPaint& paint,
        SkMatrix* paintTransform) {
    SkPoint start = SkPoint::Make(fill_radial_params.x0, -fill_radial_params.y0);
    SkPoint end = SkPoint::Make(fill_radial_params.x1, -fill_radial_params.y1);

    float startRadius = fill_radial_params.r0;
    float endRadius = fill_radial_params.r1;

    std::vector<SkScalar> stops;
    std::vector<SkColor4f> colors;

    populateStopsAndColors(stops, colors, fPalette, fForegroundColor, bridge_stops);

    // Draw single color if there's only one stop.
    if (stops.size() == 1) {
        paint.setColor(colors[0]);
        fCanvas.drawPaint(paint);
        return;
    }

    SkTileMode tileMode = ToSkTileMode(extend_mode);

    // For negative radii, interpolation is needed to prepare parameters suitable
    // for invoking the shader. Implementation below as resolution discussed in
    // https://github.com/googlefonts/colr-gradients-spec/issues/367.
    // Truncate to manually interpolated color for tile mode clamp, otherwise
    // calculate positive projected circles.
    if (startRadius < 0 || endRadius < 0) {
        if (startRadius == endRadius && startRadius < 0) {
            paint.setColor(SK_ColorTRANSPARENT);
            // return true;
            return;
        }

        if (tileMode == SkTileMode::kClamp) {
            SkVector startToEnd = end - start;
            SkScalar radiusDiff = endRadius - startRadius;
            SkScalar zeroRadiusStop = 0.f;
            TruncateStops truncateSide = TruncateStart;
            if (startRadius < 0) {
                truncateSide = TruncateStart;

                // Compute color stop position where radius is = 0.  After the scaling
                // of stop positions to the normal 0,1 range that we have done above,
                // the size of the radius as a function of the color stops is: r(x) = r0
                // + x*(r1-r0) Solving this function for r(x) = 0, we get: x = -r0 /
                // (r1-r0)
                zeroRadiusStop = -startRadius / (endRadius - startRadius);
                startRadius = 0.f;
                SkVector startEndDiff = end - start;
                startEndDiff.scale(zeroRadiusStop);
                start = start + startEndDiff;
            }

            if (endRadius < 0) {
                truncateSide = TruncateEnd;
                zeroRadiusStop = -startRadius / (endRadius - startRadius);
                endRadius = 0.f;
                SkVector startEndDiff = end - start;
                startEndDiff.scale(1 - zeroRadiusStop);
                end = end - startEndDiff;
            }

            if (!(startRadius == 0 && endRadius == 0)) {
                truncateToStopInterpolating(zeroRadiusStop, colors, stops, truncateSide);
            } else {
                // If both radii have become negative and where clamped to 0, we need to
                // produce a single color cone, otherwise the shader colors the whole
                // plane in a single color when two radii are specified as 0.
                if (radiusDiff > 0) {
                    end = start + startToEnd;
                    endRadius = radiusDiff;
                    colors.erase(colors.begin(), colors.end() - 1);
                    stops.erase(stops.begin(), stops.end() - 1);
                } else {
                    start -= startToEnd;
                    startRadius = -radiusDiff;
                    colors.erase(colors.begin() + 1, colors.end());
                    stops.erase(stops.begin() + 1, stops.end());
                }
            }
        } else {
            if (startRadius < 0 || endRadius < 0) {
                auto roundIntegerMultiple = [](SkScalar factorZeroCrossing, SkTileMode tileMode) {
                    int roundedMultiple = factorZeroCrossing > 0 ? ceilf(factorZeroCrossing)
                                                                 : floorf(factorZeroCrossing) - 1;
                    if (tileMode == SkTileMode::kMirror && roundedMultiple % 2 != 0) {
                        roundedMultiple += roundedMultiple < 0 ? -1 : 1;
                    }
                    return roundedMultiple;
                };

                SkVector startToEnd = end - start;
                SkScalar radiusDiff = endRadius - startRadius;
                SkScalar factorZeroCrossing = (startRadius / (startRadius - endRadius));
                bool inRange = 0.f <= factorZeroCrossing && factorZeroCrossing <= 1.0f;
                SkScalar direction = inRange && radiusDiff < 0 ? -1.0f : 1.0f;
                SkScalar circleProjectionFactor =
                        roundIntegerMultiple(factorZeroCrossing * direction, tileMode);
                startToEnd.scale(circleProjectionFactor);
                startRadius += circleProjectionFactor * radiusDiff;
                endRadius += circleProjectionFactor * radiusDiff;
                start += startToEnd;
                end += startToEnd;
            }
        }
    }

    // An opaque color is needed to ensure the gradient is not modulated by alpha.
    paint.setColor(SK_ColorBLACK);

    paint.setShader(SkGradientShader::MakeTwoPointConical(
            start,
            startRadius,
            end,
            endRadius,
            colors.data(),
            SkColorSpace::MakeSRGB(),
            stops.data(),
            stops.size(),
            tileMode,
            SkGradientShader::Interpolation{SkGradientShader::Interpolation::InPremul::kNo,
                                            SkGradientShader::Interpolation::ColorSpace::kSRGB,
                                            SkGradientShader::Interpolation::HueMethod::kShorter},
            paintTransform));
}

void ColorPainter::fill_radial(const fontations_ffi::FillRadialParams& fill_radial_params,
                               fontations_ffi::BridgeColorStops& bridge_stops,
                               uint8_t extend_mode) {
    SkPaint paint;

    configure_radial_paint(fill_radial_params, bridge_stops, extend_mode, paint);

    fCanvas.drawPaint(paint);
}

void ColorPainter::fill_glyph_radial(uint16_t glyph_id,
                                     const fontations_ffi::Transform& transform,
                                     const fontations_ffi::FillRadialParams& fill_radial_params,
                                     fontations_ffi::BridgeColorStops& bridge_stops,
                                     uint8_t extend_mode) {
    SkPath path;
    fScalerContext.generateYScalePathForGlyphId(glyph_id, &path, fUpem);

    SkPaint paint;
    SkMatrix paintTransform = SkMatrixFromFontationsTransform(transform);
    configure_radial_paint(fill_radial_params, bridge_stops, extend_mode, paint, &paintTransform);
    fCanvas.drawPath(path, paint);
}

void ColorPainter::configure_sweep_paint(const fontations_ffi::FillSweepParams& sweep_params,
                                         fontations_ffi::BridgeColorStops& bridge_stops,
                                         uint8_t extend_mode,
                                         SkPaint& paint,
                                         SkMatrix* paintTransform) {
    SkPoint center = SkPoint::Make(sweep_params.x0, -sweep_params.y0);

    std::vector<SkScalar> stops;
    std::vector<SkColor4f> colors;

    populateStopsAndColors(stops, colors, fPalette, fForegroundColor, bridge_stops);

    if (stops.size() == 1) {
        paint.setColor(colors[0]);
        fCanvas.drawPaint(paint);
        return;
    }

    // An opaque color is needed to ensure the gradient is not modulated by alpha.
    paint.setColor(SK_ColorBLACK);
    SkTileMode tileMode = ToSkTileMode(extend_mode);

    paint.setColor(SK_ColorBLACK);
    paint.setShader(SkGradientShader::MakeSweep(
            center.x(),
            center.y(),
            colors.data(),
            SkColorSpace::MakeSRGB(),
            stops.data(),
            stops.size(),
            tileMode,
            sweep_params.start_angle,
            sweep_params.end_angle,
            SkGradientShader::Interpolation{SkGradientShader::Interpolation::InPremul::kNo,
                                            SkGradientShader::Interpolation::ColorSpace::kSRGB,
                                            SkGradientShader::Interpolation::HueMethod::kShorter},
            paintTransform));
}

void ColorPainter::fill_sweep(const fontations_ffi::FillSweepParams& sweep_params,
                              fontations_ffi::BridgeColorStops& bridge_stops,
                              uint8_t extend_mode) {
    SkPaint paint;

    configure_sweep_paint(sweep_params, bridge_stops, extend_mode, paint);

    fCanvas.drawPaint(paint);
}

void ColorPainter::fill_glyph_sweep(uint16_t glyph_id,
                                    const fontations_ffi::Transform& transform,
                                    const fontations_ffi::FillSweepParams& sweep_params,
                                    fontations_ffi::BridgeColorStops& bridge_stops,
                                    uint8_t extend_mode) {
    SkPath path;
    fScalerContext.generateYScalePathForGlyphId(glyph_id, &path, fUpem);

    SkPaint paint;
    SkMatrix paintTransform = SkMatrixFromFontationsTransform(transform);
    configure_sweep_paint(sweep_params, bridge_stops, extend_mode, paint, &paintTransform);
    fCanvas.drawPath(path, paint);
}

void ColorPainter::push_layer(uint8_t compositeMode) {
    SkPaint paint;
    paint.setBlendMode(ToSkBlendMode(compositeMode));
    fCanvas.saveLayer(nullptr, &paint);
}

void ColorPainter::pop_layer() { fCanvas.restore(); }

BoundsPainter::BoundsPainter(SkFontationsScalerContext& scaler_context,
                             SkMatrix initialTransfom,
                             uint16_t upem)
        : fScalerContext(scaler_context)
        , fCurrentTransform(initialTransfom)
        , fUpem(upem)
        , fBounds(SkRect::MakeEmpty()) {}

SkRect BoundsPainter::getBoundingBox() { return fBounds; }

// fontations_ffi::ColorPainter interface.
void BoundsPainter::push_transform(const fontations_ffi::Transform& transform_arg) {
    SkMatrix transform = SkMatrix::MakeAll(transform_arg.xx,
                                           -transform_arg.xy,
                                           transform_arg.dx,
                                           -transform_arg.yx,
                                           transform_arg.yy,
                                           -transform_arg.dy,
                                           0.f,
                                           0.f,
                                           1.0f);
    fCurrentTransform.preConcat(transform);
    bool invertResult = transform.invert(&fStackTopTransformInverse);
    SkASSERT(invertResult);
}
void BoundsPainter::pop_transform() {
    fCurrentTransform.preConcat(fStackTopTransformInverse);
    fStackTopTransformInverse = SkMatrix();
}

void BoundsPainter::push_clip_glyph(uint16_t glyph_id) {
    SkPath path;
    fScalerContext.generateYScalePathForGlyphId(glyph_id, &path, fUpem);
    path.transform(fCurrentTransform);
    fBounds.join(path.getBounds());
}

void BoundsPainter::push_clip_rectangle(float x_min, float y_min, float x_max, float y_max) {
    SkRect clipRect = SkRect::MakeLTRB(x_min, -y_min, x_max, -y_max);
    SkPath rectPath = SkPath::Rect(clipRect);
    rectPath.transform(fCurrentTransform);
    fBounds.join(rectPath.getBounds());
}

void BoundsPainter::fill_glyph_solid(uint16_t glyph_id, uint16_t, float) {
    push_clip_glyph(glyph_id);
    pop_clip();
}

void BoundsPainter::fill_glyph_radial(uint16_t glyph_id,
                                      const fontations_ffi::Transform&,
                                      const fontations_ffi::FillRadialParams&,
                                      fontations_ffi::BridgeColorStops&,
                                      uint8_t) {
    push_clip_glyph(glyph_id);
    pop_clip();
}
void BoundsPainter::fill_glyph_linear(uint16_t glyph_id,
                                      const fontations_ffi::Transform&,
                                      const fontations_ffi::FillLinearParams&,
                                      fontations_ffi::BridgeColorStops&,
                                      uint8_t) {
    push_clip_glyph(glyph_id);
    pop_clip();
}

void BoundsPainter::fill_glyph_sweep(uint16_t glyph_id,
                                     const fontations_ffi::Transform&,
                                     const fontations_ffi::FillSweepParams&,
                                     fontations_ffi::BridgeColorStops&,
                                     uint8_t) {
    push_clip_glyph(glyph_id);
    pop_clip();
}

}  // namespace sk_fontations
