/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPDFCanon_DEFINED
#define SkPDFCanon_DEFINED

#include "SkBitmapKey.h"
#include "SkPDFGradientShader.h"
#include "SkPDFGraphicState.h"
#include "SkPDFShader.h"
#include "SkPixelSerializer.h"
#include "SkTDArray.h"
#include "SkTHash.h"
#include "SkTypeface.h"

class SkPDFFont;
struct SkAdvancedTypefaceMetrics;

/**
 *  The SkPDFCanon canonicalizes objects across PDF pages
 *  (SkPDFDevices) and across draw calls.
 */
class SkPDFCanon {
public:
    ~SkPDFCanon();
    SkPDFCanon();
    SkPDFCanon(const SkPDFCanon&) = delete;
    SkPDFCanon& operator=(const SkPDFCanon&) = delete;

    SkTHashMap<SkPDFImageShaderKey, sk_sp<SkPDFObject>> fImageShaderMap;

    SkPDFGradientShader::HashMap fGradientPatternMap;

    SkTHashMap<SkBitmapKey, sk_sp<SkPDFObject>> fPDFBitmapMap;

    SkTHashMap<uint32_t, std::unique_ptr<SkAdvancedTypefaceMetrics>> fTypefaceMetrics;
    SkTHashMap<uint32_t, sk_sp<SkPDFDict>> fFontDescriptors;
    SkTHashMap<uint64_t, sk_sp<SkPDFFont>> fFontMap;

    SkTHashMap<SkPDFStrokeGraphicState, sk_sp<SkPDFDict>> fStrokeGSMap;
    SkTHashMap<SkPDFFillGraphicState, sk_sp<SkPDFDict>> fFillGSMap;

    sk_sp<SkPixelSerializer> fPixelSerializer;
    sk_sp<SkPDFStream> fInvertFunction;
    sk_sp<SkPDFDict> fNoSmaskGraphicState;
    sk_sp<SkPDFArray> fRangeObject;

    SK_BEGIN_REQUIRE_DENSE
    struct BitmapGlyphKey {
        SkFontID fFontID;      // uint32_t
        SkScalar fTextSize;    // float32
        SkScalar fTextScaleX;  // float32
        SkScalar fTextSkewX;   // float32
        SkGlyphID fGlyphID;    // uint16_t
        uint16_t fPadding;
    };
    SK_END_REQUIRE_DENSE
    struct BitmapGlyph {
        sk_sp<SkImage> fImage;
        SkIPoint fOffset;
    };
    SkTHashMap<BitmapGlyphKey, BitmapGlyph> fBitmapGlyphImages;
};

inline bool operator==(const SkPDFCanon::BitmapGlyphKey& u, const SkPDFCanon::BitmapGlyphKey& v) {
    return memcmp(&u, &u, sizeof(SkPDFCanon::BitmapGlyphKey)) == 0;
}
#endif  // SkPDFCanon_DEFINED
