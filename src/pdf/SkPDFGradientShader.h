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

sk_sp<SkPDFObject> Make(SkPDFDocument* doc,
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
    uint32_t operator()(const Key& k) const {
        SkASSERT(0 != k.fHash);
        return k.fHash;
    }
};

using HashMap = SkTHashMap<Key, sk_sp<SkPDFObject>, KeyHash>;

inline bool operator==(const Key& u, const Key& v) {
    return u.fType                == v.fType
        && u.fInfo.fColorCount    == v.fInfo.fColorCount
        && u.fInfo.fPoint[0]      == v.fInfo.fPoint[0]
        && u.fInfo.fPoint[1]      == v.fInfo.fPoint[1]
        && u.fInfo.fRadius[0]     == v.fInfo.fRadius[0]
        && u.fInfo.fRadius[1]     == v.fInfo.fRadius[1]
        && u.fInfo.fTileMode      == v.fInfo.fTileMode
        && u.fInfo.fGradientFlags == v.fInfo.fGradientFlags
        && u.fCanvasTransform     == v.fCanvasTransform
        && u.fShaderTransform     == v.fShaderTransform
        && u.fBBox                == v.fBBox
        && SkArrayEqual(u.fColors.get(), v.fColors.get(), u.fInfo.fColorCount)
        && SkArrayEqual(u.fStops.get(), v.fStops.get(), u.fInfo.fColorCount);
}
inline bool operator!=(const Key& u, const Key& v) { return !(u == v); }

}  // namespace SkPDFGradientShader
#endif  // SkPDFGradientShader_DEFINED
