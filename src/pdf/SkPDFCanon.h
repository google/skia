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
#include "SkTDArray.h"
#include "SkTHash.h"

class SkBitmap;
class SkPDFFont;
class SkPDFBitmap;
class SkPaint;

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

    // Returns exact match if there is one.  If not, it returns NULL.
    // If there is no exact match, but there is a related font, we
    // still return NULL, but also set *relatedFont.
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

    SkPDFBitmap* findBitmap(const SkBitmap&) const;
    void addBitmap(SkPDFBitmap*);

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
        explicit WrapGS(const SkPDFGraphicState* ptr = NULL) : fPtr(ptr) {}
        const SkPDFGraphicState* fPtr;
        bool operator==(const WrapGS& rhs) const {
            SkASSERT(fPtr);
            SkASSERT(rhs.fPtr);
            return *fPtr == *rhs.fPtr;
        }
        static uint32_t Hash(const WrapGS& w) {
            SkASSERT(w.fPtr);
            return w.fPtr->hash();
        }
    };
    SkTHashSet<WrapGS, WrapGS::Hash> fGraphicStateRecords;

    SkTDArray<SkPDFBitmap*> fBitmapRecords;
};
#endif  // SkPDFCanon_DEFINED
