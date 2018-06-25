/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDiscretePathEffect.h"
#include "SkFixed.h"
#include "SkPathMeasure.h"
#include "SkPointPriv.h"
#include "SkReadBuffer.h"
#include "SkStrokeRec.h"
#include "SkWriteBuffer.h"

sk_sp<SkPathEffect> SkDiscretePathEffect::Make(SkScalar segLength, SkScalar deviation,
                                               uint32_t seedAssist) {
    if (!SkScalarIsFinite(segLength) || !SkScalarIsFinite(deviation)) {
        return nullptr;
    }
    if (segLength <= SK_ScalarNearlyZero) {
        return nullptr;
    }
    return sk_sp<SkPathEffect>(new SkDiscretePathEffect(segLength, deviation, seedAssist));
}

static void Perterb(SkPoint* p, const SkVector& tangent, SkScalar scale) {
    SkVector normal = tangent;
    SkPointPriv::RotateCCW(&normal);
    normal.setLength(scale);
    *p += normal;
}

SkDiscretePathEffect::SkDiscretePathEffect(SkScalar segLength,
                                           SkScalar deviation,
                                           uint32_t seedAssist)
    : fSegLength(segLength), fPerterb(deviation), fSeedAssist(seedAssist)
{
}

/** \class LCGRandom

    Utility class that implements pseudo random 32bit numbers using a fast
    linear equation. Unlike rand(), this class holds its own seed (initially
    set to 0), so that multiple instances can be used with no side-effects.

    Copied from the original implementation of SkRandom. Only contains the
    methods used by SkDiscretePathEffect::filterPath, with methods that were
    not called directly moved to private.
*/

class LCGRandom {
public:
    LCGRandom(uint32_t seed) : fSeed(seed) {}

    /** Return the next pseudo random number expressed as a SkScalar
        in the range [-SK_Scalar1..SK_Scalar1).
    */
    SkScalar nextSScalar1() { return SkFixedToScalar(this->nextSFixed1()); }

private:
    /** Return the next pseudo random number as an unsigned 32bit value.
    */
    uint32_t nextU() { uint32_t r = fSeed * kMul + kAdd; fSeed = r; return r; }

    /** Return the next pseudo random number as a signed 32bit value.
     */
    int32_t nextS() { return (int32_t)this->nextU(); }

    /** Return the next pseudo random number expressed as a signed SkFixed
     in the range [-SK_Fixed1..SK_Fixed1).
     */
    SkFixed nextSFixed1() { return this->nextS() >> 15; }

    //  See "Numerical Recipes in C", 1992 page 284 for these constants
    enum {
        kMul = 1664525,
        kAdd = 1013904223
    };
    uint32_t fSeed;
};

bool SkDiscretePathEffect::filterPath(SkPath* dst, const SkPath& src,
                                      SkStrokeRec* rec, const SkRect*) const {
    bool doFill = rec->isFillStyle();

    SkPathMeasure   meas(src, doFill);

    /* Caller may supply their own seed assist, which by default is 0 */
    uint32_t seed = fSeedAssist ^ SkScalarRoundToInt(meas.getLength());

    LCGRandom   rand(seed ^ ((seed << 16) | (seed >> 16)));
    SkScalar    scale = fPerterb;
    SkPoint     p;
    SkVector    v;

    do {
        SkScalar    length = meas.getLength();

        if (fSegLength * (2 + doFill) > length) {
            meas.getSegment(0, length, dst, true);  // to short for us to mangle
        } else {
            int         n = SkScalarRoundToInt(length / fSegLength);
            constexpr int kMaxReasonableIterations = 100000;
            n = SkTMin(n, kMaxReasonableIterations);
            SkScalar    delta = length / n;
            SkScalar    distance = 0;

            if (meas.isClosed()) {
                n -= 1;
                distance += delta/2;
            }

            if (meas.getPosTan(distance, &p, &v)) {
                Perterb(&p, v, rand.nextSScalar1() * scale);
                dst->moveTo(p);
            }
            while (--n >= 0) {
                distance += delta;
                if (meas.getPosTan(distance, &p, &v)) {
                    Perterb(&p, v, rand.nextSScalar1() * scale);
                    dst->lineTo(p);
                }
            }
            if (meas.isClosed()) {
                dst->close();
            }
        }
    } while (meas.nextContour());
    return true;
}

sk_sp<SkFlattenable> SkDiscretePathEffect::CreateProc(SkReadBuffer& buffer) {
    SkScalar segLength = buffer.readScalar();
    SkScalar perterb = buffer.readScalar();
    uint32_t seed = buffer.readUInt();
    return Make(segLength, perterb, seed);
}

