/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPDFCanon_DEFINED
#define SkPDFCanon_DEFINED

#include "SkBitmap.h"
#include "SkPDFGraphicState.h"
#include "SkPDFShader.h"
#include "SkPixelSerializer.h"
#include "SkTDArray.h"
#include "SkTHash.h"

class SkPDFFont;
class SkPaint;
class SkImage;

class SkBitmapKey {
public:
    SkBitmapKey() : fSubset(SkIRect::MakeEmpty()), fGenID(0) {}
    explicit SkBitmapKey(const SkBitmap& bm)
        : fSubset(bm.getSubset()), fGenID(bm.getGenerationID()) {}
    bool operator==(const SkBitmapKey& rhs) const {
        return fGenID == rhs.fGenID && fSubset == rhs.fSubset;
    }

private:
    SkIRect fSubset;
    uint32_t fGenID;
};

/**
 *  The SkPDFCanon canonicalizes objects across PDF pages(SkPDFDevices).
 *
 *  The PDF backend works correctly if:
 *  -  There is no more than one SkPDFCanon for each thread.
 *  -  Every SkPDFDevice is given a pointer to a SkPDFCanon on creation.
 *  -  All SkPDFDevices in a document share the same SkPDFCanon.
 *  The SkDocument_PDF class makes this happen by owning a single
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
    ~SkPDFCanon() { this->reset(); }

    // reset to original setting, unrefs all objects.
    void reset();

    // Returns exact match if there is one.  If not, it returns nullptr.
    // If there is no exact match, but there is a related font, we
    // still return nullptr, but also set *relatedFont.
    SkPDFFont* findFont(uint32_t fontID,
                        uint16_t glyphID,
                        SkPDFFont** relatedFont) const;
    void addFont(SkPDFFont* font, uint32_t fontID, uint16_t fGlyphID);

    SkPDFFunctionShader* findFunctionShader(const SkPDFShader::State&) const;
    void addFunctionShader(SkPDFFunctionShader*);

    SkPDFAlphaFunctionShader* findAlphaShader(const SkPDFShader::State&) const;
    void addAlphaShader(SkPDFAlphaFunctionShader*);

    SkPDFImageShader* findImageShader(const SkPDFShader::State&) const;
    void addImageShader(SkPDFImageShader*);

    const SkPDFGraphicState* findGraphicState(const SkPDFGraphicState&) const;
    void addGraphicState(const SkPDFGraphicState*);

    SkPDFObject* findPDFBitmap(const SkImage* image) const;
    void addPDFBitmap(uint32_t imageUniqueID, SkPDFObject*);
    const SkImage* bitmapToImage(const SkBitmap&);

    SkTHashMap<uint32_t, bool> fCanEmbedTypeface;

    SkAutoTUnref<SkPixelSerializer> fPixelSerializer;

private:
    struct FontRec {
        SkPDFFont* fFont;
        uint32_t fFontID;
        uint16_t fGlyphID;
    };
    SkTDArray<FontRec> fFontRecords;

    SkTDArray<SkPDFFunctionShader*> fFunctionShaderRecords;

    SkTDArray<SkPDFAlphaFunctionShader*> fAlphaShaderRecords;

    SkTDArray<SkPDFImageShader*> fImageShaderRecords;

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

    SkTHashMap<SkBitmapKey, const SkImage*> fBitmapToImageMap;
    SkTHashMap<uint32_t /*ImageUniqueID*/, SkPDFObject*> fPDFBitmapMap;
};
#endif  // SkPDFCanon_DEFINED
