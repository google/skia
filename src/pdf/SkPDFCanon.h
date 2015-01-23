/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPDFCanon_DEFINED
#define SkPDFCanon_DEFINED

#include "SkPDFShader.h"
#include "SkThread.h"
#include "SkTDArray.h"

struct SkIRect;
class SkMatrix;
class SkPDFFont;
class SkPDFGraphicState;
class SkPaint;

// This class's fields and methods will eventually become part of
// SkPDFDocument/SkDocument_PDF.  For now, it exists as a singleton to
// preflight that transition.  This replaces three global arrays in
// SkPDFFont, SkPDFShader, and SkPDFGraphicsContext.
//
// IF YOU ARE LOOKING AT THIS API PLEASE DO NOT WRITE THE CHANGE
// YOU ARE ABOUT TO WRITE WITHOUT TALKING TO HALCANARY@.
//
// Note that this class does not create, delete, reference or
// dereference the SkPDFObject objects that it indexes.  It is up to
// the caller to manage the lifetime of these objects.
class SkPDFCanon : SkNoncopyable {
public:
    SkPDFCanon();
    ~SkPDFCanon();

    static SkPDFCanon& GetCanon();

    // This mutexes will be removed once this class is subsumed into
    // SkPDFDocument.
    static SkBaseMutex& GetFontMutex();
    static SkBaseMutex& GetShaderMutex();
    static SkBaseMutex& GetPaintMutex();

    // Returns exact match if there is one.  If not, it returns NULL.
    // If there is no exact match, but there is a related font, we
    // still return NULL, but also set *relatedFont.
    SkPDFFont* findFont(uint32_t fontID,
                        uint16_t glyphID,
                        SkPDFFont** relatedFont) const;
    void addFont(SkPDFFont* font, uint32_t fontID, uint16_t fGlyphID);
    void removeFont(SkPDFFont*);

    SkPDFFunctionShader* findFunctionShader(const SkPDFShader::State&) const;
    void addFunctionShader(SkPDFFunctionShader*);
    void removeFunctionShader(SkPDFFunctionShader*);

    SkPDFAlphaFunctionShader* findAlphaShader(const SkPDFShader::State&) const;
    void addAlphaShader(SkPDFAlphaFunctionShader*);
    void removeAlphaShader(SkPDFAlphaFunctionShader*);

    SkPDFImageShader* findImageShader(const SkPDFShader::State&) const;
    void addImageShader(SkPDFImageShader*);
    void removeImageShader(SkPDFImageShader*);

    SkPDFGraphicState* findGraphicState(const SkPaint&) const;
    void addGraphicState(SkPDFGraphicState*);
    void removeGraphicState(SkPDFGraphicState*);

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

    SkTDArray<SkPDFGraphicState*> fGraphicStateRecords;
};
#endif  // SkPDFCanon_DEFINED