void SkDiscretePathEffect::flatten(SkWriteBuffer& buffer) const {
    buffer.writeScalar(fSegLength);
    buffer.writeScalar(fPerterb);
    buffer.writeUInt(fSeedAssist);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#define kNumWarpPts     9

class SkPathMapper {
public:
    virtual ~SkPathMapper() {}

    void mapPts(SkPoint dst[], const SkPoint src[], int count) {
        for (int i = 0; i < count; ++i) {
            dst[i] = this->onMap(src[i]);
        }
    }

    SkPoint map(SkPoint p) { return this->onMap(p); }

protected:
    virtual SkPoint onMap(SkPoint) = 0;
};

static void MapPath(SkPath* dst, SkPathMapper* mapper, const SkPath& src) {
    SkPath::Iter    iter(src, false);
    SkPoint         srcP[4], dstP[3];
    SkPath::Verb    verb;

    while ((verb = iter.next(srcP)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                dst->moveTo(mapper->map(srcP[0]));
                break;
            case SkPath::kLine_Verb:
                // turn lines into quads to look bendy
                srcP[0].fX = SkScalarAve(srcP[0].fX, srcP[1].fX);
                srcP[0].fY = SkScalarAve(srcP[0].fY, srcP[1].fY);
                mapper->mapPts(dstP, srcP, 2);
                dst->quadTo(dstP[0], dstP[1]);
                break;
            case SkPath::kQuad_Verb:
                mapper->mapPts(dstP, &srcP[1], 2);
                dst->quadTo(dstP[0], dstP[1]);
                break;
            case SkPath::kConic_Verb:
                mapper->mapPts(dstP, &srcP[1], 2);
                dst->conicTo(dstP[0], dstP[1], iter.conicWeight());
                break;
            case SkPath::kCubic_Verb:
                mapper->mapPts(dstP, &srcP[1], 3);
                dst->cubicTo(dstP[0], dstP[1], dstP[2]);
                break;
            case SkPath::kClose_Verb:
                dst->close();
                break;
            default:
                SkDEBUGFAIL("unknown verb");
                break;
        }
    }
}

//////////

#include "SkWarpPE.h"

void SkWarpPE::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeRect(fSrc);
    buffer.writePad32(fDst, sizeof(fDst));
}

sk_sp<SkFlattenable> SkWarpPE::CreateProc(SkReadBuffer& buffer) {
    SkRect src;
    SkPoint dst[kNumWarpPts];
    buffer.readRect(&src);
    buffer.readPad32(dst, sizeof(dst));
    return SkWarpPathEffect::Make(src, dst);
}

// 0,0  1,0  1,1  0,1
const uint8_t quadrantIndices[] = {
    0, 1, 4, 3,     1, 2, 5, 4,
    3, 4, 7, 6,     4, 5, 8, 7,
};

class warpmapper : public SkPathMapper {
    SkPoint  fDst[kNumWarpPts];
    SkRect   fSrc;
    SkScalar fInvW, fInvH;

public:
    warpmapper(const SkRect& src, const SkPoint dst[]) : fSrc(src) {
        memcpy(fDst, dst, kNumWarpPts * sizeof(SkPoint));

        fInvW = SkScalarInvert(src.width()) * 2;
        fInvH = SkScalarInvert(src.height()) * 2;
    }

    SkPoint onMap(SkPoint p) {
        SkScalar u = (p.fX - fSrc.fLeft) * fInvW;
        SkASSERT(u >= 0 && u <= 2);
        SkScalar v = (p.fY - fSrc.fTop)  * fInvH;
        SkASSERT(v >= 0 && v <= 2);

        int ix = std::min(SkScalarFloorToInt(u), 1);
        int iy = std::min(SkScalarFloorToInt(v), 1);
        SkASSERT(ix >= 0 && ix < 2);
        SkASSERT(iy >= 0 && iy < 2);
        u -= ix;
        v -= iy;
        SkASSERT(u >= 0 && u <= 1);
        SkASSERT(v >= 0 && v <= 1);

        const uint8_t* idx = &quadrantIndices[(ix + iy * 2) * 4];
        SkPoint dst[4] = {
            fDst[idx[0]], fDst[idx[1]], fDst[idx[2]], fDst[idx[3]],
        };

        SkScalar iu = 1 - u;
        SkScalar iv = 1 - v;
        return dst[0] * (iu * iv) + dst[1] * (u * iv) + dst[2] * (u * v) + dst[3] * (iu * v);
    }
};

bool SkWarpPE::filterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*) const {
    warpmapper mapper(fSrc, fDst);
    MapPath(dst, &mapper, src);
    return true;
}

sk_sp<SkPathEffect> SkWarpPathEffect::Make(const SkRect& src, const SkPoint dst[]) {
    if (src.isEmpty() && !src.isFinite()) {
        return nullptr;
    }
    return sk_sp<SkPathEffect>(new SkWarpPE(src, dst));
}

