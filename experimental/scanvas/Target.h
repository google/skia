// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef Target_DEFINED
#define Target_DEFINED

#include "include/core/SkImageFilter.h"
#include "include/core/SkRSXform.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "src/core/SkClipStack.h"
#include "src/core/SkGlyphRun.h"
#include "src/core/SkLatticeIter.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkUtils.h"
#include "src/utils/SkPatchUtils.h"

#include <vector>

// 2x3 matrix stored column-major.
// Same order as specified in PDF standard.
// See PDF32000.book section 8.3.4 "Transformation Matrices"
// The default value is the identity matrix.
SK_BEGIN_REQUIRE_DENSE
struct SkAffineMatrix {
    SkScalar fMat[6] = {1, 0, 0, 1, 0, 0};
    bool setFromMatrix(const SkMatrix& m) { return m.asAffine(fMat); }
    SkMatrix toMatrix() const { SkMatrix m; m.setAffine(fMat); return m; }
};
SK_END_REQUIRE_DENSE

static inline bool operator==(const SkAffineMatrix& u, const SkAffineMatrix& v) {
    return 0 == memcmp(&u, &v, sizeof(u));
}
static inline bool operator!=(const SkAffineMatrix& u, const SkAffineMatrix& v) { return !(u == v); }

struct RGBColorF {
    float fR = 0;
    float fG = 0;
    float fB = 0;
};

struct GraphicState {
    SkAffineMatrix fMatrix;
    uint32_t fClipStackGenID = SkClipStack::kWideOpenGenID;
    float fAlpha;
    SkBlendMode fBlendMode = SkBlendMode::kSrcOver;
    RGBColorF fColor;
    SkScalar fTextScaleX = 1;  // Zero means we don't care what the value is.
    int fFontIndex = -1;
    int fShaderIndex = -1;
    SkScalar fStrokeWidth = 1;
    SkScalar fStrokeMiter = 10;
    SkPaint::Cap fStrokeCap = SkPaint::kButt_Cap;
    SkPaint::Join fStrokeJoin = SkPaint::kMiter_Join;
};

// Used by restoreLayer()  bottom device doesn't use.
struct LayerRecord {
    SkRect fBounds = {0, 0 ,0, 0};
    SkPaint fPaint;
    sk_sp<SkImageFilter> fBackdrop;
    sk_sp<SkImage> fClipMask;
    SkMatrix fClipMatrix = SkMatrix::I();
};

struct Layer {
    SkDynamicMemoryWStream fContent;
    GraphicState fStateStack[3];
    int fStateStackDepth = 0;
    LayerRecord fLayerRecord;
};


struct Target {
    SkISize fSize;
    std::vector<Layer> fLayers;
    int fNodeId = -1;

    Target(SkISize s) : fSize(s) { fLayers.emplace_back(); }
    ~Target() = default;

    SkISize size() const { return fSize; }
    void flush() {}
    void discard() {}

    void saveLayer(const SkClipStack& cs, const SkMatrix& ctm, const SkCanvas::SaveLayerRec&);
    void restoreLayer();

    void drawGlyphs(const SkClipStack& cs, const SkMatrix& ctm,
                    const SkGlyphRunList& glyphRuns);
    void drawGlyphsRSXform(const SkClipStack& cs, const SkMatrix& ctm,
                           const SkGlyphRunList& glyphRuns, const SkRSXform* xform);
    void drawPath(const SkClipStack& cs, const SkMatrix& ctm,
                  const SkPath& path, const SkPaint& paint);
    void drawPoints(const SkClipStack& cs, const SkMatrix& ctm,
                    SkCanvas::PointMode mode, size_t count, const SkPoint pts[],
                    const SkPaint& paint);
    void drawVertices(const SkClipStack& cs, const SkMatrix& ctm,
                      const SkVertices* vertices, const SkVertices::Bone bones[],
                      int boneCount, SkBlendMode mode, const SkPaint& paint);
    void drawImageRect(const SkClipStack& cs, const SkMatrix& ctm,
                       const SkImage* image, const SkRect* src, const SkRect& dst,
                       const SkPaint* paint, SkCanvas::SrcRectConstraint constraint);
    void drawAnnotation(const SkClipStack& cs, const SkMatrix& ctm,
                        const SkRect& rect, const char key[], SkData* value);
    void drawShadowRec(const SkClipStack& cs, const SkMatrix& ctm,
                       const SkPath&, const SkDrawShadowRec&);

    Target(Target&&) = delete;
    Target(const Target&) = delete;
    Target& operator=(Target&&) = delete;
    Target& operator=(const Target&) = delete;
};
#endif  // Target_DEFINED
