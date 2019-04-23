/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPaint.h"
#include "src/core/SkXfermodeInterpretation.h"

static bool just_solid_color(const SkPaint& p) {
    return SK_AlphaOPAQUE == p.getAlpha() && !p.getColorFilter() && !p.getShader();
}

SkXfermodeInterpretation SkInterpretXfermode(const SkPaint& paint, bool dstIsOpaque) {
    switch (paint.getBlendMode()) {
        case SkBlendMode::kSrcOver:
            return kSrcOver_SkXfermodeInterpretation;
        case SkBlendMode::kSrc:
            if (just_solid_color(paint)) {
                return kSrcOver_SkXfermodeInterpretation;
            }
            return kNormal_SkXfermodeInterpretation;
        case SkBlendMode::kDst:
            return kSkipDrawing_SkXfermodeInterpretation;
        case SkBlendMode::kDstOver:
            if (dstIsOpaque) {
                return kSkipDrawing_SkXfermodeInterpretation;
            }
            return kNormal_SkXfermodeInterpretation;
        case SkBlendMode::kSrcIn:
            if (dstIsOpaque && just_solid_color(paint)) {
                return kSrcOver_SkXfermodeInterpretation;
            }
            return kNormal_SkXfermodeInterpretation;
        case SkBlendMode::kDstIn:
            if (just_solid_color(paint)) {
                return kSkipDrawing_SkXfermodeInterpretation;
            }
            return kNormal_SkXfermodeInterpretation;
        default:
            return kNormal_SkXfermodeInterpretation;
    }
}
