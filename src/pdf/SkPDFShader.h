/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPDFShader_DEFINED
#define SkPDFShader_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkMacros.h"
#include "src/core/SkChecksum.h"
#include "src/pdf/SkBitmapKey.h"
#include "src/pdf/SkPDFTypes.h"

class SkPDFDocument;
class SkShader;
enum class SkTileMode;

/** Make a PDF shader for the passed SkShader. If the SkShader is invalid in
 *  some way, returns nullptr.
 *
 *  In PDF parlance, this is a pattern, used in place of a color when the
 *  pattern color space is selected.
 *
 *  May cache the shader in the document for later re-use.  If this function is
 *  called again with an equivalent shader,  a new reference to the cached pdf
 *  shader may be returned.
 *
 *  @param doc         The parent document, must be non-null.
 *  @param shader      The SkShader to emulate.
 *  @param ctm         The current transform matrix. (PDF shaders are absolutely
 *                     positioned, relative to where the page is drawn.)
 *  @param surfaceBBox The bounding box of the drawing surface (with matrix
 *                     already applied).
 *  @param paintColor  Color+Alpha of the paint.  Color is usually ignored,
 *                     unless it is a alpha shader.
 */
SkPDFIndirectReference SkPDFMakeShader(SkPDFDocument* doc,
                                       SkShader* shader,
                                       const SkMatrix& ctm,
                                       const SkIRect& surfaceBBox,
                                       SkColor4f paintColor);

SK_BEGIN_REQUIRE_DENSE
struct SkPDFImageShaderKey {
    SkMatrix fTransform;
    SkIRect fBBox;
    SkBitmapKey fBitmapKey;
    SkTileMode fImageTileModes[2];
    SkColor4f fPaintColor;

    using Hash = SkForceDirectHash<SkPDFImageShaderKey>;
};
SK_END_REQUIRE_DENSE

inline bool operator==(const SkPDFImageShaderKey& a, const SkPDFImageShaderKey& b) {
    SkASSERT(a.fBitmapKey.fID != 0);
    SkASSERT(b.fBitmapKey.fID != 0);
    return a.fTransform         == b.fTransform
        && a.fBBox              == b.fBBox
        && a.fBitmapKey         == b.fBitmapKey
        && a.fImageTileModes[0] == b.fImageTileModes[0]
        && a.fImageTileModes[1] == b.fImageTileModes[1]
        && a.fPaintColor        == b.fPaintColor;
}
#endif
