/*
 * Copyright 2024 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
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

bool SkFontScanner_Fontations::recognizedFont(SkStreamAsset* stream, int* numFaces) const {
    sk_sp<SkData> fontData = SkData::MakeFromStream(stream, stream->getLength());
    rust::Slice<const uint8_t> slice{fontData->bytes(), fontData->size()};
    ::std::uint32_t num_fonts;
    if (!fontations_ffi::font_or_collection(slice, num_fonts)) {
        return false;
    }
    *numFaces = num_fonts == 0 ? 1 : num_fonts;
    return true;
}

// Assuming default values for everything we don't have right now
bool SkFontScanner_Fontations::scanFont(
        SkStreamAsset* stream, int ttcIndex,
        SkString* name, SkFontStyle* style, bool* isFixedPitch, AxisDefinitions* axes) const
{
    rust::Box<fontations_ffi::BridgeFontRef> fontRef =
            make_bridge_font_ref(SkData::MakeFromStream(stream, stream->getLength()), ttcIndex);
    if (!fontations_ffi::font_ref_is_valid(*fontRef)) {
        return false;
    }

    if (name != nullptr) {
        rust::String readFamilyName = fontations_ffi::family_name(*fontRef);
        *name = SkString(readFamilyName.data(), readFamilyName.size());
    }

    if (style != nullptr) {
        fontations_ffi::BridgeFontStyle fontStyle;
        if (fontations_ffi::get_font_style(*fontRef, fontStyle)) {
            *style = SkFontStyle(fontStyle.weight, fontStyle.width,  (SkFontStyle::Slant)fontStyle.slant);
        } else {
            *style = SkFontStyle::Normal();
        }
    }

    return true;
}

