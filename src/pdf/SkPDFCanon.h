/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPDFCanon_DEFINED
#define SkPDFCanon_DEFINED

#include "SkPDFGraphicState.h"
#include "SkPDFShader.h"
#include "SkPDFGradientShader.h"
#include "SkPixelSerializer.h"
#include "SkTDArray.h"
#include "SkTHash.h"
#include "SkBitmapKey.h"

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
};
#endif  // SkPDFCanon_DEFINED
