/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/ports/SkTypeface_fontations.h"

#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkImage.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkStream.h"
#include "include/effects/SkGradientShader.h"
#include "include/private/base/SkMutex.h"
#include "src/base/SkScopeExit.h"
#include "src/codec/SkCodecPriv.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkFontPriv.h"
#include "src/ports/SkTypeface_fontations_priv.h"
#include "src/ports/fontations/src/skpath_bridge.h"

namespace {

template <typename T> rust::Slice<T> toSlice(SkSpan<T> span) {
    return rust::Slice<T>(span.data(), span.size());
}

void CheckPng() {
#if defined(SK_DEBUG)
    if (!SkCodecs::HasDecoder("png")) {
        SkDebugf("No PNG decoder registered. A call to SkCodecs::Register is necessary.\n");
    }
#endif
}

[[maybe_unused]] static inline const constexpr bool kSkShowTextBlitCoverage = false;

sk_sp<const SkData> streamToData(const std::unique_ptr<SkStreamAsset>& font_data) {
    if (!font_data) {
        return SkData::MakeEmpty();
    }
    // TODO(drott): Remove this once SkData::MakeFromStream is able to do this itself.
    if (font_data->getData()) {
        return font_data->getData();
    }
    if (font_data->getMemoryBase() && font_data->getLength()) {
        return SkData::MakeWithCopy(font_data->getMemoryBase(), font_data->getLength());
    }
    font_data->rewind();
    return SkData::MakeFromStream(font_data.get(), font_data->getLength());
}

rust::Box<::fontations_ffi::BridgeFontRef> make_bridge_font_ref(sk_sp<const SkData> fontData,
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
        fontations_ffi::BridgeFontRef const& bridgeFontRef,
        const SkFontArguments::VariationPosition& variationPosition) {
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

bool isLCD(const SkScalerContextRec& rec) { return SkMask::kLCD16_Format == rec.fMaskFormat; }

bool bothZero(SkScalar a, SkScalar b) { return 0 == a && 0 == b; }

bool isAxisAligned(const SkScalerContextRec& rec) {
    return 0 == rec.fPreSkewX && (bothZero(rec.fPost2x2[0][1], rec.fPost2x2[1][0]) ||
                                  bothZero(rec.fPost2x2[0][0], rec.fPost2x2[1][1]));
}

}  // namespace

sk_sp<SkTypeface> SkTypeface_Make_Fontations(std::unique_ptr<SkStreamAsset> fontData,
                                             const SkFontArguments& args) {
    return SkTypeface_Fontations::MakeFromStream(std::move(fontData), args);
}

sk_sp<SkTypeface> SkTypeface_Make_Fontations(sk_sp<const SkData> fontData,
                                             const SkFontArguments& args) {
    return SkTypeface_Fontations::MakeFromData(std::move(fontData), args);
}

static_assert(
        sizeof(fontations_ffi::PaletteOverride) == sizeof(SkFontArguments::Palette::Override) &&
                sizeof(fontations_ffi::PaletteOverride::index) ==
                        sizeof(SkFontArguments::Palette::Override::index) &&
                sizeof(fontations_ffi::PaletteOverride::color_8888) ==
                        sizeof(SkFontArguments::Palette::Override::color),
        "Struct fontations_ffi::PaletteOverride must match SkFontArguments::Palette::Override.");

SkTypeface_Fontations::SkTypeface_Fontations(
        sk_sp<const SkData> fontData,
        const SkFontStyle& style,
        uint32_t ttcIndex,
        rust::Box<fontations_ffi::BridgeFontRef>&& fontRef,
        rust::Box<fontations_ffi::BridgeMappingIndex>&& mappingIndex,
        rust::Box<fontations_ffi::BridgeNormalizedCoords>&& normalizedCoords,
        rust::Box<fontations_ffi::BridgeOutlineCollection>&& outlines,
        rust::Box<fontations_ffi::BridgeGlyphStyles>&& glyphStyles,
        rust::Vec<uint32_t>&& palette)
        : SkTypeface(style, true)
        , fFontData(std::move(fontData))
        , fTtcIndex(ttcIndex)
        , fBridgeFontRef(std::move(fontRef))
        , fMappingIndex(std::move(mappingIndex))
        , fBridgeNormalizedCoords(std::move(normalizedCoords))
        , fOutlines(std::move(outlines))
        , fGlyphStyles(std::move(glyphStyles))
        , fPalette(std::move(palette)) {}

sk_sp<SkTypeface> SkTypeface_Fontations::MakeFromStream(std::unique_ptr<SkStreamAsset> stream,
                                                        const SkFontArguments& args) {
    return MakeFromData(streamToData(stream), args);
}

sk_sp<SkTypeface> SkTypeface_Fontations::MakeFromData(sk_sp<const SkData> data,
                                                      const SkFontArguments& args) {
    uint32_t ttcIndex = args.getCollectionIndex() & 0xFFFF;
    rust::Box<fontations_ffi::BridgeFontRef> bridgeFontRef = make_bridge_font_ref(data, ttcIndex);
    if (!fontations_ffi::font_ref_is_valid(*bridgeFontRef)) {
        return nullptr;
    }

    rust::Box<fontations_ffi::BridgeMappingIndex> mappingIndex =
            fontations_ffi::make_mapping_index(*bridgeFontRef);

    SkFontArguments::VariationPosition variationPosition = args.getVariationDesignPosition();
    std::unique_ptr<SkFontArguments::VariationPosition::Coordinate[]> concatenatedCoords = nullptr;
    // Handle FreeType behaviour of upper 15 bits of collection index
    // representing a named instance choice. If so, prepopulate the variation
    // coordinates with the values from the named instance and append the user
    // coordinates after that so they can override the named instance's
    // coordinates.
    if (args.getCollectionIndex() & 0xFFFF0000) {
        size_t numNamedInstanceCoords =
                fontations_ffi::coordinates_for_shifted_named_instance_index(
                        *bridgeFontRef,
                        args.getCollectionIndex(),
                        rust::cxxbridge1::Slice<fontations_ffi::SkiaDesignCoordinate>());
        concatenatedCoords.reset(
                new SkFontArguments::VariationPosition::Coordinate
                        [numNamedInstanceCoords + variationPosition.coordinateCount]);

        rust::cxxbridge1::Slice<fontations_ffi::SkiaDesignCoordinate> targetSlice(
                reinterpret_cast<fontations_ffi::SkiaDesignCoordinate*>(concatenatedCoords.get()),
                numNamedInstanceCoords);
        size_t retrievedNamedInstanceCoords =
                fontations_ffi::coordinates_for_shifted_named_instance_index(
                        *bridgeFontRef, args.getCollectionIndex(), targetSlice);
        if (numNamedInstanceCoords != retrievedNamedInstanceCoords) {
            return nullptr;
        }
        for (int i = 0; i < variationPosition.coordinateCount; ++i) {
            concatenatedCoords[numNamedInstanceCoords + i] = variationPosition.coordinates[i];
        }
        variationPosition.coordinateCount += numNamedInstanceCoords;
        variationPosition.coordinates = concatenatedCoords.get();
    }

    rust::Box<fontations_ffi::BridgeNormalizedCoords> normalizedCoords =
            make_normalized_coords(*bridgeFontRef, variationPosition);
    SkFontStyle style;
    fontations_ffi::BridgeFontStyle fontStyle;
    if (fontations_ffi::get_font_style(*bridgeFontRef, *normalizedCoords, fontStyle)) {
        style = SkFontStyle(fontStyle.weight,
                            fontStyle.width,
                            static_cast<SkFontStyle::Slant>(fontStyle.slant));
    }
    rust::Box<fontations_ffi::BridgeOutlineCollection> outlines =
            fontations_ffi::get_outline_collection(*bridgeFontRef);

    // Container for precomputed work for autohinting, see Skrifa's GlyphStyles.
    rust::Box<fontations_ffi::BridgeGlyphStyles> glyphStyles =
            fontations_ffi::get_bridge_glyph_styles();

    rust::Slice<const fontations_ffi::PaletteOverride> paletteOverrides(
            reinterpret_cast<const ::fontations_ffi::PaletteOverride*>(args.getPalette().overrides),
            args.getPalette().overrideCount);
    rust::Vec<uint32_t> palette =
            resolve_palette(*bridgeFontRef, args.getPalette().index, paletteOverrides);

    return sk_sp<SkTypeface>(new SkTypeface_Fontations(data,
                                                       style,
                                                       ttcIndex,
                                                       std::move(bridgeFontRef),
                                                       std::move(mappingIndex),
                                                       std::move(normalizedCoords),
                                                       std::move(outlines),
                                                       std::move(glyphStyles),
                                                       std::move(palette)));
}

namespace sk_fontations {

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

void SkTypeface_Fontations::onCharsToGlyphs(SkSpan<const SkUnichar> chars,
                                            SkSpan<SkGlyphID> glyphs) const {
    SkASSERT(chars.size() == glyphs.size());
    rust::Slice<const uint32_t> codepointSlice{reinterpret_cast<const uint32_t*>(chars.data()),
                                               chars.size()};
    fontations_ffi::lookup_glyph_or_zero(*fBridgeFontRef, *fMappingIndex,
                                         codepointSlice, toSlice(glyphs));
}
int SkTypeface_Fontations::onCountGlyphs() const {
    return fontations_ffi::num_glyphs(*fBridgeFontRef);
}

void SkTypeface_Fontations::getGlyphToUnicodeMap(SkSpan<SkUnichar> codepointForGlyphMap) const {
    size_t numGlyphs = std::min(SkToSizeT(onCountGlyphs()), codepointForGlyphMap.size());
    rust::Slice<uint32_t> codepointForGlyphSlice
        {reinterpret_cast<uint32_t*>(codepointForGlyphMap.data()), numGlyphs};
    fontations_ffi::fill_glyph_to_unicode_map(*fBridgeFontRef, codepointForGlyphSlice);
}

void SkTypeface_Fontations::onFilterRec(SkScalerContextRec* rec) const {
    rec->useStrokeForFakeBold();

    // See https://issues.skia.org/issues/396360753
    // We would like Fontations anti-aliasing on a surface with unknown pixel geometry to
    // look like the FreeType backend in order to avoid perceived regressions
    // in sharpness, so we ignore SkScalerContext::kGenA8FromLCD_Flag in fRec.fFlags.
    rec->fFlags &= ~SkScalerContext::kGenA8FromLCD_Flag;

    // Opportunistic hinting downgrades copied from SkFontHost_FreeType.cpp
    SkFontHinting h = rec->getHinting();
    if (SkFontHinting::kFull == h && !isLCD(*rec)) {
        // Collapse full->normal hinting if we're not doing LCD.
        h = SkFontHinting::kNormal;
    }

    // Rotated text looks bad with hinting, so we disable it as needed.
    if (!isAxisAligned(*rec)) {
        h = SkFontHinting::kNone;
    }
    rec->setHinting(h);
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
    SkFontationsScalerContext(const SkTypeface_Fontations& realTypeface,
                              const SkScalerContextEffects& effects,
                              const SkDescriptor* desc,
                              SkTypeface& proxyTypeface)
            : SkScalerContext(proxyTypeface, effects, desc) // proxyTypeface owns the realTypeface
            , fBridgeFontRef(realTypeface.getBridgeFontRef())
            , fBridgeNormalizedCoords(realTypeface.getBridgeNormalizedCoords())
            , fOutlines(realTypeface.getOutlines())
            , fGlyphStyles(realTypeface.getGlyphStyles())
            , fMappingIndex(realTypeface.getMappingIndex())
            , fPalette(realTypeface.getPalette())
            , fHintingInstance(fontations_ffi::no_hinting_instance()) {
        using AutoHinting = fontations_ffi::AutoHintingControl;

        fRec.computeMatrices(
                SkScalerContextRec::PreMatrixScale::kVertical, &fScale, &fRemainingMatrix);

        fDoLinearMetrics = this->isLinearMetrics();

        // See below for the exception for SkFontHinting::kSlight.
        bool forceAutoHinting = SkToBool(fRec.fFlags & kForceAutohinting_Flag);
        AutoHinting autoHintingControl = forceAutoHinting ? AutoHinting::ForceForGlyfAndCff
                                                          : AutoHinting::Fallback;

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
        // On Android, match the FreeType backend and disable autohinting completely
        // unless the force flag is set.
        if (!forceAutoHinting)
            autoHintingControl = AutoHinting::ForceOff;
#endif

        // Hinting-reliant fonts exist that display incorrect contours when not executing their
        // hinting instructions. Detect those and force-enable hinting for them.
        if (fontations_ffi::hinting_reliant(fOutlines)) {
            fHintingInstance = fontations_ffi::make_mono_hinting_instance(
                    fOutlines, fScale.y(), fBridgeNormalizedCoords);
            fDoLinearMetrics = false;
        } else {
            if (SkMask::kBW_Format == fRec.fMaskFormat) {
                if (fRec.getHinting() == SkFontHinting::kNone) {
                    fHintingInstance = fontations_ffi::no_hinting_instance();
                    fDoLinearMetrics = true;
                } else {
                    fHintingInstance = fontations_ffi::make_mono_hinting_instance(
                            fOutlines, fScale.y(), fBridgeNormalizedCoords);
                    fDoLinearMetrics = false;
                }
            } else {
                switch (fRec.getHinting()) {
                    case SkFontHinting::kNone:
                        fHintingInstance = fontations_ffi::no_hinting_instance();
                        fDoLinearMetrics = true;
                        break;
                    case SkFontHinting::kSlight:
                        // Unhinted metrics.
                        if (autoHintingControl != AutoHinting::ForceForGlyfAndCff &&
                            autoHintingControl != AutoHinting::ForceOff)
                        {
                            autoHintingControl = AutoHinting::ForceForGlyf;
                        }
                        fHintingInstance = fontations_ffi::make_hinting_instance(
                                fOutlines,
                                fGlyphStyles,
                                fScale.y(),
                                fBridgeNormalizedCoords,
                                true /* do_light_hinting */,
                                false /* do_lcd_antialiasing */,
                                false /* lcd_orientation_vertical */,
                                autoHintingControl);
                        fDoLinearMetrics = true;
                        break;
                    case SkFontHinting::kNormal:
                        // No hinting to subpixel coordinates.
                        fHintingInstance = fontations_ffi::make_hinting_instance(
                                fOutlines,
                                fGlyphStyles,
                                fScale.y(),
                                fBridgeNormalizedCoords,
                                false /* do_light_hinting */,
                                false /* do_lcd_antialiasing */,
                                false /* lcd_orientation_vertical */,
                                autoHintingControl);
                        break;
                    case SkFontHinting::kFull:
                        // Attempt to make use of hinting to subpixel coordinates.
                        fHintingInstance = fontations_ffi::make_hinting_instance(
                                fOutlines,
                                fGlyphStyles,
                                fScale.y(),
                                fBridgeNormalizedCoords,
                                false /* do_light_hinting */,
                                isLCD(fRec) /* do_lcd_antialiasing */,
                                SkToBool(fRec.fFlags &
                                         SkScalerContext::
                                                 kLCD_Vertical_Flag) /* lcd_orientation_vertical */,
                                autoHintingControl);
                }
            }
        }
    }

    std::optional<SkScalar> getContourHeightForLetter(SkUnichar letter) {
        SkGlyphID glyphId;
        rust::Slice<const uint32_t> codepointSlice{reinterpret_cast<const uint32_t*>(&letter), 1};
        rust::Slice<uint16_t> glyphSlice{reinterpret_cast<uint16_t*>(&glyphId), 1};
        fontations_ffi::lookup_glyph_or_zero(fBridgeFontRef, fMappingIndex,
                                             codepointSlice, glyphSlice);
        if (!glyphId) {
            return {};
        }
        fontations_ffi::BridgeScalerMetrics scalerMetrics;
        if (auto glyphPath = generatePathForGlyphId(glyphId, fScale.y(),
                                                    *fHintingInstance, scalerMetrics)) {
            return glyphPath->getBounds().height();
        }
        return {};
    }

    // yScale is only used if hintinInstance is set to Unhinted,
    // otherwise the size is controlled by the configured hintingInstance.
    // hintingInstance argument is needed as COLRv1 drawing performs unhinted,
    // unscaled path retrieval.
    std::optional<SkPath> generatePathForGlyphId(uint16_t glyphId,
                                float yScale,
                                const fontations_ffi::BridgeHintingInstance& hintingInstance,
                                fontations_ffi::BridgeScalerMetrics& scalerMetrics) {
        // See https://crbug.com/390889644 - while SkScalerContexts are single-thread
        // in general, generateYScalePathForGlyphId() can be called from a COLRv1
        // SkDrawable as well (see generateDrawable()). For this reason access to the
        // path extraction array needs to be made thread-safe here.
        SkAutoMutexExclusive l(fPathMutex);
        // Keep allocations in check. The rust side pre-allocates a fixed amount,
        // and afer leaving this function, we shrink to the same amount.
        SK_AT_SCOPE_EXIT(fontations_ffi::shrink_verbs_points_if_needed(fPathVerbs, fPathPoints));

        if (!fontations_ffi::get_path_verbs_points(fOutlines,
                                                   glyphId,
                                                   yScale,
                                                   fBridgeNormalizedCoords,
                                                   hintingInstance,
                                                   fPathVerbs,
                                                   fPathPoints,
                                                   scalerMetrics)) {
            return {};
        }

        // See https://issues.skia.org/345178242 for details:
        // The FreeType backend performs a path simplification here based on the
        // equivalent of what we have here as scalerMetrics.has_overlaps
        // Since PathOps::Simplify fails or at times produces incorrect simplified
        // contours, skip that step here.
        return SkPath::Make({reinterpret_cast<const SkPoint*>(fPathPoints.data()),
                             fPathPoints.size()},
                            fPathVerbs,
                            {},
                            SkPathFillType::kWinding);
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

        bool has_colrv1_glyph = false;
        bool has_colrv0_glyph = false;
        bool has_bitmap_glyph = false;
        if (fontations_ffi::has_any_color_table(fBridgeFontRef)) {
            has_colrv1_glyph = fontations_ffi::has_colrv1_glyph(fBridgeFontRef, glyph.getGlyphID());
            has_colrv0_glyph = fontations_ffi::has_colrv0_glyph(fBridgeFontRef, glyph.getGlyphID());
            has_bitmap_glyph = fontations_ffi::has_bitmap_glyph(fBridgeFontRef, glyph.getGlyphID());
        }

        // Local overrides for color fonts etc. may alter the request for linear metrics.
        bool doLinearMetrics = fDoLinearMetrics;

        if (has_bitmap_glyph) {
            // Bitmap advance metrics can originate from different strike sizes in the bitmap
            // font and are thus not linearly scaling with font size.
            doLinearMetrics = false;
        }
        if (has_colrv0_glyph || has_colrv1_glyph) {
            // We prefer color vector glyphs, and hinting is disabled for those.
            doLinearMetrics = true;
        }

        float x_advance = 0.0f;
        x_advance = fontations_ffi::unhinted_advance_width_or_zero(
                fBridgeFontRef, fScale.y(), fBridgeNormalizedCoords, glyph.getGlyphID());
        if (!doLinearMetrics) {
            fontations_ffi::BridgeScalerMetrics scalerMetrics;
            if (auto generatedPath = generatePathImpl(glyph.getGlyphID(),
                                                      scalerMetrics)) {
                mx.generatedPath = std::move(*generatedPath);

                if (scalerMetrics.has_adjusted_advance) {
                    // FreeType rounds the advance to full pixels when in hinting modes.
                    // Match FreeType and round here.
                    // See
                    // * https://gitlab.freedesktop.org/freetype/freetype/-/blob/57617782464411201ce7bbc93b086c1b4d7d84a5/src/autofit/afloader.c#L422
                    // * https://gitlab.freedesktop.org/freetype/freetype/-/blob/57617782464411201ce7bbc93b086c1b4d7d84a5/src/truetype/ttgload.c#L823
                    float hinted_advance = roundf(scalerMetrics.adjusted_advance);
                    // TODO(drott): Remove this workaround for fontations returning 0
                    // for a space glyph without contours, compare
                    // https://github.com/googlefonts/fontations/issues/905
                    if (hinted_advance != x_advance && hinted_advance != 0) {
                        x_advance = hinted_advance;
                    }
                }
            }
        }
        mx.advance = fRemainingMatrix.mapPoint({x_advance, 0});

        if (has_colrv1_glyph || has_colrv0_glyph) {
            mx.extraBits = has_colrv1_glyph ? ScalerContextBits::COLRv1 : ScalerContextBits::COLRv0;
            mx.maskFormat = SkMask::kARGB32_Format;
            mx.neverRequestPath = true;

            fontations_ffi::ClipBox clipBox;
            if (has_colrv1_glyph && fontations_ffi::get_colrv1_clip_box(fBridgeFontRef,
                                                                        fBridgeNormalizedCoords,
                                                                        glyph.getGlyphID(),
                                                                        fScale.y(),
                                                                        clipBox)) {
                // Flip y.
                SkRect boundsRect = SkRect::MakeLTRB(
                        clipBox.x_min, -clipBox.y_max, clipBox.x_max, -clipBox.y_min);

                if (!fRemainingMatrix.isIdentity()) {
                    boundsRect = fRemainingMatrix.mapRect(boundsRect);
                }

                boundsRect.roundOut(&mx.bounds);

            } else {
                uint16_t upem = fontations_ffi::units_per_em_or_zero(fBridgeFontRef);
                if (upem == 0) {
                    mx.bounds = SkRect::MakeEmpty();
                } else {
                    SkMatrix fullTransform = fRec.getSingleMatrix();
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
                    fontations_ffi::bitmap_glyph(fBridgeFontRef, glyph.getGlyphID(), fScale.y());
            rust::cxxbridge1::Slice<const uint8_t> png_data =
                    fontations_ffi::png_data(*bitmap_glyph);

            if (png_data.empty()) {
                return mx;
            }

            const fontations_ffi::BitmapMetrics bitmapMetrics =
                    fontations_ffi::bitmap_metrics(*bitmap_glyph);

            sk_sp<SkImage> img = SkImages::DeferredFromEncodedData(
                    SkData::MakeWithoutCopy(png_data.data(), png_data.size()));
            if (!img) {
                CheckPng();
                return mx;
            }

            SkImageInfo info = img->imageInfo();

            SkRect bounds = SkRect::Make(info.bounds());
            SkMatrix matrix = fRemainingMatrix;

            // We deal with two scale factors here: Scaling from font units to
            // device pixels, and scaling the embedded PNG from its number of
            // rows to a specific size, depending on the ppem values in the
            // bitmap glyph information.
            float imageToSize = fScale.y() / bitmapMetrics.ppem_y;
            float fontUnitsToSize = fScale.y() /
                                    fontations_ffi::units_per_em_or_zero(fBridgeFontRef);

            // The offset from origin is given in font units, so requires a
            // different scale factor than the scaling of the image.
            matrix.preTranslate( bitmapMetrics.bearing_x * fontUnitsToSize,
                                -bitmapMetrics.bearing_y * fontUnitsToSize);
            matrix.preScale(imageToSize, imageToSize);
            matrix.preTranslate( bitmapMetrics.inner_bearing_x,
                                -bitmapMetrics.inner_bearing_y);

            // For sbix bitmap glyphs, the origin is the bottom left of the image.
            float heightAdjustment =
                    bitmapMetrics.placement_origin_bottom_left ? bounds.height() : 0;
            matrix.preTranslate(0, -heightAdjustment);

            if (this->isSubpixel()) {
                matrix.postTranslate(SkFixedToScalar(glyph.getSubXFixed()),
                                     SkFixedToScalar(glyph.getSubYFixed()));
            }
            matrix.mapRect(&bounds);
            mx.bounds = SkRect::Make(bounds.roundOut());

            if (SkIsFinite(bitmapMetrics.advance)) {
                mx.advance = matrix.mapVector(bitmapMetrics.advance, 0);
            }
        } else {
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

        rust::cxxbridge1::Box<fontations_ffi::BridgeBitmapGlyph> bitmap_glyph =
                fontations_ffi::bitmap_glyph(fBridgeFontRef, glyph.getGlyphID(), fScale.y());
        rust::cxxbridge1::Slice<const uint8_t> png_data = fontations_ffi::png_data(*bitmap_glyph);
        SkASSERT(png_data.size());

        sk_sp<SkImage> glyph_image = SkImages::DeferredFromEncodedData(
                SkData::MakeWithoutCopy(png_data.data(), png_data.size()));
        if (!glyph_image) {
            CheckPng();
            return;
        }

        canvas.clear(SK_ColorTRANSPARENT);
        canvas.concat(fRemainingMatrix);

        if (this->isSubpixel()) {
            canvas.translate(SkFixedToScalar(glyph.getSubXFixed()),
                             SkFixedToScalar(glyph.getSubYFixed()));
        }
        const fontations_ffi::BitmapMetrics bitmapMetrics =
                fontations_ffi::bitmap_metrics(*bitmap_glyph);

        // We need two different scale factors here, one for font units to size,
        // one for scaling the embedded PNG, see generateMetrics() for details.
        SkScalar imageScaleFactor = fScale.y() / bitmapMetrics.ppem_y;

        float fontUnitsToSize = fScale.y() / fontations_ffi::units_per_em_or_zero(fBridgeFontRef);
        canvas.translate( bitmapMetrics.bearing_x * fontUnitsToSize,
                         -bitmapMetrics.bearing_y * fontUnitsToSize);
        canvas.scale(imageScaleFactor, imageScaleFactor);
        canvas.translate( bitmapMetrics.inner_bearing_x,
                         -bitmapMetrics.inner_bearing_y);

        float heightAdjustment =
                bitmapMetrics.placement_origin_bottom_left ? glyph_image->height() : 0;

        canvas.translate(0, -heightAdjustment);

        SkSamplingOptions sampling(SkFilterMode::kLinear, SkMipmapMode::kNearest);
        canvas.drawImage(glyph_image, 0, 0, sampling);
    }

    void generateImage(const SkGlyph& glyph, void* imageBuffer) override {
        ScalerContextBits::value_type format = glyph.extraBits();
        if (format == ScalerContextBits::PATH) {
            this->generateImageFromPath(glyph, imageBuffer);
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

    std::optional<GeneratedPath> generatePathImpl(SkGlyphID glyphId,
                          fontations_ffi::BridgeScalerMetrics& scalerMetrics) {
        if (auto path = generatePathForGlyphId(glyphId, fScale.y(),
                                               *fHintingInstance, scalerMetrics)) {
            if (auto newpath = path->tryMakeTransform(fRemainingMatrix)) {
                return {{
                    *newpath,
                    !fRemainingMatrix.isIdentity()
                }};
            }
        }
        return {};
    }

    // For hinted glyphs, generateMetrics provides a shortcut to set a generated path on
    // SkGlyph - so this method will not be called when a path has already been set.
    std::optional<GeneratedPath> generatePath(const SkGlyph& glyph) override {
        SkASSERT(glyph.extraBits() == ScalerContextBits::PATH);
        fontations_ffi::BridgeScalerMetrics scalerMetrics;
        return generatePathImpl(glyph.getGlyphID(), scalerMetrics);
    }

    bool drawCOLRGlyph(const SkGlyph& glyph, SkColor foregroundColor, SkCanvas* canvas) {
        uint16_t upem = fontations_ffi::units_per_em_or_zero(fBridgeFontRef);
        if (upem == 0) {
            return false;
        }

        SkMatrix scalerMatrix = fRec.getSingleMatrix();
        SkAutoCanvasRestore autoRestore(canvas, true /* doSave */);

        // Scale down so that COLR operations can happen in glyph coordinates.
        SkMatrix upemToPpem = SkMatrix::Scale(1.f / upem, 1.f / upem);
        scalerMatrix.preConcat(upemToPpem);
        canvas->concat(scalerMatrix);
        SkPaint defaultPaint;
        defaultPaint.setColor(SK_ColorRED);
        sk_fontations::ColorPainter colorPainter(*this, *canvas, fPalette, foregroundColor,
                                                 SkMask::kBW_Format != fRec.fMaskFormat, upem);
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
        fontations_ffi::Metrics metrics =
                fontations_ffi::get_skia_metrics(fBridgeFontRef, fScale.y(), fBridgeNormalizedCoords);
        out_metrics->fTop = -metrics.top;
        out_metrics->fAscent = -metrics.ascent;
        out_metrics->fDescent = -metrics.descent;
        out_metrics->fBottom = -metrics.bottom;
        out_metrics->fLeading = metrics.leading;
        out_metrics->fAvgCharWidth = metrics.avg_char_width;
        out_metrics->fMaxCharWidth = metrics.max_char_width;
        out_metrics->fXMin = metrics.x_min;
        out_metrics->fXMax = metrics.x_max;
        out_metrics->fXHeight = -metrics.x_height;
        out_metrics->fCapHeight = -metrics.cap_height;
        out_metrics->fFlags = 0;

        // Cap height synthesis.
        if (!out_metrics->fCapHeight) {
            if (auto height = getContourHeightForLetter('H')) {
                out_metrics->fCapHeight = *height;
            } else  {
                out_metrics->fCapHeight = metrics.ascent;
            }
        }

        if (!out_metrics->fXHeight) {
            if (auto xHeight = getContourHeightForLetter('x')) {
                out_metrics->fXHeight = *xHeight;
            } else {
                out_metrics->fXHeight = metrics.ascent;
            }
        }

        if (fontations_ffi::table_data(fBridgeFontRef,
                                       SkSetFourByteTag('f', 'v', 'a', 'r'),
                                       0,
                                       rust::Slice<uint8_t>())) {
            out_metrics->fFlags |= SkFontMetrics::kBoundsInvalid_Flag;
        }
        auto setMetric = [](float& dstMetric, const float srcMetric,
                            uint32_t& flags, const SkFontMetrics::FontMetricsFlags flag)
        {
            if (std::isnan(srcMetric)) {
                dstMetric = 0;
            } else {
                dstMetric = srcMetric;
                flags |= flag;
            }
        };
        setMetric(out_metrics->fUnderlinePosition, -metrics.underline_position,
                  out_metrics->fFlags, SkFontMetrics::kUnderlinePositionIsValid_Flag);
        setMetric(out_metrics->fUnderlineThickness, metrics.underline_thickness,
                  out_metrics->fFlags, SkFontMetrics::kUnderlineThicknessIsValid_Flag);

        setMetric(out_metrics->fStrikeoutPosition, -metrics.strikeout_position,
                  out_metrics->fFlags, SkFontMetrics::kStrikeoutPositionIsValid_Flag);
        setMetric(out_metrics->fStrikeoutThickness, metrics.strikeout_thickness,
                  out_metrics->fFlags, SkFontMetrics::kStrikeoutThicknessIsValid_Flag);
    }

private:
    SkVector fScale;
    SkMatrix fRemainingMatrix;
    sk_sp<SkData> fFontData = nullptr;
    const fontations_ffi::BridgeFontRef& fBridgeFontRef;
    const fontations_ffi::BridgeNormalizedCoords& fBridgeNormalizedCoords;
    const fontations_ffi::BridgeOutlineCollection& fOutlines;
    const fontations_ffi::BridgeGlyphStyles& fGlyphStyles;
    const fontations_ffi::BridgeMappingIndex& fMappingIndex;
    const SkSpan<const SkColor> fPalette;
    rust::Box<fontations_ffi::BridgeHintingInstance> fHintingInstance;
    bool fDoLinearMetrics = false;
    // Keeping the path extraction target buffers around significantly avoids
    // allocation churn.
    SkMutex fPathMutex;
    rust::Vec<uint8_t> fPathVerbs;
    rust::Vec<fontations_ffi::FfiPoint> fPathPoints;

    friend class sk_fontations::ColorPainter;
};

std::unique_ptr<SkStreamAsset> SkTypeface_Fontations::onOpenStream(int* ttcIndex) const {
    *ttcIndex = fTtcIndex;
    return std::make_unique<SkMemoryStream>(fFontData);
}

sk_sp<SkTypeface> SkTypeface_Fontations::onMakeClone(const SkFontArguments& args) const {
    // Matching DWrite implementation, return self if ttc index mismatches.
    if (fTtcIndex != SkTo<uint32_t>(args.getCollectionIndex())) {
        return sk_ref_sp(this);
    }

    int numAxes = onGetVariationDesignPosition({});
    auto fusedDesignPosition =
            std::make_unique<SkFontArguments::VariationPosition::Coordinate[]>(numAxes);
    int retrievedAxes = onGetVariationDesignPosition({fusedDesignPosition.get(), (size_t)numAxes});
    if (numAxes != retrievedAxes) {
        return nullptr;
    }

    // We know the internally retrieved axes are normalized, contain a value for every possible
    // axis, other axes do not exist, so we only need to override any of those.
    for (int i = 0; i < numAxes; ++i) {
        const SkFontArguments::VariationPosition& argPosition = args.getVariationDesignPosition();
        for (int j = 0; j < argPosition.coordinateCount; ++j) {
            if (fusedDesignPosition[i].axis == argPosition.coordinates[j].axis) {
                fusedDesignPosition[i].value = argPosition.coordinates[j].value;
            }
        }
    }

    SkFontArguments fusedArgs;
    fusedArgs.setVariationDesignPosition({fusedDesignPosition.get(), SkToInt(numAxes)});
    fusedArgs.setPalette(args.getPalette());

    rust::cxxbridge1::Box<fontations_ffi::BridgeNormalizedCoords> normalized_args =
            make_normalized_coords(*fBridgeFontRef, fusedArgs.getVariationDesignPosition());

    if (!fontations_ffi::normalized_coords_equal(*normalized_args, *fBridgeNormalizedCoords)) {
        return MakeFromData(fFontData, fusedArgs);
    }

    // TODO(skbug.com/330149870): Palette differences are not fused, see DWrite backend impl.
    rust::Slice<const fontations_ffi::PaletteOverride> argPaletteOverrides(
            reinterpret_cast<const fontations_ffi::PaletteOverride*>(args.getPalette().overrides),
            args.getPalette().overrideCount);
    rust::Vec<uint32_t> newPalette =
            resolve_palette(*fBridgeFontRef, args.getPalette().index, argPaletteOverrides);

    if (fPalette.size() != newPalette.size() ||
        memcmp(fPalette.data(), newPalette.data(), fPalette.size() * sizeof(fPalette[0]))) {
        return MakeFromData(fFontData, fusedArgs);
    }

    return sk_ref_sp(this);
}

std::unique_ptr<SkScalerContext> SkTypeface_Fontations::onCreateScalerContext(
        const SkScalerContextEffects& effects, const SkDescriptor* desc) const {
    return this->onCreateScalerContextAsProxyTypeface(effects, desc, nullptr);
}

std::unique_ptr<SkScalerContext> SkTypeface_Fontations::onCreateScalerContextAsProxyTypeface(
                                    const SkScalerContextEffects& effects,
                                    const SkDescriptor* desc,
                                    SkTypeface* proxyTypeface) const {
    return std::make_unique<SkFontationsScalerContext>(
            *this,
            effects,
            desc,
            proxyTypeface ? *proxyTypeface : *const_cast<SkTypeface_Fontations*>(this));
}

std::unique_ptr<SkAdvancedTypefaceMetrics> SkTypeface_Fontations::onGetAdvancedMetrics() const {
    std::unique_ptr<SkAdvancedTypefaceMetrics> info(new SkAdvancedTypefaceMetrics);

    if (!fontations_ffi::is_embeddable(*fBridgeFontRef)) {
        info->fFlags |= SkAdvancedTypefaceMetrics::kNotEmbeddable_FontFlag;
    }

    if (!fontations_ffi::is_subsettable(*fBridgeFontRef)) {
        info->fFlags |= SkAdvancedTypefaceMetrics::kNotSubsettable_FontFlag;
    }

    if (fontations_ffi::table_data(
                *fBridgeFontRef, SkSetFourByteTag('f', 'v', 'a', 'r'), 0, rust::Slice<uint8_t>())) {
        info->fFlags |= SkAdvancedTypefaceMetrics::kVariable_FontFlag;
    }

    switch (fontations_ffi::outline_format(*fOutlines)) {
        case fontations_ffi::OutlineFormat::Glyf:
            info->fType = SkAdvancedTypefaceMetrics::kTrueType_Font;
            break;
        case fontations_ffi::OutlineFormat::Cff:
            info->fType = SkAdvancedTypefaceMetrics::kCFF_Font;
            break;
        default:
            // leave info->fType SkAdvancedTypefaceMetrics::kOther;
            break;
    }

    // Metrics information.
    fontations_ffi::Metrics metrics =
            fontations_ffi::get_unscaled_metrics(*fBridgeFontRef, *fBridgeNormalizedCoords);
    info->fAscent = metrics.ascent;
    info->fDescent = metrics.descent;
    info->fCapHeight = metrics.cap_height;

    info->fBBox = SkIRect::MakeLTRB((int32_t)metrics.x_min,
                                    (int32_t)metrics.top,
                                    (int32_t)metrics.x_max,
                                    (int32_t)metrics.bottom);

    // Style information.
    if (fontations_ffi::is_fixed_pitch(*fBridgeFontRef)) {
        info->fStyle |= SkAdvancedTypefaceMetrics::kFixedPitch_Style;
    }

    rust::String readPsName;
    if (fontations_ffi::postscript_name(*fBridgeFontRef, readPsName)) {
        info->fPostScriptName = SkString(readPsName.data(), readPsName.size());
    }

    fontations_ffi::BridgeFontStyle fontStyle;
    if (fontations_ffi::get_font_style(*fBridgeFontRef, *fBridgeNormalizedCoords, fontStyle)) {
        if (fontStyle.slant == SkFontStyle::Slant::kItalic_Slant) {
            info->fStyle |= SkAdvancedTypefaceMetrics::kItalic_Style;
        }
    }

    if (fontations_ffi::is_serif_style(*fBridgeFontRef)) {
        info->fStyle |= SkAdvancedTypefaceMetrics::kSerif_Style;
    } else if (fontations_ffi::is_script_style(*fBridgeFontRef)) {
        info->fStyle |= SkAdvancedTypefaceMetrics::kScript_Style;
    }

    info->fItalicAngle = fontations_ffi::italic_angle(*fBridgeFontRef);

    return info;
}

void SkTypeface_Fontations::onGetFontDescriptor(SkFontDescriptor* desc, bool* serialize) const {
    SkString familyName;
    onGetFamilyName(&familyName);
    desc->setFamilyName(familyName.c_str());
    desc->setStyle(this->fontStyle());
    desc->setFactoryId(FactoryId);

    // TODO: keep the index to emit here
    desc->setPaletteIndex(0);
    SkSpan<const SkColor> palette = getPalette();
    // TODO: omit override when palette[n] == CPAL[paletteIndex][n]
    size_t paletteOverrideCount = palette.size();
    auto overrides = desc->setPaletteEntryOverrides(paletteOverrideCount);
    for (size_t i = 0; i < paletteOverrideCount; ++i) {
        overrides[i] = {(uint16_t)i, palette[i]};
    }

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

int SkTypeface_Fontations::onGetTableTags(SkSpan<SkFontTableTag> tags) const {
    uint16_t numTables = fontations_ffi::table_tags(*fBridgeFontRef, rust::Slice<uint32_t>());
    if (tags.empty()) {
        return numTables;
    }
    const size_t n = std::min<size_t>(numTables, tags.size());
    return fontations_ffi::table_tags(*fBridgeFontRef, toSlice(tags.first(n)));
}

int SkTypeface_Fontations::onGetVariationDesignPosition(
        SkSpan<SkFontArguments::VariationPosition::Coordinate> coordinates) const {
    rust::Slice<fontations_ffi::SkiaDesignCoordinate> copyToCoordinates;
    if (!coordinates.empty()) {
        copyToCoordinates = rust::Slice<fontations_ffi::SkiaDesignCoordinate>(
                reinterpret_cast<fontations_ffi::SkiaDesignCoordinate*>(coordinates.data()),
                coordinates.size());
    }
    return fontations_ffi::variation_position(*fBridgeNormalizedCoords, copyToCoordinates);
}

int SkTypeface_Fontations::onGetVariationDesignParameters(
        SkSpan<SkFontParameters::Variation::Axis> parameters) const {
    sk_fontations::AxisWrapper axisWrapper(parameters.data(), parameters.size());
    return fontations_ffi::populate_axes(*fBridgeFontRef, axisWrapper);
}

namespace sk_fontations {

namespace {

const uint16_t kForegroundColorPaletteIndex = 0xFFFF;

void populateStopsAndColors(std::vector<SkScalar>& dest_stops,
                            std::vector<SkColor4f>& dest_colors,
                            const SkSpan<const SkColor>& palette,
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
                           SkSpan<const SkColor> palette,
                           SkColor foregroundColor,
                           bool antialias,
                           uint16_t upem)
        : fScalerContext(scaler_context)
        , fCanvas(canvas)
        , fPalette(palette)
        , fForegroundColor(foregroundColor)
        , fAntialias(antialias)
        , fUpem(upem) {}

void ColorPainter::push_transform(const fontations_ffi::Transform& transform_arg) {
    fCanvas.save();
    fCanvas.concat(SkMatrixFromFontationsTransform(transform_arg));
}

void ColorPainter::pop_transform() { fCanvas.restore(); }

void ColorPainter::push_clip_glyph(uint16_t glyph_id) {
    fCanvas.save();
    fontations_ffi::BridgeScalerMetrics scalerMetrics;
    auto path = fScalerContext.generatePathForGlyphId(glyph_id, fUpem,
                                                      *fontations_ffi::no_hinting_instance(),
                                                      scalerMetrics);
    fCanvas.clipPath(path.has_value() ? *path : SkPath(), fAntialias);
}

void ColorPainter::push_clip_rectangle(float x_min, float y_min, float x_max, float y_max) {
    fCanvas.save();
    SkRect clipRect = SkRect::MakeLTRB(x_min, -y_min, x_max, -y_max);
    fCanvas.clipRect(clipRect, fAntialias);
}

void ColorPainter::pop_clip() { fCanvas.restore(); }

void ColorPainter::configure_solid_paint(uint16_t palette_index, float alpha, SkPaint& paint) {
    paint.setAntiAlias(fAntialias);
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
    fontations_ffi::BridgeScalerMetrics scalerMetrics;
    if (auto path = fScalerContext.generatePathForGlyphId(glyph_id, fUpem,
                                                          *fontations_ffi::no_hinting_instance(),
                                                          scalerMetrics)) {
        SkPaint paint;
        configure_solid_paint(palette_index, alpha, paint);
        fCanvas.drawPath(*path, paint);
    }
}

void ColorPainter::configure_linear_paint(const fontations_ffi::FillLinearParams& linear_params,
                                          fontations_ffi::BridgeColorStops& bridge_stops,
                                          uint8_t extend_mode,
                                          SkPaint& paint,
                                          SkMatrix* paintTransform) {
    paint.setAntiAlias(fAntialias);

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
    fontations_ffi::BridgeScalerMetrics scalerMetrics;
    if (auto path = fScalerContext.generatePathForGlyphId(glyph_id, fUpem,
                                                          *fontations_ffi::no_hinting_instance(),
                                                          scalerMetrics)) {
        SkPaint paint;
        SkMatrix paintTransform = SkMatrixFromFontationsTransform(transform);
        configure_linear_paint(linear_params, bridge_stops, extend_mode, paint, &paintTransform);
        fCanvas.drawPath(*path, paint);
    }
}

void ColorPainter::configure_radial_paint(
        const fontations_ffi::FillRadialParams& fill_radial_params,
        fontations_ffi::BridgeColorStops& bridge_stops,
        uint8_t extend_mode,
        SkPaint& paint,
        SkMatrix* paintTransform) {
    paint.setAntiAlias(fAntialias);

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
    fontations_ffi::BridgeScalerMetrics scalerMetrics;
    if (auto path = fScalerContext.generatePathForGlyphId(glyph_id, fUpem,
                                                          *fontations_ffi::no_hinting_instance(),
                                                          scalerMetrics)) {
        SkPaint paint;
        SkMatrix paintTransform = SkMatrixFromFontationsTransform(transform);
        configure_radial_paint(fill_radial_params, bridge_stops, extend_mode, paint, &paintTransform);
        fCanvas.drawPath(*path, paint);
    }
}

void ColorPainter::configure_sweep_paint(const fontations_ffi::FillSweepParams& sweep_params,
                                         fontations_ffi::BridgeColorStops& bridge_stops,
                                         uint8_t extend_mode,
                                         SkPaint& paint,
                                         SkMatrix* paintTransform) {
    paint.setAntiAlias(fAntialias);

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
    fontations_ffi::BridgeScalerMetrics scalerMetrics;
    if (auto path = fScalerContext.generatePathForGlyphId(glyph_id, fUpem,
                                                          *fontations_ffi::no_hinting_instance(),
                                                          scalerMetrics)) {
        SkPaint paint;
        SkMatrix paintTransform = SkMatrixFromFontationsTransform(transform);
        configure_sweep_paint(sweep_params, bridge_stops, extend_mode, paint, &paintTransform);
        fCanvas.drawPath(*path, paint);
    }
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
        , fMatrixStack({initialTransfom})
        , fUpem(upem)
        , fBounds(SkRect::MakeEmpty()) {}

SkRect BoundsPainter::getBoundingBox() { return fBounds; }

// fontations_ffi::ColorPainter interface.
void BoundsPainter::push_transform(const fontations_ffi::Transform& transform_arg) {
    SkMatrix newTop(fMatrixStack.back());
    newTop.preConcat(SkMatrixFromFontationsTransform(transform_arg));
    fMatrixStack.push_back(newTop);
}
void BoundsPainter::pop_transform() {
    fMatrixStack.pop_back();
}

void BoundsPainter::push_clip_glyph(uint16_t glyph_id) {
    fontations_ffi::BridgeScalerMetrics scalerMetrics;
    if (auto path = fScalerContext.generatePathForGlyphId(glyph_id, fUpem,
                                                          *fontations_ffi::no_hinting_instance(),
                                                          scalerMetrics)) {
        if (auto newpath = path->tryMakeTransform(fMatrixStack.back())) {
            fBounds.join(newpath->getBounds());
        }
    }
}

void BoundsPainter::push_clip_rectangle(float x_min, float y_min, float x_max, float y_max) {
    const SkRect clipRect = SkRect::MakeLTRB(x_min, -y_min, x_max, -y_max);
    fBounds.join(fMatrixStack.back().mapRect(clipRect));
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
