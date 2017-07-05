/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPDFShader_DEFINED
#define SkPDFShader_DEFINED

#include "SkBitmapKey.h"
#include "SkPDFTypes.h"
#include "SkShader.h"

class SkPDFCanon;
class SkPDFDocument;
class SkMatrix;
struct SkIRect;

/** \class SkPDFShader

    In PDF parlance, this is a pattern, used in place of a color when the
    pattern color space is selected.
*/

class SkPDFShader {
public:
    /** Get the PDF shader for the passed SkShader. If the SkShader is
     *  invalid in some way, returns nullptr. The reference count of
     *  the object is incremented and it is the caller's responsibility to
     *  unreference it when done.  This is needed to accommodate the weak
     *  reference pattern used when the returned object is new and has no
     *  other references.
     *  @param shader      The SkShader to emulate.
     *  @param matrix      The current transform. (PDF shaders are absolutely
     *                     positioned, relative to where the page is drawn.)
     *  @param surfceBBox  The bounding box of the drawing surface (with matrix
     *                     already applied).
     */
    static sk_sp<SkPDFObject> GetPDFShader(SkPDFDocument* doc,
                                           SkShader* shader,
                                           const SkMatrix& matrix,
                                           const SkIRect& surfaceBBox);

    static sk_sp<SkPDFArray> MakeRangeObject();

    SK_BEGIN_REQUIRE_DENSE
    struct State {
        SkMatrix fCanvasTransform;
        SkMatrix fShaderTransform;
        SkIRect fBBox;
        SkBitmapKey fBitmapKey;
        SkShader::TileMode fImageTileModes[2];
    };
    SK_END_REQUIRE_DENSE
};

inline bool operator==(const SkPDFShader::State& a, const SkPDFShader::State& b) {
    SkASSERT(a.fBitmapKey.fID != 0);
    SkASSERT(b.fBitmapKey.fID != 0);
    return a.fCanvasTransform   == b.fCanvasTransform
        && a.fShaderTransform   == b.fShaderTransform
        && a.fBBox              == b.fBBox
        && a.fBitmapKey         == b.fBitmapKey
        && a.fImageTileModes[0] == b.fImageTileModes[0]
        && a.fImageTileModes[1] == b.fImageTileModes[1];
}

#endif
