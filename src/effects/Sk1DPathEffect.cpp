
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "Sk1DPathEffect.h"
#include "SkPathMeasure.h"

bool Sk1DPathEffect::filterPath(SkPath* dst, const SkPath& src, SkScalar* width) {
    SkPathMeasure   meas(src, false);
    do {
        SkScalar    length = meas.getLength();
        SkScalar    distance = this->begin(length);
        while (distance < length) {
            SkScalar delta = this->next(dst, distance, meas);
            if (delta <= 0) {
                break;
            }
            distance += delta;
        }
    } while (meas.nextContour());
    return true;
}

///////////////////////////////////////////////////////////////////////////////

SkPath1DPathEffect::SkPath1DPathEffect(const SkPath& path, SkScalar advance, 
    SkScalar phase, Style style) : fPath(path)
{
    if (advance <= 0 || path.isEmpty()) {
        SkDEBUGF(("SkPath1DPathEffect can't use advance <= 0\n"));
        fAdvance = 0;   // signals we can't draw anything
    } else {
        // cleanup their phase parameter, inverting it so that it becomes an
        // offset along the path (to match the interpretation in PostScript)
        if (phase < 0) {
            phase = -phase;
            if (phase > advance) {
                phase = SkScalarMod(phase, advance);
            }
        } else {
            if (phase > advance) {
                phase = SkScalarMod(phase, advance);
            }
            phase = advance - phase;
        }
        // now catch the edge case where phase == advance (within epsilon)
        if (phase >= advance) {
            phase = 0;
        }
        SkASSERT(phase >= 0);

        fAdvance = advance;
        fInitialOffset = phase;
        
        if ((unsigned)style >= kStyleCount) {
            SkDEBUGF(("SkPath1DPathEffect style enum out of range %d\n", style));
        }
        fStyle = style;
    }
}

bool SkPath1DPathEffect::filterPath(SkPath* dst, const SkPath& src,
                                    SkScalar* width) {
    if (fAdvance > 0) {
        *width = -1;
        return this->INHERITED::filterPath(dst, src, width);
    }
    return false;
}

static void morphpoints(SkPoint dst[], const SkPoint src[], int count,
                        SkPathMeasure& meas, SkScalar dist) {
    for (int i = 0; i < count; i++) {
        SkPoint pos;
        SkVector tangent;
        
        SkScalar sx = src[i].fX;
        SkScalar sy = src[i].fY;
        
        meas.getPosTan(dist + sx, &pos, &tangent);
        
        SkMatrix    matrix;
        SkPoint     pt;
        
        pt.set(sx, sy);
        matrix.setSinCos(tangent.fY, tangent.fX, 0, 0);
        matrix.preTranslate(-sx, 0);
        matrix.postTranslate(pos.fX, pos.fY);
        matrix.mapPoints(&dst[i], &pt, 1);
    }
}

/*  TODO

Need differentially more subdivisions when the follow-path is curvy. Not sure how to
determine that, but we need it. I guess a cheap answer is let the caller tell us,
but that seems like a cop-out. Another answer is to get Rob Johnson to figure it out.
*/
static void morphpath(SkPath* dst, const SkPath& src, SkPathMeasure& meas,
                      SkScalar dist) {
    SkPath::Iter    iter(src, false);
    SkPoint         srcP[4], dstP[3];
    SkPath::Verb    verb;

    while ((verb = iter.next(srcP)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                morphpoints(dstP, srcP, 1, meas, dist);
                dst->moveTo(dstP[0]);
                break;
            case SkPath::kLine_Verb:
                srcP[2] = srcP[1];
                srcP[1].set(SkScalarAve(srcP[0].fX, srcP[2].fX),
                            SkScalarAve(srcP[0].fY, srcP[2].fY));
                // fall through to quad
            case SkPath::kQuad_Verb:
                morphpoints(dstP, &srcP[1], 2, meas, dist);
                dst->quadTo(dstP[0], dstP[1]);
                break;
            case SkPath::kCubic_Verb:
                morphpoints(dstP, &srcP[1], 3, meas, dist);
                dst->cubicTo(dstP[0], dstP[1], dstP[2]);
                break;
            case SkPath::kClose_Verb:
                dst->close();
                break;
            default:
                SkASSERT(!"unknown verb");
                break;
        }
    }
}

SkPath1DPathEffect::SkPath1DPathEffect(SkFlattenableReadBuffer& buffer) {
    fAdvance = buffer.readScalar();
    if (fAdvance > 0) {
        fPath.unflatten(buffer);
        fInitialOffset = buffer.readScalar();
        fStyle = (Style) buffer.readU8();
    }
}

SkScalar SkPath1DPathEffect::begin(SkScalar contourLength) {
    return fInitialOffset;
}

void SkPath1DPathEffect::flatten(SkFlattenableWriteBuffer& buffer) {
    buffer.writeScalar(fAdvance);
    if (fAdvance > 0) {
        fPath.flatten(buffer);
        buffer.writeScalar(fInitialOffset);
        buffer.write8(fStyle);
    }
}

SkScalar SkPath1DPathEffect::next(SkPath* dst, SkScalar distance,
                                  SkPathMeasure& meas) {
    switch (fStyle) {
        case kTranslate_Style: {
            SkPoint pos;
            meas.getPosTan(distance, &pos, NULL);
            dst->addPath(fPath, pos.fX, pos.fY);
        } break;
        case kRotate_Style: {
            SkMatrix matrix;
            meas.getMatrix(distance, &matrix);
            dst->addPath(fPath, matrix);
        } break;
        case kMorph_Style:
            morphpath(dst, fPath, meas, distance);
            break;
        default:
            SkASSERT(!"unknown Style enum");
            break;
    }
    return fAdvance;
}

///////////////////////////////////////////////////////////////////////////////

static SkFlattenable::Registrar gReg("SkPath1DPathEffect",
                                     SkPath1DPathEffect::CreateProc);

