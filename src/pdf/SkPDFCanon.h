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
class SkPDFCanon : SkNoncopyable {
public:
    ~SkPDFCanon();

    sk_sp<SkPDFObject> findFunctionShader(const SkPDFShader::State&) const;
    void addFunctionShader(sk_sp<SkPDFObject>, SkPDFShader::State);

    sk_sp<SkPDFObject> findAlphaShader(const SkPDFShader::State&) const;
    void addAlphaShader(sk_sp<SkPDFObject>, SkPDFShader::State);

    sk_sp<SkPDFObject> findImageShader(const SkPDFShader::State&) const;
    void addImageShader(sk_sp<SkPDFObject>, SkPDFShader::State);

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

private:
    struct ShaderRec {
        SkPDFShader::State fShaderState;
        sk_sp<SkPDFObject> fShaderObject;
    };
    SkTArray<ShaderRec> fFunctionShaderRecords;
    SkTArray<ShaderRec> fAlphaShaderRecords;
    SkTArray<ShaderRec> fImageShaderRecords;
};
#endif  // SkPDFCanon_DEFINED
