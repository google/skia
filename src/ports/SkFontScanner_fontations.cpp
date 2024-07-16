/*
 * Copyright 2024 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/core/SkFontScanner.h"
#include "src/sfnt/SkOTUtils.h"

#include "src/ports/SkFontScanner_fontations.h"
#include "src/ports/SkTypeface_fontations_priv.h"
#include "src/ports/fontations/src/skpath_bridge.h"

using namespace skia_private;

namespace {
rust::Box<::fontations_ffi::BridgeFontRef> make_bridge_font_ref(sk_sp<SkData> fontData,
                                                                uint32_t index) {
    rust::Slice<const uint8_t> slice{fontData->bytes(), fontData->size()};
    return fontations_ffi::make_font_ref(slice, index);
}
}

SkFontScanner_Fontations::SkFontScanner_Fontations() {
}

SkFontScanner_Fontations::~SkFontScanner_Fontations() {
}

bool SkFontScanner_Fontations::scanFile(SkStreamAsset* stream, int* numFaces) const {
    sk_sp<SkData> fontData = SkData::MakeFromStream(stream, stream->getLength());
    stream->rewind();
    rust::Slice<const uint8_t> slice{fontData->bytes(), fontData->size()};
    ::std::uint32_t num_fonts;
    if (!fontations_ffi::font_or_collection(slice, num_fonts)) {
        return false;
    }
    *numFaces = num_fonts == 0 ? 1 : num_fonts;
    return true;
}

bool SkFontScanner_Fontations::scanFace(SkStreamAsset* stream,
                                        int faceIndex,
                                        int* numInstances) const {
    rust::Box<fontations_ffi::BridgeFontRef> fontRef =
            make_bridge_font_ref(SkData::MakeFromStream(stream, stream->getLength()), faceIndex);
    stream->rewind();
    if (!fontations_ffi::font_ref_is_valid(*fontRef)) {
        return false;
    }

    if (numInstances) {
        *numInstances = fontations_ffi::num_named_instances(*fontRef);
    }
    return true;
}

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

bool SkFontScanner_Fontations::scanInstance(SkStreamAsset* stream,
                                            int faceIndex,
                                            int instanceIndex,
                                            SkString* name,
                                            SkFontStyle* style,
                                            bool* isFixedPitch,
                                            AxisDefinitions* axes) const {

    rust::Box<fontations_ffi::BridgeFontRef> bridgeFontFaceRef =
            make_bridge_font_ref(SkData::MakeFromStream(stream, stream->getLength()), faceIndex);
    stream->rewind();
    if (!fontations_ffi::font_ref_is_valid(*bridgeFontFaceRef)) {
        return false;
    }

    if (name != nullptr) {
        rust::String readFamilyName = fontations_ffi::family_name(*bridgeFontFaceRef);
        *name = SkString(readFamilyName.data(), readFamilyName.size());
    }

    if (isFixedPitch != nullptr) {
        *isFixedPitch = false;  // TODO
    }

    if (style != nullptr) {
        auto num = SkToInt(fontations_ffi::num_named_instances(*bridgeFontFaceRef));
        if (instanceIndex > num) {
            return false;
        } else if (instanceIndex == 0) {
            // This is the default instance
            rust::Slice<const fontations_ffi::SkiaDesignCoordinate> coordinates;
            rust::Box<fontations_ffi::BridgeNormalizedCoords> normalizedCoords =
                    resolve_into_normalized_coords(*bridgeFontFaceRef, coordinates);
            fontations_ffi::BridgeFontStyle fontStyle;
            if (fontations_ffi::get_font_style(*bridgeFontFaceRef, *normalizedCoords, fontStyle)) {
                *style = SkFontStyle(fontStyle.weight, fontStyle.width, (SkFontStyle::Slant)fontStyle.slant);
            } else {
                *style = SkFontStyle::Normal();
            }
        } else {
            std::unique_ptr<SkFontArguments::VariationPosition::Coordinate[]> extractedCoords =
                    nullptr;
            size_t numNamedInstanceCoords =
                    fontations_ffi::coordinates_for_shifted_named_instance_index(
                            *bridgeFontFaceRef,
                            instanceIndex << 16,
                            rust::cxxbridge1::Slice<fontations_ffi::SkiaDesignCoordinate>());
            extractedCoords.reset(new SkFontArguments::VariationPosition::Coordinate[numNamedInstanceCoords]);

            rust::cxxbridge1::Slice<fontations_ffi::SkiaDesignCoordinate> targetSlice(
                    reinterpret_cast<fontations_ffi::SkiaDesignCoordinate*>(extractedCoords.get()),
                    numNamedInstanceCoords);
            size_t retrievedNamedInstanceCoords =
                    fontations_ffi::coordinates_for_shifted_named_instance_index(
                            *bridgeFontFaceRef, faceIndex + (instanceIndex << 16), targetSlice);
            if (numNamedInstanceCoords != retrievedNamedInstanceCoords) {
                return false;
            }

            SkFontArguments::VariationPosition variationPosition;
            variationPosition.coordinateCount = numNamedInstanceCoords;
            variationPosition.coordinates = extractedCoords.get();

            rust::Box<fontations_ffi::BridgeNormalizedCoords> normalizedCoords =
                    make_normalized_coords(*bridgeFontFaceRef, variationPosition);
            fontations_ffi::BridgeFontStyle fontStyle;
            if (fontations_ffi::get_font_style(*bridgeFontFaceRef, *normalizedCoords, fontStyle)) {
                *style = SkFontStyle(fontStyle.weight,
                                     fontStyle.width,
                                     static_cast<SkFontStyle::Slant>(fontStyle.slant));
            }
        }
    }

    if (axes != nullptr) {
        rust::Box<fontations_ffi::BridgeFontRef> bridgeFontNamedInstanceRef =
                make_bridge_font_ref(SkData::MakeFromStream(stream, stream->getLength()), faceIndex + (instanceIndex << 16));
        stream->rewind();
        auto size = SkToInt(fontations_ffi::num_axes(*bridgeFontNamedInstanceRef));
        axes->reset(size);
        auto variationAxes = std::make_unique<SkFontParameters::Variation::Axis[]>(size);
        sk_fontations::AxisWrapper axisWrapper(variationAxes.get(), size);
        auto size1 = fontations_ffi::populate_axes(*bridgeFontNamedInstanceRef, axisWrapper);
        SkASSERT(size == size1);
        for (auto i = 0; i < size; ++i) {
            const auto var = variationAxes[i];
            (*axes)[i].fTag = var.tag;
            (*axes)[i].fMinimum = var.min;
            (*axes)[i].fDefault = var.def;
            (*axes)[i].fMaximum = var.max;
        }
    }

    return true;
}

