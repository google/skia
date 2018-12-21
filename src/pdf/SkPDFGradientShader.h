/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPDFGradientShader_DEFINED
#define SkPDFGradientShader_DEFINED

#include "SkPDFTypes.h"
#include "SkPDFUtils.h"
#include "SkShader.h"

class SkMatrix;
class SkPDFDocument;
struct SkIRect;

namespace SkPDFGradientShader {

SkPDFIndirectReference Make(SkPDFDocument* doc,
                            SkShader* shader,
                            const SkMatrix& matrix,
                            const SkIRect& surfaceBBox);

struct Key {
    SkShader::GradientType fType;
    SkShader::GradientInfo fInfo;
    std::unique_ptr<SkColor[]> fColors;
    std::unique_ptr<SkScalar[]> fStops;
    SkMatrix fCanvasTransform;
    SkMatrix fShaderTransform;
    SkIRect fBBox;
    uint32_t fHash;
};

struct KeyHash {
    uint32_t operator()(const Key& k) const { return k.fHash; }
};

inline bool operator==(const SkShader::GradientInfo& u, const SkShader::GradientInfo& v) {
    return u.fColorCount    == v.fColorCount
        && u.fPoint[0]      == v.fPoint[0]
        && u.fPoint[1]      == v.fPoint[1]
        && u.fRadius[0]     == v.fRadius[0]
        && u.fRadius[1]     == v.fRadius[1]
        && u.fTileMode      == v.fTileMode
        && u.fGradientFlags == v.fGradientFlags
        && SkPackedArrayEqual(u.fColors, v.fColors, u.fColorCount)
        && SkPackedArrayEqual(u.fColorOffsets, v.fColorOffsets, u.fColorCount);
}

inline bool operator==(const Key& u, const Key& v) {
    SkASSERT(u.fInfo.fColors       == u.fColors.get());
    SkASSERT(u.fInfo.fColorOffsets == u.fStops.get());
    SkASSERT(v.fInfo.fColors       == v.fColors.get());
    SkASSERT(v.fInfo.fColorOffsets == v.fStops.get());
    return u.fType            == v.fType
        && u.fInfo            == v.fInfo
        && u.fCanvasTransform == v.fCanvasTransform
        && u.fShaderTransform == v.fShaderTransform
        && u.fBBox            == v.fBBox;
}
inline bool operator!=(const Key& u, const Key& v) { return !(u == v); }

}  // namespace SkPDFGradientShader
#endif  // SkPDFGradientShader_DEFINED
