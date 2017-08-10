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
 *
 *  The PDF backend works correctly if:
 *  -  There is no more than one SkPDFCanon for each thread.
 *  -  Every SkPDFDevice is given a pointer to a SkPDFCanon on creation.
 *  -  All SkPDFDevices in a document share the same SkPDFCanon.
 *  The SkPDFDocument class makes this happen by owning a single
 *  SkPDFCanon.
 *
 *  The addFoo() methods will ref the Foo; the canon's destructor will
 *  call foo->unref() on all of these objects.
 *
 *  The findFoo() methods do not change the ref count of the Foo
 *  objects.
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

    const SkPDFGraphicState* findGraphicState(const SkPDFGraphicState&) const;
    void addGraphicState(const SkPDFGraphicState*);

    SkTHashMap<SkBitmapKey, sk_sp<SkPDFObject>> fPDFBitmapMap;

    SkTHashMap<uint32_t, std::unique_ptr<SkAdvancedTypefaceMetrics>> fTypefaceMetrics;
    SkTHashMap<uint32_t, sk_sp<SkPDFDict>> fFontDescriptors;
    SkTHashMap<uint64_t, sk_sp<SkPDFFont>> fFontMap;

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

    struct WrapGS {
        explicit WrapGS(const SkPDFGraphicState* ptr = nullptr) : fPtr(ptr) {}
        const SkPDFGraphicState* fPtr;
        bool operator==(const WrapGS& rhs) const {
            SkASSERT(fPtr);
            SkASSERT(rhs.fPtr);
            return *fPtr == *rhs.fPtr;
        }
        struct Hash {
            uint32_t operator()(const WrapGS& w) const {
                SkASSERT(w.fPtr);
                return w.fPtr->hash();
            }
        };
    };
    SkTHashSet<WrapGS, WrapGS::Hash> fGraphicStateRecords;
};
#endif  // SkPDFCanon_DEFINED
