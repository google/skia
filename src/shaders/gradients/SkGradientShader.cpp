/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algorithm>
#include "Sk4fLinearGradient.h"
#include "SkColorSpace_XYZ.h"
#include "SkGradientShaderPriv.h"
#include "SkHalf.h"
#include "SkLinearGradient.h"
#include "SkMallocPixelRef.h"
#include "SkRadialGradient.h"
#include "SkSweepGradient.h"
#include "SkTwoPointConicalGradient.h"
#include "../../jumper/SkJumper.h"


enum GradientSerializationFlags {
    // Bits 29:31 used for various boolean flags
    kHasPosition_GSF    = 0x80000000,
    kHasLocalMatrix_GSF = 0x40000000,
    kHasColorSpace_GSF  = 0x20000000,

    // Bits 12:28 unused

    // Bits 8:11 for fTileMode
    kTileModeShift_GSF  = 8,
    kTileModeMask_GSF   = 0xF,

    // Bits 0:7 for fGradFlags (note that kForce4fContext_PrivateFlag is 0x80)
    kGradFlagsShift_GSF = 0,
    kGradFlagsMask_GSF  = 0xFF,
};

void SkGradientShaderBase::Descriptor::flatten(SkWriteBuffer& buffer) const {
    uint32_t flags = 0;
    if (fPos) {
        flags |= kHasPosition_GSF;
    }
    if (fLocalMatrix) {
        flags |= kHasLocalMatrix_GSF;
    }
    sk_sp<SkData> colorSpaceData = fColorSpace ? fColorSpace->serialize() : nullptr;
    if (colorSpaceData) {
        flags |= kHasColorSpace_GSF;
    }
    SkASSERT(static_cast<uint32_t>(fTileMode) <= kTileModeMask_GSF);
    flags |= (fTileMode << kTileModeShift_GSF);
    SkASSERT(fGradFlags <= kGradFlagsMask_GSF);
    flags |= (fGradFlags << kGradFlagsShift_GSF);

    buffer.writeUInt(flags);

    buffer.writeColor4fArray(fColors, fCount);
    if (colorSpaceData) {
        buffer.writeDataAsByteArray(colorSpaceData.get());
    }
    if (fPos) {
        buffer.writeScalarArray(fPos, fCount);
    }
    if (fLocalMatrix) {
        buffer.writeMatrix(*fLocalMatrix);
    }
}

bool SkGradientShaderBase::DescriptorScope::unflatten(SkReadBuffer& buffer) {
    // New gradient format. Includes floating point color, color space, densely packed flags
    uint32_t flags = buffer.readUInt();

    fTileMode = (SkShader::TileMode)((flags >> kTileModeShift_GSF) & kTileModeMask_GSF);
    fGradFlags = (flags >> kGradFlagsShift_GSF) & kGradFlagsMask_GSF;

    fCount = buffer.getArrayCount();
    if (fCount > kStorageCount) {
        size_t allocSize = (sizeof(SkColor4f) + sizeof(SkScalar)) * fCount;
        fDynamicStorage.reset(allocSize);
        fColors = (SkColor4f*)fDynamicStorage.get();
        fPos = (SkScalar*)(fColors + fCount);
    } else {
        fColors = fColorStorage;
        fPos = fPosStorage;
    }
    if (!buffer.readColor4fArray(mutableColors(), fCount)) {
        return false;
    }
    if (SkToBool(flags & kHasColorSpace_GSF)) {
        sk_sp<SkData> data = buffer.readByteArrayAsData();
        fColorSpace = SkColorSpace::Deserialize(data->data(), data->size());
    } else {
        fColorSpace = nullptr;
    }
    if (SkToBool(flags & kHasPosition_GSF)) {
        if (!buffer.readScalarArray(mutablePos(), fCount)) {
            return false;
        }
    } else {
        fPos = nullptr;
    }
    if (SkToBool(flags & kHasLocalMatrix_GSF)) {
        fLocalMatrix = &fLocalMatrixStorage;
        buffer.readMatrix(&fLocalMatrixStorage);
    } else {
        fLocalMatrix = nullptr;
    }
    return buffer.isValid();
}

////////////////////////////////////////////////////////////////////////////////////////////

SkGradientShaderBase::SkGradientShaderBase(const Descriptor& desc, const SkMatrix& ptsToUnit)
    : INHERITED(desc.fLocalMatrix)
    , fPtsToUnit(ptsToUnit)
{
    fPtsToUnit.getType();  // Precache so reads are threadsafe.
    SkASSERT(desc.fCount > 1);

    fGradFlags = static_cast<uint8_t>(desc.fGradFlags);

    SkASSERT((unsigned)desc.fTileMode < SkShader::kTileModeCount);
    SkASSERT(SkShader::kTileModeCount == SK_ARRAY_COUNT(gTileProcs));
    fTileMode = desc.fTileMode;
    fTileProc = gTileProcs[desc.fTileMode];

    /*  Note: we let the caller skip the first and/or last position.
        i.e. pos[0] = 0.3, pos[1] = 0.7
        In these cases, we insert dummy entries to ensure that the final data
        will be bracketed by [0, 1].
        i.e. our_pos[0] = 0, our_pos[1] = 0.3, our_pos[2] = 0.7, our_pos[3] = 1

        Thus colorCount (the caller's value, and fColorCount (our value) may
        differ by up to 2. In the above example:
            colorCount = 2
            fColorCount = 4
     */
    fColorCount = desc.fCount;
    // check if we need to add in dummy start and/or end position/colors
    bool dummyFirst = false;
    bool dummyLast = false;
    if (desc.fPos) {
        dummyFirst = desc.fPos[0] != 0;
        dummyLast = desc.fPos[desc.fCount - 1] != SK_Scalar1;
        fColorCount += dummyFirst + dummyLast;
    }

    if (fColorCount > kColorStorageCount) {
        size_t size = sizeof(SkColor) + sizeof(SkColor4f) + sizeof(Rec);
        if (desc.fPos) {
            size += sizeof(SkScalar);
        }
        fOrigColors = reinterpret_cast<SkColor*>(sk_malloc_throw(size * fColorCount));
    }
    else {
        fOrigColors = fStorage;
    }

    fOrigColors4f = (SkColor4f*)(fOrigColors + fColorCount);

    // Now copy over the colors, adding the dummies as needed
    SkColor4f* origColors = fOrigColors4f;
    if (dummyFirst) {
        *origColors++ = desc.fColors[0];
    }
    memcpy(origColors, desc.fColors, desc.fCount * sizeof(SkColor4f));
    if (dummyLast) {
        origColors += desc.fCount;
        *origColors = desc.fColors[desc.fCount - 1];
    }

    // Convert our SkColor4f colors to SkColor as well. Note that this is incorrect if the
    // source colors are not in sRGB gamut. We would need to do a gamut transformation, but
    // SkColorSpaceXform can't do that (yet). GrColorSpaceXform can, but we may not have GPU
    // support compiled in here. For the common case (sRGB colors), this does the right thing.
    for (int i = 0; i < fColorCount; ++i) {
        fOrigColors[i] = fOrigColors4f[i].toSkColor();
    }

    if (!desc.fColorSpace) {
        // This happens if we were constructed from SkColors, so our colors are really sRGB
        fColorSpace = SkColorSpace::MakeSRGBLinear();
    } else {
        // The color space refers to the float colors, so it must be linear gamma
        SkASSERT(desc.fColorSpace->gammaIsLinear());
        fColorSpace = desc.fColorSpace;
    }

    if (desc.fPos && fColorCount) {
        fOrigPos = (SkScalar*)(fOrigColors4f + fColorCount);
        fRecs = (Rec*)(fOrigPos + fColorCount);
    } else {
        fOrigPos = nullptr;
        fRecs = (Rec*)(fOrigColors4f + fColorCount);
    }

    if (fColorCount > 2) {
        Rec* recs = fRecs;
        recs->fPos = 0;
        //  recs->fScale = 0; // unused;
        recs += 1;
        if (desc.fPos) {
            SkScalar* origPosPtr = fOrigPos;
            *origPosPtr++ = 0;

            /*  We need to convert the user's array of relative positions into
                fixed-point positions and scale factors. We need these results
                to be strictly monotonic (no two values equal or out of order).
                Hence this complex loop that just jams a zero for the scale
                value if it sees a segment out of order, and it assures that
                we start at 0 and end at 1.0
            */
            SkScalar prev = 0;
            int startIndex = dummyFirst ? 0 : 1;
            int count = desc.fCount + dummyLast;
            for (int i = startIndex; i < count; i++) {
                // force the last value to be 1.0
                SkScalar curr;
                if (i == desc.fCount) {  // we're really at the dummyLast
                    curr = 1;
                } else {
                    curr = SkScalarPin(desc.fPos[i], 0, 1);
                }
                *origPosPtr++ = curr;

                recs->fPos = SkScalarToFixed(curr);
                SkFixed diff = SkScalarToFixed(curr - prev);
                if (diff > 0) {
                    recs->fScale = (1 << 24) / diff;
                } else {
                    recs->fScale = 0; // ignore this segment
                }
                // get ready for the next value
                prev = curr;
                recs += 1;
            }
        } else {    // assume even distribution
            fOrigPos = nullptr;

            SkFixed dp = SK_Fixed1 / (desc.fCount - 1);
            SkFixed p = dp;
            SkFixed scale = (desc.fCount - 1) << 8;  // (1 << 24) / dp
            for (int i = 1; i < desc.fCount - 1; i++) {
                recs->fPos   = p;
                recs->fScale = scale;
                recs += 1;
                p += dp;
            }
            recs->fPos = SK_Fixed1;
            recs->fScale = scale;
        }
    } else if (desc.fPos) {
        SkASSERT(2 == fColorCount);
        fOrigPos[0] = SkScalarPin(desc.fPos[0], 0, 1);
        fOrigPos[1] = SkScalarPin(desc.fPos[1], fOrigPos[0], 1);
        if (0 == fOrigPos[0] && 1 == fOrigPos[1]) {
            fOrigPos = nullptr;
        }
    }
    this->initCommon();
}

SkGradientShaderBase::~SkGradientShaderBase() {
    if (fOrigColors != fStorage) {
        sk_free(fOrigColors);
    }
}

void SkGradientShaderBase::initCommon() {
    unsigned colorAlpha = 0xFF;
    for (int i = 0; i < fColorCount; i++) {
        colorAlpha &= SkColorGetA(fOrigColors[i]);
    }
    fColorsAreOpaque = colorAlpha == 0xFF;
}

void SkGradientShaderBase::flatten(SkWriteBuffer& buffer) const {
    Descriptor desc;
    desc.fColors = fOrigColors4f;
    desc.fColorSpace = fColorSpace;
    desc.fPos = fOrigPos;
    desc.fCount = fColorCount;
    desc.fTileMode = fTileMode;
    desc.fGradFlags = fGradFlags;

    const SkMatrix& m = this->getLocalMatrix();
    desc.fLocalMatrix = m.isIdentity() ? nullptr : &m;
    desc.flatten(buffer);
}

void SkGradientShaderBase::FlipGradientColors(SkColor* colorDst, Rec* recDst,
                                              SkColor* colorSrc, Rec* recSrc,
                                              int count) {
    SkAutoSTArray<8, SkColor> colorsTemp(count);
    for (int i = 0; i < count; ++i) {
        int offset = count - i - 1;
        colorsTemp[i] = colorSrc[offset];
    }
    if (count > 2) {
        SkAutoSTArray<8, Rec> recsTemp(count);
        for (int i = 0; i < count; ++i) {
            int offset = count - i - 1;
            recsTemp[i].fPos = SK_Fixed1 - recSrc[offset].fPos;
            recsTemp[i].fScale = recSrc[offset].fScale;
        }
        memcpy(recDst, recsTemp.get(), count * sizeof(Rec));
    }
    memcpy(colorDst, colorsTemp.get(), count * sizeof(SkColor));
}

static void add_stop_color(SkJumper_GradientCtx* ctx, size_t stop, SkPM4f Fs, SkPM4f Bs) {
    (ctx->fs[0])[stop] = Fs.r();
    (ctx->fs[1])[stop] = Fs.g();
    (ctx->fs[2])[stop] = Fs.b();
    (ctx->fs[3])[stop] = Fs.a();
    (ctx->bs[0])[stop] = Bs.r();
    (ctx->bs[1])[stop] = Bs.g();
    (ctx->bs[2])[stop] = Bs.b();
    (ctx->bs[3])[stop] = Bs.a();
}

static void add_const_color(SkJumper_GradientCtx* ctx, size_t stop, SkPM4f color) {
    add_stop_color(ctx, stop, SkPM4f::FromPremulRGBA(0,0,0,0), color);
}

// Calculate a factor F and a bias B so that color = F*t + B when t is in range of
// the stop. Assume that the distance between stops is 1/gapCount.
static void init_stop_evenly(
    SkJumper_GradientCtx* ctx, float gapCount, size_t stop, SkPM4f c_l, SkPM4f c_r) {
    // Clankium's GCC 4.9 targeting ARMv7 is barfing when we use Sk4f math here, so go scalar...
    SkPM4f Fs = {{
        (c_r.r() - c_l.r()) * gapCount,
        (c_r.g() - c_l.g()) * gapCount,
        (c_r.b() - c_l.b()) * gapCount,
        (c_r.a() - c_l.a()) * gapCount,
    }};
    SkPM4f Bs = {{
        c_l.r() - Fs.r()*(stop/gapCount),
        c_l.g() - Fs.g()*(stop/gapCount),
        c_l.b() - Fs.b()*(stop/gapCount),
        c_l.a() - Fs.a()*(stop/gapCount),
    }};
    add_stop_color(ctx, stop, Fs, Bs);
}

// For each stop we calculate a bias B and a scale factor F, such that
// for any t between stops n and n+1, the color we want is B[n] + F[n]*t.
static void init_stop_pos(
    SkJumper_GradientCtx* ctx, size_t stop, float t_l, float t_r, SkPM4f c_l, SkPM4f c_r) {
    // See note about Clankium's old compiler in init_stop_evenly().
    SkPM4f Fs = {{
        (c_r.r() - c_l.r()) / (t_r - t_l),
        (c_r.g() - c_l.g()) / (t_r - t_l),
        (c_r.b() - c_l.b()) / (t_r - t_l),
        (c_r.a() - c_l.a()) / (t_r - t_l),
    }};
    SkPM4f Bs = {{
        c_l.r() - Fs.r()*t_l,
        c_l.g() - Fs.g()*t_l,
        c_l.b() - Fs.b()*t_l,
        c_l.a() - Fs.a()*t_l,
    }};
    ctx->ts[stop] = t_l;
    add_stop_color(ctx, stop, Fs, Bs);
}

bool SkGradientShaderBase::onAppendStages(SkRasterPipeline* p,
                                          SkColorSpace* dstCS,
                                          SkArenaAlloc* alloc,
                                          const SkMatrix& ctm,
                                          const SkPaint& paint,
                                          const SkMatrix* localM) const {
    SkMatrix matrix;
    if (!this->computeTotalInverse(ctm, localM, &matrix)) {
        return false;
    }

    SkRasterPipeline_<256> tPipeline;
    SkRasterPipeline_<256> postPipeline;
    if (!this->adjustMatrixAndAppendStages(alloc, &matrix, &tPipeline, &postPipeline)) {
        return false;
    }

    p->append(SkRasterPipeline::seed_shader);
    p->append_matrix(alloc, matrix);
    p->extend(tPipeline);

    switch(fTileMode) {
        case kMirror_TileMode: p->append(SkRasterPipeline::mirror_x_1); break;
        case kRepeat_TileMode: p->append(SkRasterPipeline::repeat_x_1); break;
        case kClamp_TileMode:
            if (!fOrigPos) {
                // We clamp only when the stops are evenly spaced.
                // If not, there may be hard stops, and clamping ruins hard stops at 0 and/or 1.
                // In that case, we must make sure we're using the general "gradient" stage,
                // which is the only stage that will correctly handle unclamped t.
                p->append(SkRasterPipeline::clamp_x_1);
            }
    }

    const bool premulGrad = fGradFlags & SkGradientShader::kInterpolateColorsInPremul_Flag;
    auto prepareColor = [premulGrad, dstCS, this](int i) {
        SkColor4f c = this->getXformedColor(i, dstCS);
        return premulGrad ? c.premul()
                          : SkPM4f::From4f(Sk4f::Load(&c));
    };

    // The two-stop case with stops at 0 and 1.
    if (fColorCount == 2 && fOrigPos == nullptr) {
        const SkPM4f c_l = prepareColor(0),
            c_r = prepareColor(1);

        // See F and B below.
        auto* f_and_b = alloc->makeArrayDefault<SkPM4f>(2);
        f_and_b[0] = SkPM4f::From4f(c_r.to4f() - c_l.to4f());
        f_and_b[1] = c_l;

        p->append(SkRasterPipeline::evenly_spaced_2_stop_gradient, f_and_b);
    } else {
        auto* ctx = alloc->make<SkJumper_GradientCtx>();

        // Note: In order to handle clamps in search, the search assumes a stop conceptully placed
        // at -inf. Therefore, the max number of stops is fColorCount+1.
        for (int i = 0; i < 4; i++) {
            // Allocate at least at for the AVX2 gather from a YMM register.
            ctx->fs[i] = alloc->makeArray<float>(std::max(fColorCount+1, 8));
            ctx->bs[i] = alloc->makeArray<float>(std::max(fColorCount+1, 8));
        }

        if (fOrigPos == nullptr) {
            // Handle evenly distributed stops.

            size_t stopCount = fColorCount;
            float gapCount = stopCount - 1;

            SkPM4f c_l = prepareColor(0);
            for (size_t i = 0; i < stopCount - 1; i++) {
                SkPM4f c_r = prepareColor(i + 1);
                init_stop_evenly(ctx, gapCount, i, c_l, c_r);
                c_l = c_r;
            }
            add_const_color(ctx, stopCount - 1, c_l);

            ctx->stopCount = stopCount;
            p->append(SkRasterPipeline::evenly_spaced_gradient, ctx);
        } else {
            // Handle arbitrary stops.

            ctx->ts = alloc->makeArray<float>(fColorCount+1);

            // Remove the dummy stops inserted by SkGradientShaderBase::SkGradientShaderBase
            // because they are naturally handled by the search method.
            int firstStop;
            int lastStop;
            if (fColorCount > 2) {
                firstStop = fOrigColors4f[0] != fOrigColors4f[1] ? 0 : 1;
                lastStop = fOrigColors4f[fColorCount - 2] != fOrigColors4f[fColorCount - 1]
                           ? fColorCount - 1 : fColorCount - 2;
            } else {
                firstStop = 0;
                lastStop = 1;
            }

            size_t stopCount = 0;
            float  t_l = fOrigPos[firstStop];
            SkPM4f c_l = prepareColor(firstStop);
            add_const_color(ctx, stopCount++, c_l);
            // N.B. lastStop is the index of the last stop, not one after.
            for (int i = firstStop; i < lastStop; i++) {
                float  t_r = fOrigPos[i + 1];
                SkPM4f c_r = prepareColor(i + 1);
                if (t_l < t_r) {
                    init_stop_pos(ctx, stopCount, t_l, t_r, c_l, c_r);
                    stopCount += 1;
                }
                t_l = t_r;
                c_l = c_r;
            }

            ctx->ts[stopCount] = t_l;
            add_const_color(ctx, stopCount++, c_l);

            ctx->stopCount = stopCount;
            p->append(SkRasterPipeline::gradient, ctx);
        }
    }

    if (!premulGrad && !this->colorsAreOpaque()) {
        p->append(SkRasterPipeline::premul);
    }

    p->extend(postPipeline);

    return true;
}


bool SkGradientShaderBase::isOpaque() const {
    return fColorsAreOpaque;
}

static unsigned rounded_divide(unsigned numer, unsigned denom) {
    return (numer + (denom >> 1)) / denom;
}

bool SkGradientShaderBase::onAsLuminanceColor(SkColor* lum) const {
    // we just compute an average color.
    // possibly we could weight this based on the proportional width for each color
    //   assuming they are not evenly distributed in the fPos array.
    int r = 0;
    int g = 0;
    int b = 0;
    const int n = fColorCount;
    for (int i = 0; i < n; ++i) {
        SkColor c = fOrigColors[i];
        r += SkColorGetR(c);
        g += SkColorGetG(c);
        b += SkColorGetB(c);
    }
    *lum = SkColorSetRGB(rounded_divide(r, n), rounded_divide(g, n), rounded_divide(b, n));
    return true;
}

SkGradientShaderBase::GradientShaderBaseContext::GradientShaderBaseContext(
        const SkGradientShaderBase& shader, const ContextRec& rec)
    : INHERITED(shader, rec)
#ifdef SK_SUPPORT_LEGACY_GRADIENT_DITHERING
    , fDither(true)
#else
    , fDither(rec.fPaint->isDither())
#endif
    , fCache(shader.refCache(getPaintAlpha(), fDither))
{
    const SkMatrix& inverse = this->getTotalInverse();

    fDstToIndex.setConcat(shader.fPtsToUnit, inverse);
    SkASSERT(!fDstToIndex.hasPerspective());

    fDstToIndexProc = fDstToIndex.getMapXYProc();

    // now convert our colors in to PMColors
    unsigned paintAlpha = this->getPaintAlpha();

    fFlags = this->INHERITED::getFlags();
    if (shader.fColorsAreOpaque && paintAlpha == 0xFF) {
        fFlags |= kOpaqueAlpha_Flag;
    }
}

bool SkGradientShaderBase::GradientShaderBaseContext::isValid() const {
    return fDstToIndex.isFinite();
}

SkGradientShaderBase::GradientShaderCache::GradientShaderCache(
        U8CPU alpha, bool dither, const SkGradientShaderBase& shader)
    : fCacheAlpha(alpha)
    , fCacheDither(dither)
    , fShader(shader)
{
    // Only initialize the cache in getCache32.
    fCache32 = nullptr;
}

SkGradientShaderBase::GradientShaderCache::~GradientShaderCache() {}

/*
 *  r,g,b used to be SkFixed, but on gcc (4.2.1 mac and 4.6.3 goobuntu) in
 *  release builds, we saw a compiler error where the 0xFF parameter in
 *  SkPackARGB32() was being totally ignored whenever it was called with
 *  a non-zero add (e.g. 0x8000).
 *
 *  We found two work-arounds:
 *      1. change r,g,b to unsigned (or just one of them)
 *      2. change SkPackARGB32 to + its (a << SK_A32_SHIFT) value instead
 *         of using |
 *
 *  We chose #1 just because it was more localized.
 *  See http://code.google.com/p/skia/issues/detail?id=1113
 *
 *  The type SkUFixed encapsulate this need for unsigned, but logically Fixed.
 */
typedef uint32_t SkUFixed;

void SkGradientShaderBase::GradientShaderCache::Build32bitCache(
        SkPMColor cache[], SkColor c0, SkColor c1,
        int count, U8CPU paintAlpha, uint32_t gradFlags, bool dither) {
    SkASSERT(count > 1);

    // need to apply paintAlpha to our two endpoints
    uint32_t a0 = SkMulDiv255Round(SkColorGetA(c0), paintAlpha);
    uint32_t a1 = SkMulDiv255Round(SkColorGetA(c1), paintAlpha);


    const bool interpInPremul = SkToBool(gradFlags &
                           SkGradientShader::kInterpolateColorsInPremul_Flag);

    uint32_t r0 = SkColorGetR(c0);
    uint32_t g0 = SkColorGetG(c0);
    uint32_t b0 = SkColorGetB(c0);

    uint32_t r1 = SkColorGetR(c1);
    uint32_t g1 = SkColorGetG(c1);
    uint32_t b1 = SkColorGetB(c1);

    if (interpInPremul) {
        r0 = SkMulDiv255Round(r0, a0);
        g0 = SkMulDiv255Round(g0, a0);
        b0 = SkMulDiv255Round(b0, a0);

        r1 = SkMulDiv255Round(r1, a1);
        g1 = SkMulDiv255Round(g1, a1);
        b1 = SkMulDiv255Round(b1, a1);
    }

    SkFixed da = SkIntToFixed(a1 - a0) / (count - 1);
    SkFixed dr = SkIntToFixed(r1 - r0) / (count - 1);
    SkFixed dg = SkIntToFixed(g1 - g0) / (count - 1);
    SkFixed db = SkIntToFixed(b1 - b0) / (count - 1);

    /*  We pre-add 1/8 to avoid having to add this to our [0] value each time
        in the loop. Without this, the bias for each would be
            0x2000  0xA000  0xE000  0x6000
        With this trick, we can add 0 for the first (no-op) and just adjust the
        others.
     */
    const SkUFixed bias0 = dither ? 0x2000 : 0x8000;
    const SkUFixed bias1 = dither ? 0x8000 : 0;
    const SkUFixed bias2 = dither ? 0xC000 : 0;
    const SkUFixed bias3 = dither ? 0x4000 : 0;

    SkUFixed a = SkIntToFixed(a0) + bias0;
    SkUFixed r = SkIntToFixed(r0) + bias0;
    SkUFixed g = SkIntToFixed(g0) + bias0;
    SkUFixed b = SkIntToFixed(b0) + bias0;

    /*
     *  Our dither-cell (spatially) is
     *      0 2
     *      3 1
     *  Where
     *      [0] -> [-1/8 ... 1/8 ) values near 0
     *      [1] -> [ 1/8 ... 3/8 ) values near 1/4
     *      [2] -> [ 3/8 ... 5/8 ) values near 1/2
     *      [3] -> [ 5/8 ... 7/8 ) values near 3/4
     */

    if (0xFF == a0 && 0 == da) {
        do {
            cache[kCache32Count*0] = SkPackARGB32(0xFF, (r + 0    ) >> 16,
                                                        (g + 0    ) >> 16,
                                                        (b + 0    ) >> 16);
            cache[kCache32Count*1] = SkPackARGB32(0xFF, (r + bias1) >> 16,
                                                        (g + bias1) >> 16,
                                                        (b + bias1) >> 16);
            cache[kCache32Count*2] = SkPackARGB32(0xFF, (r + bias2) >> 16,
                                                        (g + bias2) >> 16,
                                                        (b + bias2) >> 16);
            cache[kCache32Count*3] = SkPackARGB32(0xFF, (r + bias3) >> 16,
                                                        (g + bias3) >> 16,
                                                        (b + bias3) >> 16);
            cache += 1;
            r += dr;
            g += dg;
            b += db;
        } while (--count != 0);
    } else if (interpInPremul) {
        do {
            cache[kCache32Count*0] = SkPackARGB32((a + 0    ) >> 16,
                                                  (r + 0    ) >> 16,
                                                  (g + 0    ) >> 16,
                                                  (b + 0    ) >> 16);
            cache[kCache32Count*1] = SkPackARGB32((a + bias1) >> 16,
                                                  (r + bias1) >> 16,
                                                  (g + bias1) >> 16,
                                                  (b + bias1) >> 16);
            cache[kCache32Count*2] = SkPackARGB32((a + bias2) >> 16,
                                                  (r + bias2) >> 16,
                                                  (g + bias2) >> 16,
                                                  (b + bias2) >> 16);
            cache[kCache32Count*3] = SkPackARGB32((a + bias3) >> 16,
                                                  (r + bias3) >> 16,
                                                  (g + bias3) >> 16,
                                                  (b + bias3) >> 16);
            cache += 1;
            a += da;
            r += dr;
            g += dg;
            b += db;
        } while (--count != 0);
    } else {    // interpolate in unpreml space
        do {
            cache[kCache32Count*0] = SkPremultiplyARGBInline((a + 0     ) >> 16,
                                                             (r + 0     ) >> 16,
                                                             (g + 0     ) >> 16,
                                                             (b + 0     ) >> 16);
            cache[kCache32Count*1] = SkPremultiplyARGBInline((a + bias1) >> 16,
                                                             (r + bias1) >> 16,
                                                             (g + bias1) >> 16,
                                                             (b + bias1) >> 16);
            cache[kCache32Count*2] = SkPremultiplyARGBInline((a + bias2) >> 16,
                                                             (r + bias2) >> 16,
                                                             (g + bias2) >> 16,
                                                             (b + bias2) >> 16);
            cache[kCache32Count*3] = SkPremultiplyARGBInline((a + bias3) >> 16,
                                                             (r + bias3) >> 16,
                                                             (g + bias3) >> 16,
                                                             (b + bias3) >> 16);
            cache += 1;
            a += da;
            r += dr;
            g += dg;
            b += db;
        } while (--count != 0);
    }
}

static inline int SkFixedToFFFF(SkFixed x) {
    SkASSERT((unsigned)x <= SK_Fixed1);
    return x - (x >> 16);
}

const SkPMColor* SkGradientShaderBase::GradientShaderCache::getCache32() {
    fCache32InitOnce(SkGradientShaderBase::GradientShaderCache::initCache32, this);
    SkASSERT(fCache32);
    return fCache32;
}

void SkGradientShaderBase::GradientShaderCache::initCache32(GradientShaderCache* cache) {
    const int kNumberOfDitherRows = 4;
    const SkImageInfo info = SkImageInfo::MakeN32Premul(kCache32Count, kNumberOfDitherRows);

    SkASSERT(nullptr == cache->fCache32PixelRef);
    cache->fCache32PixelRef = SkMallocPixelRef::MakeAllocate(info, 0);
    cache->fCache32 = (SkPMColor*)cache->fCache32PixelRef->pixels();
    if (cache->fShader.fColorCount == 2) {
        Build32bitCache(cache->fCache32, cache->fShader.fOrigColors[0],
                        cache->fShader.fOrigColors[1], kCache32Count, cache->fCacheAlpha,
                        cache->fShader.fGradFlags, cache->fCacheDither);
    } else {
        Rec* rec = cache->fShader.fRecs;
        int prevIndex = 0;
        for (int i = 1; i < cache->fShader.fColorCount; i++) {
            int nextIndex = SkFixedToFFFF(rec[i].fPos) >> kCache32Shift;
            SkASSERT(nextIndex < kCache32Count);

            if (nextIndex > prevIndex)
                Build32bitCache(cache->fCache32 + prevIndex, cache->fShader.fOrigColors[i-1],
                                cache->fShader.fOrigColors[i], nextIndex - prevIndex + 1,
                                cache->fCacheAlpha, cache->fShader.fGradFlags, cache->fCacheDither);
            prevIndex = nextIndex;
        }
    }
}

void SkGradientShaderBase::initLinearBitmap(SkBitmap* bitmap) const {
    const bool interpInPremul = SkToBool(fGradFlags &
                                         SkGradientShader::kInterpolateColorsInPremul_Flag);
    SkHalf* pixelsF16 = reinterpret_cast<SkHalf*>(bitmap->getPixels());
    uint32_t* pixelsS32 = reinterpret_cast<uint32_t*>(bitmap->getPixels());

    typedef std::function<void(const Sk4f&, int)> pixelWriteFn_t;

    pixelWriteFn_t writeF16Pixel = [&](const Sk4f& x, int index) {
        Sk4h c = SkFloatToHalf_finite_ftz(x);
        pixelsF16[4*index+0] = c[0];
        pixelsF16[4*index+1] = c[1];
        pixelsF16[4*index+2] = c[2];
        pixelsF16[4*index+3] = c[3];
    };
    pixelWriteFn_t writeS32Pixel = [&](const Sk4f& c, int index) {
        pixelsS32[index] = Sk4f_toS32(c);
    };

    pixelWriteFn_t writeSizedPixel =
        (kRGBA_F16_SkColorType == bitmap->colorType()) ? writeF16Pixel : writeS32Pixel;
    pixelWriteFn_t writeUnpremulPixel = [&](const Sk4f& c, int index) {
        writeSizedPixel(c * Sk4f(c[3], c[3], c[3], 1.0f), index);
    };

    pixelWriteFn_t writePixel = interpInPremul ? writeSizedPixel : writeUnpremulPixel;

    int prevIndex = 0;
    for (int i = 1; i < fColorCount; i++) {
        int nextIndex = (fColorCount == 2) ? (kCache32Count - 1)
            : SkFixedToFFFF(fRecs[i].fPos) >> kCache32Shift;
        SkASSERT(nextIndex < kCache32Count);

        if (nextIndex > prevIndex) {
            Sk4f c0 = Sk4f::Load(fOrigColors4f[i - 1].vec());
            Sk4f c1 = Sk4f::Load(fOrigColors4f[i].vec());
            if (interpInPremul) {
                c0 = c0 * Sk4f(c0[3], c0[3], c0[3], 1.0f);
                c1 = c1 * Sk4f(c1[3], c1[3], c1[3], 1.0f);
            }

            Sk4f step = Sk4f(1.0f / static_cast<float>(nextIndex - prevIndex));
            Sk4f delta = (c1 - c0) * step;

            for (int curIndex = prevIndex; curIndex <= nextIndex; ++curIndex) {
                writePixel(c0, curIndex);
                c0 += delta;
            }
        }
        prevIndex = nextIndex;
    }
    SkASSERT(prevIndex == kCache32Count - 1);
}

/*
 *  The gradient holds a cache for the most recent value of alpha. Successive
 *  callers with the same alpha value will share the same cache.
 */
sk_sp<SkGradientShaderBase::GradientShaderCache> SkGradientShaderBase::refCache(U8CPU alpha,
                                                                          bool dither) const {
    SkAutoMutexAcquire ama(fCacheMutex);
    if (!fCache || fCache->getAlpha() != alpha || fCache->getDither() != dither) {
        fCache.reset(new GradientShaderCache(alpha, dither, *this));
    }
    // Increment the ref counter inside the mutex to ensure the returned pointer is still valid.
    // Otherwise, the pointer may have been overwritten on a different thread before the object's
    // ref count was incremented.
    return fCache;
}

SkColor4f SkGradientShaderBase::getXformedColor(size_t i, SkColorSpace* dstCS) const {
    return dstCS ? to_colorspace(fOrigColors4f[i], fColorSpace.get(), dstCS)
                 : SkColor4f_from_SkColor(fOrigColors[i], nullptr);
}

SK_DECLARE_STATIC_MUTEX(gGradientCacheMutex);
/*
 *  Because our caller might rebuild the same (logically the same) gradient
 *  over and over, we'd like to return exactly the same "bitmap" if possible,
 *  allowing the client to utilize a cache of our bitmap (e.g. with a GPU).
 *  To do that, we maintain a private cache of built-bitmaps, based on our
 *  colors and positions. Note: we don't try to flatten the fMapper, so if one
 *  is present, we skip the cache for now.
 */
void SkGradientShaderBase::getGradientTableBitmap(SkBitmap* bitmap,
                                                  GradientBitmapType bitmapType) const {
    // our caller assumes no external alpha, so we ensure that our cache is built with 0xFF
    sk_sp<GradientShaderCache> cache(this->refCache(0xFF, true));

    // build our key: [numColors + colors[] + {positions[]} + flags + colorType ]
    int count = 1 + fColorCount + 1 + 1;
    if (fColorCount > 2) {
        count += fColorCount - 1;    // fRecs[].fPos
    }

    SkAutoSTMalloc<16, int32_t> storage(count);
    int32_t* buffer = storage.get();

    *buffer++ = fColorCount;
    memcpy(buffer, fOrigColors, fColorCount * sizeof(SkColor));
    buffer += fColorCount;
    if (fColorCount > 2) {
        for (int i = 1; i < fColorCount; i++) {
            *buffer++ = fRecs[i].fPos;
        }
    }
    *buffer++ = fGradFlags;
    *buffer++ = static_cast<int32_t>(bitmapType);
    SkASSERT(buffer - storage.get() == count);

    ///////////////////////////////////

    static SkGradientBitmapCache* gCache;
    // each cache cost 1K or 2K of RAM, since each bitmap will be 1x256 at either 32bpp or 64bpp
    static const int MAX_NUM_CACHED_GRADIENT_BITMAPS = 32;
    SkAutoMutexAcquire ama(gGradientCacheMutex);

    if (nullptr == gCache) {
        gCache = new SkGradientBitmapCache(MAX_NUM_CACHED_GRADIENT_BITMAPS);
    }
    size_t size = count * sizeof(int32_t);

    if (!gCache->find(storage.get(), size, bitmap)) {
        if (GradientBitmapType::kLegacy == bitmapType) {
            // force our cache32pixelref to be built
            (void)cache->getCache32();
            bitmap->setInfo(SkImageInfo::MakeN32Premul(kCache32Count, 1));
            bitmap->setPixelRef(sk_ref_sp(cache->getCache32PixelRef()), 0, 0);
        } else {
            // For these cases we use the bitmap cache, but not the GradientShaderCache. So just
            // allocate and populate the bitmap's data directly.

            SkImageInfo info;
            switch (bitmapType) {
                case GradientBitmapType::kSRGB:
                    info = SkImageInfo::Make(kCache32Count, 1, kRGBA_8888_SkColorType,
                                             kPremul_SkAlphaType,
                                             SkColorSpace::MakeSRGB());
                    break;
                case GradientBitmapType::kHalfFloat:
                    info = SkImageInfo::Make(
                        kCache32Count, 1, kRGBA_F16_SkColorType, kPremul_SkAlphaType,
                        SkColorSpace::MakeSRGBLinear());
                    break;
                default:
                    SkFAIL("Unexpected bitmap type");
                    return;
            }
            bitmap->allocPixels(info);
            this->initLinearBitmap(bitmap);
        }
        gCache->add(storage.get(), size, *bitmap);
    }
}

void SkGradientShaderBase::commonAsAGradient(GradientInfo* info, bool flipGrad) const {
    if (info) {
        if (info->fColorCount >= fColorCount) {
            SkColor* colorLoc;
            Rec*     recLoc;
            SkAutoSTArray<8, SkColor> colorStorage;
            SkAutoSTArray<8, Rec> recStorage;
            if (flipGrad && (info->fColors || info->fColorOffsets)) {
                colorStorage.reset(fColorCount);
                recStorage.reset(fColorCount);
                colorLoc = colorStorage.get();
                recLoc = recStorage.get();
                FlipGradientColors(colorLoc, recLoc, fOrigColors, fRecs, fColorCount);
            } else {
                colorLoc = fOrigColors;
                recLoc = fRecs;
            }
            if (info->fColors) {
                memcpy(info->fColors, colorLoc, fColorCount * sizeof(SkColor));
            }
            if (info->fColorOffsets) {
                if (fColorCount == 2) {
                    info->fColorOffsets[0] = 0;
                    info->fColorOffsets[1] = SK_Scalar1;
                } else if (fColorCount > 2) {
                    for (int i = 0; i < fColorCount; ++i) {
                        info->fColorOffsets[i] = SkFixedToScalar(recLoc[i].fPos);
                    }
                }
            }
        }
        info->fColorCount = fColorCount;
        info->fTileMode = fTileMode;
        info->fGradientFlags = fGradFlags;
    }
}

#ifndef SK_IGNORE_TO_STRING
void SkGradientShaderBase::toString(SkString* str) const {

    str->appendf("%d colors: ", fColorCount);

    for (int i = 0; i < fColorCount; ++i) {
        str->appendHex(fOrigColors[i], 8);
        if (i < fColorCount-1) {
            str->append(", ");
        }
    }

    if (fColorCount > 2) {
        str->append(" points: (");
        for (int i = 0; i < fColorCount; ++i) {
            str->appendScalar(SkFixedToScalar(fRecs[i].fPos));
            if (i < fColorCount-1) {
                str->append(", ");
            }
        }
        str->append(")");
    }

    static const char* gTileModeName[SkShader::kTileModeCount] = {
        "clamp", "repeat", "mirror"
    };

    str->append(" ");
    str->append(gTileModeName[fTileMode]);

    this->INHERITED::toString(str);
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// Return true if these parameters are valid/legal/safe to construct a gradient
//
static bool valid_grad(const SkColor4f colors[], const SkScalar pos[], int count,
                       unsigned tileMode) {
    return nullptr != colors && count >= 1 && tileMode < (unsigned)SkShader::kTileModeCount;
}

static void desc_init(SkGradientShaderBase::Descriptor* desc,
                      const SkColor4f colors[], sk_sp<SkColorSpace> colorSpace,
                      const SkScalar pos[], int colorCount,
                      SkShader::TileMode mode, uint32_t flags, const SkMatrix* localMatrix) {
    SkASSERT(colorCount > 1);

    desc->fColors       = colors;
    desc->fColorSpace   = std::move(colorSpace);
    desc->fPos          = pos;
    desc->fCount        = colorCount;
    desc->fTileMode     = mode;
    desc->fGradFlags    = flags;
    desc->fLocalMatrix  = localMatrix;
}

// assumes colors is SkColor4f* and pos is SkScalar*
#define EXPAND_1_COLOR(count)                \
     SkColor4f tmp[2];                       \
     do {                                    \
         if (1 == count) {                   \
             tmp[0] = tmp[1] = colors[0];    \
             colors = tmp;                   \
             pos = nullptr;                  \
             count = 2;                      \
         }                                   \
     } while (0)

struct ColorStopOptimizer {
    ColorStopOptimizer(const SkColor4f* colors, const SkScalar* pos,
                       int count, SkShader::TileMode mode)
        : fColors(colors)
        , fPos(pos)
        , fCount(count) {

            if (!pos || count != 3) {
                return;
            }

            if (SkScalarNearlyEqual(pos[0], 0.0f) &&
                SkScalarNearlyEqual(pos[1], 0.0f) &&
                SkScalarNearlyEqual(pos[2], 1.0f)) {

                if (SkShader::kRepeat_TileMode == mode ||
                    SkShader::kMirror_TileMode == mode ||
                    colors[0] == colors[1]) {

                    // Ignore the leftmost color/pos.
                    fColors += 1;
                    fPos    += 1;
                    fCount   = 2;
                }
            } else if (SkScalarNearlyEqual(pos[0], 0.0f) &&
                       SkScalarNearlyEqual(pos[1], 1.0f) &&
                       SkScalarNearlyEqual(pos[2], 1.0f)) {

                if (SkShader::kRepeat_TileMode == mode ||
                    SkShader::kMirror_TileMode == mode ||
                    colors[1] == colors[2]) {

                    // Ignore the rightmost color/pos.
                    fCount  = 2;
                }
            }
    }

    const SkColor4f* fColors;
    const SkScalar*  fPos;
    int              fCount;
};

struct ColorConverter {
    ColorConverter(const SkColor* colors, int count) {
        for (int i = 0; i < count; ++i) {
            fColors4f.push_back(SkColor4f::FromColor(colors[i]));
        }
    }

    SkSTArray<2, SkColor4f, true> fColors4f;
};

sk_sp<SkShader> SkGradientShader::MakeLinear(const SkPoint pts[2],
                                             const SkColor colors[],
                                             const SkScalar pos[], int colorCount,
                                             SkShader::TileMode mode,
                                             uint32_t flags,
                                             const SkMatrix* localMatrix) {
    ColorConverter converter(colors, colorCount);
    return MakeLinear(pts, converter.fColors4f.begin(), nullptr, pos, colorCount, mode, flags,
                      localMatrix);
}

sk_sp<SkShader> SkGradientShader::MakeLinear(const SkPoint pts[2],
                                             const SkColor4f colors[],
                                             sk_sp<SkColorSpace> colorSpace,
                                             const SkScalar pos[], int colorCount,
                                             SkShader::TileMode mode,
                                             uint32_t flags,
                                             const SkMatrix* localMatrix) {
    if (!pts || !SkScalarIsFinite((pts[1] - pts[0]).length())) {
        return nullptr;
    }
    if (!valid_grad(colors, pos, colorCount, mode)) {
        return nullptr;
    }
    if (1 == colorCount) {
        return SkShader::MakeColorShader(colors[0], std::move(colorSpace));
    }
    if (localMatrix && !localMatrix->invert(nullptr)) {
        return nullptr;
    }

    ColorStopOptimizer opt(colors, pos, colorCount, mode);

    SkGradientShaderBase::Descriptor desc;
    desc_init(&desc, opt.fColors, std::move(colorSpace), opt.fPos, opt.fCount, mode, flags,
              localMatrix);
    return sk_make_sp<SkLinearGradient>(pts, desc);
}

sk_sp<SkShader> SkGradientShader::MakeRadial(const SkPoint& center, SkScalar radius,
                                             const SkColor colors[],
                                             const SkScalar pos[], int colorCount,
                                             SkShader::TileMode mode,
                                             uint32_t flags,
                                             const SkMatrix* localMatrix) {
    ColorConverter converter(colors, colorCount);
    return MakeRadial(center, radius, converter.fColors4f.begin(), nullptr, pos, colorCount, mode,
                      flags, localMatrix);
}

sk_sp<SkShader> SkGradientShader::MakeRadial(const SkPoint& center, SkScalar radius,
                                             const SkColor4f colors[],
                                             sk_sp<SkColorSpace> colorSpace,
                                             const SkScalar pos[], int colorCount,
                                             SkShader::TileMode mode,
                                             uint32_t flags,
                                             const SkMatrix* localMatrix) {
    if (radius <= 0) {
        return nullptr;
    }
    if (!valid_grad(colors, pos, colorCount, mode)) {
        return nullptr;
    }
    if (1 == colorCount) {
        return SkShader::MakeColorShader(colors[0], std::move(colorSpace));
    }
    if (localMatrix && !localMatrix->invert(nullptr)) {
        return nullptr;
    }

    ColorStopOptimizer opt(colors, pos, colorCount, mode);

    SkGradientShaderBase::Descriptor desc;
    desc_init(&desc, opt.fColors, std::move(colorSpace), opt.fPos, opt.fCount, mode, flags,
              localMatrix);
    return sk_make_sp<SkRadialGradient>(center, radius, desc);
}

sk_sp<SkShader> SkGradientShader::MakeTwoPointConical(const SkPoint& start,
                                                      SkScalar startRadius,
                                                      const SkPoint& end,
                                                      SkScalar endRadius,
                                                      const SkColor colors[],
                                                      const SkScalar pos[],
                                                      int colorCount,
                                                      SkShader::TileMode mode,
                                                      uint32_t flags,
                                                      const SkMatrix* localMatrix) {
    ColorConverter converter(colors, colorCount);
    return MakeTwoPointConical(start, startRadius, end, endRadius, converter.fColors4f.begin(),
                               nullptr, pos, colorCount, mode, flags, localMatrix);
}

sk_sp<SkShader> SkGradientShader::MakeTwoPointConical(const SkPoint& start,
                                                      SkScalar startRadius,
                                                      const SkPoint& end,
                                                      SkScalar endRadius,
                                                      const SkColor4f colors[],
                                                      sk_sp<SkColorSpace> colorSpace,
                                                      const SkScalar pos[],
                                                      int colorCount,
                                                      SkShader::TileMode mode,
                                                      uint32_t flags,
                                                      const SkMatrix* localMatrix) {
    if (startRadius < 0 || endRadius < 0) {
        return nullptr;
    }
    if (SkScalarNearlyZero((start - end).length()) && SkScalarNearlyZero(startRadius)) {
        // We can treat this gradient as radial, which is faster.
        return MakeRadial(start, endRadius, colors, std::move(colorSpace), pos, colorCount,
                          mode, flags, localMatrix);
    }
    if (!valid_grad(colors, pos, colorCount, mode)) {
        return nullptr;
    }
    if (startRadius == endRadius) {
        if (start == end || startRadius == 0) {
            return SkShader::MakeEmptyShader();
        }
    }
    if (localMatrix && !localMatrix->invert(nullptr)) {
        return nullptr;
    }
    EXPAND_1_COLOR(colorCount);

    ColorStopOptimizer opt(colors, pos, colorCount, mode);

    bool flipGradient = startRadius > endRadius;

    SkGradientShaderBase::Descriptor desc;

    if (!flipGradient) {
        desc_init(&desc, opt.fColors, std::move(colorSpace), opt.fPos, opt.fCount, mode, flags,
                  localMatrix);
        return sk_make_sp<SkTwoPointConicalGradient>(start, startRadius, end, endRadius,
                                                     flipGradient, desc);
    } else {
        SkAutoSTArray<8, SkColor4f> colorsNew(opt.fCount);
        SkAutoSTArray<8, SkScalar> posNew(opt.fCount);
        for (int i = 0; i < opt.fCount; ++i) {
            colorsNew[i] = opt.fColors[opt.fCount - i - 1];
        }

        if (pos) {
            for (int i = 0; i < opt.fCount; ++i) {
                posNew[i] = 1 - opt.fPos[opt.fCount - i - 1];
            }
            desc_init(&desc, colorsNew.get(), std::move(colorSpace), posNew.get(), opt.fCount, mode,
                      flags, localMatrix);
        } else {
            desc_init(&desc, colorsNew.get(), std::move(colorSpace), nullptr, opt.fCount, mode,
                      flags, localMatrix);
        }

        return sk_make_sp<SkTwoPointConicalGradient>(end, endRadius, start, startRadius,
                                                     flipGradient, desc);
    }
}

sk_sp<SkShader> SkGradientShader::MakeSweep(SkScalar cx, SkScalar cy,
                                            const SkColor colors[],
                                            const SkScalar pos[],
                                            int colorCount,
                                            uint32_t flags,
                                            const SkMatrix* localMatrix) {
    ColorConverter converter(colors, colorCount);
    return MakeSweep(cx, cy, converter.fColors4f.begin(), nullptr, pos, colorCount, flags,
                     localMatrix);
}

sk_sp<SkShader> SkGradientShader::MakeSweep(SkScalar cx, SkScalar cy,
                                            const SkColor4f colors[],
                                            sk_sp<SkColorSpace> colorSpace,
                                            const SkScalar pos[],
                                            int colorCount,
                                            uint32_t flags,
                                            const SkMatrix* localMatrix) {
    if (!valid_grad(colors, pos, colorCount, SkShader::kClamp_TileMode)) {
        return nullptr;
    }
    if (1 == colorCount) {
        return SkShader::MakeColorShader(colors[0], std::move(colorSpace));
    }
    if (localMatrix && !localMatrix->invert(nullptr)) {
        return nullptr;
    }

    auto mode = SkShader::kClamp_TileMode;

    ColorStopOptimizer opt(colors, pos, colorCount, mode);

    SkGradientShaderBase::Descriptor desc;
    desc_init(&desc, opt.fColors, std::move(colorSpace), opt.fPos, opt.fCount, mode, flags,
              localMatrix);
    return sk_make_sp<SkSweepGradient>(cx, cy, desc);
}

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkGradientShader)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLinearGradient)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkRadialGradient)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkSweepGradient)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkTwoPointConicalGradient)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END

///////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrShaderCaps.h"
#include "GrTextureStripAtlas.h"
#include "gl/GrGLContext.h"
#include "glsl/GrGLSLColorSpaceXformHelper.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "SkGr.h"

static inline bool close_to_one_half(const SkFixed& val) {
    return SkScalarNearlyEqual(SkFixedToScalar(val), SK_ScalarHalf);
}

static inline int color_type_to_color_count(GrGradientEffect::ColorType colorType) {
    switch (colorType) {
#if GR_GL_USE_ACCURATE_HARD_STOP_GRADIENTS
        case GrGradientEffect::kSingleHardStop_ColorType:
            return 4;
        case GrGradientEffect::kHardStopLeftEdged_ColorType:
        case GrGradientEffect::kHardStopRightEdged_ColorType:
            return 3;
#endif
        case GrGradientEffect::kTwo_ColorType:
            return 2;
        case GrGradientEffect::kThree_ColorType:
            return 3;
        case GrGradientEffect::kTexture_ColorType:
            return 0;
    }

    SkDEBUGFAIL("Unhandled ColorType in color_type_to_color_count()");
    return -1;
}

GrGradientEffect::ColorType GrGradientEffect::determineColorType(
        const SkGradientShaderBase& shader) {
#if GR_GL_USE_ACCURATE_HARD_STOP_GRADIENTS
    if (shader.fOrigPos) {
        if (4 == shader.fColorCount) {
            if (SkScalarNearlyEqual(shader.fOrigPos[0], 0.0f) &&
                SkScalarNearlyEqual(shader.fOrigPos[1], shader.fOrigPos[2]) &&
                SkScalarNearlyEqual(shader.fOrigPos[3], 1.0f)) {

                return kSingleHardStop_ColorType;
            }
        } else if (3 == shader.fColorCount) {
            if (SkScalarNearlyEqual(shader.fOrigPos[0], 0.0f) &&
                SkScalarNearlyEqual(shader.fOrigPos[1], 0.0f) &&
                SkScalarNearlyEqual(shader.fOrigPos[2], 1.0f)) {

                return kHardStopLeftEdged_ColorType;
            } else if (SkScalarNearlyEqual(shader.fOrigPos[0], 0.0f) &&
                       SkScalarNearlyEqual(shader.fOrigPos[1], 1.0f) &&
                       SkScalarNearlyEqual(shader.fOrigPos[2], 1.0f)) {

                return kHardStopRightEdged_ColorType;
            }
        }
    }
#endif

    if (SkShader::kClamp_TileMode == shader.getTileMode()) {
        if (2 == shader.fColorCount) {
            return kTwo_ColorType;
        } else if (3 == shader.fColorCount &&
                   close_to_one_half(shader.getRecs()[1].fPos)) {
            return kThree_ColorType;
        }
    }

    return kTexture_ColorType;
}

void GrGradientEffect::GLSLProcessor::emitUniforms(GrGLSLUniformHandler* uniformHandler,
                                                   const GrGradientEffect& ge) {
    if (int colorCount = color_type_to_color_count(ge.getColorType())) {
        fColorsUni = uniformHandler->addUniformArray(kFragment_GrShaderFlag,
                                                     kVec4f_GrSLType,
                                                     kDefault_GrSLPrecision,
                                                     "Colors",
                                                     colorCount);
        if (ge.fColorType == kSingleHardStop_ColorType) {
            fHardStopT = uniformHandler->addUniform(kFragment_GrShaderFlag, kFloat_GrSLType,
                                                    kDefault_GrSLPrecision, "HardStopT");
        }
    } else {
        fFSYUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                             kFloat_GrSLType, kDefault_GrSLPrecision,
                                             "GradientYCoordFS");
    }
}

static inline void set_after_interp_color_uni_array(
                                                  const GrGLSLProgramDataManager& pdman,
                                                  const GrGLSLProgramDataManager::UniformHandle uni,
                                                  const SkTDArray<SkColor4f>& colors,
                                                  const GrColorSpaceXform* colorSpaceXform) {
    int count = colors.count();
    if (colorSpaceXform) {
        constexpr int kSmallCount = 10;
        SkAutoSTArray<4 * kSmallCount, float> vals(4 * count);

        for (int i = 0; i < count; i++) {
            colorSpaceXform->srcToDst().mapScalars(colors[i].vec(), &vals[4 * i]);
        }

        pdman.set4fv(uni, count, vals.get());
    } else {
        pdman.set4fv(uni, count, (float*)&colors[0]);
    }
}

static inline void set_before_interp_color_uni_array(
                                                  const GrGLSLProgramDataManager& pdman,
                                                  const GrGLSLProgramDataManager::UniformHandle uni,
                                                  const SkTDArray<SkColor4f>& colors,
                                                  const GrColorSpaceXform* colorSpaceXform) {
    int count = colors.count();
    constexpr int kSmallCount = 10;
    SkAutoSTArray<4 * kSmallCount, float> vals(4 * count);

    for (int i = 0; i < count; i++) {
        float a = colors[i].fA;
        vals[4 * i + 0] = colors[i].fR * a;
        vals[4 * i + 1] = colors[i].fG * a;
        vals[4 * i + 2] = colors[i].fB * a;
        vals[4 * i + 3] = a;
    }

    if (colorSpaceXform) {
        for (int i = 0; i < count; i++) {
            colorSpaceXform->srcToDst().mapScalars(&vals[4 * i]);
        }
    }

    pdman.set4fv(uni, count, vals.get());
}

static inline void set_after_interp_color_uni_array(const GrGLSLProgramDataManager& pdman,
                                       const GrGLSLProgramDataManager::UniformHandle uni,
                                       const SkTDArray<SkColor>& colors) {
    int count = colors.count();
    constexpr int kSmallCount = 10;

    SkAutoSTArray<4*kSmallCount, float> vals(4*count);

    for (int i = 0; i < colors.count(); i++) {
        // RGBA
        vals[4*i + 0] = SkColorGetR(colors[i]) / 255.f;
        vals[4*i + 1] = SkColorGetG(colors[i]) / 255.f;
        vals[4*i + 2] = SkColorGetB(colors[i]) / 255.f;
        vals[4*i + 3] = SkColorGetA(colors[i]) / 255.f;
    }

    pdman.set4fv(uni, colors.count(), vals.get());
}

static inline void set_before_interp_color_uni_array(const GrGLSLProgramDataManager& pdman,
                                              const GrGLSLProgramDataManager::UniformHandle uni,
                                              const SkTDArray<SkColor>& colors) {
    int count = colors.count();
    constexpr int kSmallCount = 10;

    SkAutoSTArray<4*kSmallCount, float> vals(4*count);

    for (int i = 0; i < count; i++) {
        float a = SkColorGetA(colors[i]) / 255.f;
        float aDiv255 = a / 255.f;

        // RGBA
        vals[4*i + 0] = SkColorGetR(colors[i]) * aDiv255;
        vals[4*i + 1] = SkColorGetG(colors[i]) * aDiv255;
        vals[4*i + 2] = SkColorGetB(colors[i]) * aDiv255;
        vals[4*i + 3] = a;
    }

    pdman.set4fv(uni, count, vals.get());
}

void GrGradientEffect::GLSLProcessor::onSetData(const GrGLSLProgramDataManager& pdman,
                                                const GrFragmentProcessor& processor) {
    const GrGradientEffect& e = processor.cast<GrGradientEffect>();

    switch (e.getColorType()) {
#if GR_GL_USE_ACCURATE_HARD_STOP_GRADIENTS
        case GrGradientEffect::kSingleHardStop_ColorType:
            pdman.set1f(fHardStopT, e.fPositions[1]);
            // fall through
        case GrGradientEffect::kHardStopLeftEdged_ColorType:
        case GrGradientEffect::kHardStopRightEdged_ColorType:
#endif
        case GrGradientEffect::kTwo_ColorType:
        case GrGradientEffect::kThree_ColorType: {
            if (e.fColors4f.count() > 0) {
                // Gamma-correct / color-space aware
                if (GrGradientEffect::kBeforeInterp_PremulType == e.getPremulType()) {
                    set_before_interp_color_uni_array(pdman, fColorsUni, e.fColors4f,
                                                      e.fColorSpaceXform.get());
                } else {
                    set_after_interp_color_uni_array(pdman, fColorsUni, e.fColors4f,
                                                     e.fColorSpaceXform.get());
                }
            } else {
                // Legacy mode. Would be nice if we had converted the 8-bit colors to float earlier
                if (GrGradientEffect::kBeforeInterp_PremulType == e.getPremulType()) {
                    set_before_interp_color_uni_array(pdman, fColorsUni, e.fColors);
                } else {
                    set_after_interp_color_uni_array(pdman, fColorsUni, e.fColors);
                }
            }

            break;
        }

        case GrGradientEffect::kTexture_ColorType: {
            SkScalar yCoord = e.getYCoord();
            if (yCoord != fCachedYCoord) {
                pdman.set1f(fFSYUni, yCoord);
                fCachedYCoord = yCoord;
            }
            if (SkToBool(e.fColorSpaceXform)) {
                fColorSpaceHelper.setData(pdman, e.fColorSpaceXform.get());
            }
            break;
        }
    }
}

uint32_t GrGradientEffect::GLSLProcessor::GenBaseGradientKey(const GrProcessor& processor) {
    const GrGradientEffect& e = processor.cast<GrGradientEffect>();

    uint32_t key = 0;

    if (GrGradientEffect::kBeforeInterp_PremulType == e.getPremulType()) {
        key |= kPremulBeforeInterpKey;
    }

    if (GrGradientEffect::kTwo_ColorType == e.getColorType()) {
        key |= kTwoColorKey;
    } else if (GrGradientEffect::kThree_ColorType == e.getColorType()) {
        key |= kThreeColorKey;
    }
#if GR_GL_USE_ACCURATE_HARD_STOP_GRADIENTS
    else if (GrGradientEffect::kSingleHardStop_ColorType == e.getColorType()) {
        key |= kHardStopCenteredKey;
    } else if (GrGradientEffect::kHardStopLeftEdged_ColorType == e.getColorType()) {
        key |= kHardStopZeroZeroOneKey;
    } else if (GrGradientEffect::kHardStopRightEdged_ColorType == e.getColorType()) {
        key |= kHardStopZeroOneOneKey;
    }

    if (SkShader::TileMode::kClamp_TileMode == e.fTileMode) {
        key |= kClampTileMode;
    } else if (SkShader::TileMode::kRepeat_TileMode == e.fTileMode) {
        key |= kRepeatTileMode;
    } else {
        key |= kMirrorTileMode;
    }
#endif

    key |= GrColorSpaceXform::XformKey(e.fColorSpaceXform.get()) << kReservedBits;

    return key;
}

void GrGradientEffect::GLSLProcessor::emitColor(GrGLSLFPFragmentBuilder* fragBuilder,
                                                GrGLSLUniformHandler* uniformHandler,
                                                const GrShaderCaps* shaderCaps,
                                                const GrGradientEffect& ge,
                                                const char* gradientTValue,
                                                const char* outputColor,
                                                const char* inputColor,
                                                const TextureSamplers& texSamplers) {
    switch (ge.getColorType()) {
#if GR_GL_USE_ACCURATE_HARD_STOP_GRADIENTS
        case kSingleHardStop_ColorType: {
            const char* t      = gradientTValue;
            const char* colors = uniformHandler->getUniformCStr(fColorsUni);
            const char* stopT = uniformHandler->getUniformCStr(fHardStopT);

            fragBuilder->codeAppendf("float clamp_t = clamp(%s, 0.0, 1.0);", t);

            // Account for tile mode
            if (SkShader::kRepeat_TileMode == ge.fTileMode) {
                fragBuilder->codeAppendf("clamp_t = fract(%s);", t);
            } else if (SkShader::kMirror_TileMode == ge.fTileMode) {
                fragBuilder->codeAppendf("if (%s < 0.0 || %s > 1.0) {", t, t);
                fragBuilder->codeAppendf("    if (mod(floor(%s), 2.0) == 0.0) {", t);
                fragBuilder->codeAppendf("        clamp_t = fract(%s);", t);
                fragBuilder->codeAppendf("    } else {");
                fragBuilder->codeAppendf("        clamp_t = 1.0 - fract(%s);", t);
                fragBuilder->codeAppendf("    }");
                fragBuilder->codeAppendf("}");
            }

            // Calculate color
            fragBuilder->codeAppend ("vec4 start, end;");
            fragBuilder->codeAppend ("float relative_t;");
            fragBuilder->codeAppendf("if (clamp_t < %s) {", stopT);
            fragBuilder->codeAppendf("    start = %s[0];", colors);
            fragBuilder->codeAppendf("    end   = %s[1];", colors);
            fragBuilder->codeAppendf("    relative_t = clamp_t / %s;", stopT);
            fragBuilder->codeAppend ("} else {");
            fragBuilder->codeAppendf("    start = %s[2];", colors);
            fragBuilder->codeAppendf("    end   = %s[3];", colors);
            fragBuilder->codeAppendf("    relative_t = (clamp_t - %s) / (1 - %s);", stopT, stopT);
            fragBuilder->codeAppend ("}");
            fragBuilder->codeAppend ("vec4 colorTemp = mix(start, end, relative_t);");

            if (GrGradientEffect::kAfterInterp_PremulType == ge.getPremulType()) {
                fragBuilder->codeAppend("colorTemp.rgb *= colorTemp.a;");
            }
            if (ge.fColorSpaceXform) {
                fragBuilder->codeAppend("colorTemp.rgb = clamp(colorTemp.rgb, 0, colorTemp.a);");
            }
            fragBuilder->codeAppendf("%s = %s * colorTemp;", outputColor, inputColor);

            break;
        }

        case kHardStopLeftEdged_ColorType: {
            const char* t      = gradientTValue;
            const char* colors = uniformHandler->getUniformCStr(fColorsUni);

            fragBuilder->codeAppendf("float clamp_t = clamp(%s, 0.0, 1.0);", t);

            // Account for tile mode
            if (SkShader::kRepeat_TileMode == ge.fTileMode) {
                fragBuilder->codeAppendf("clamp_t = fract(%s);", t);
            } else if (SkShader::kMirror_TileMode == ge.fTileMode) {
                fragBuilder->codeAppendf("if (%s < 0.0 || %s > 1.0) {", t, t);
                fragBuilder->codeAppendf("    if (mod(floor(%s), 2.0) == 0.0) {", t);
                fragBuilder->codeAppendf("        clamp_t = fract(%s);", t);
                fragBuilder->codeAppendf("    } else {");
                fragBuilder->codeAppendf("        clamp_t = 1.0 - fract(%s);", t);
                fragBuilder->codeAppendf("    }");
                fragBuilder->codeAppendf("}");
            }

            fragBuilder->codeAppendf("vec4 colorTemp = mix(%s[1], %s[2], clamp_t);", colors,
                                     colors);
            if (SkShader::kClamp_TileMode == ge.fTileMode) {
                fragBuilder->codeAppendf("if (%s < 0.0) {", t);
                fragBuilder->codeAppendf("    colorTemp = %s[0];", colors);
                fragBuilder->codeAppendf("}");
            }

            if (GrGradientEffect::kAfterInterp_PremulType == ge.getPremulType()) {
                fragBuilder->codeAppend("colorTemp.rgb *= colorTemp.a;");
            }
            if (ge.fColorSpaceXform) {
                fragBuilder->codeAppend("colorTemp.rgb = clamp(colorTemp.rgb, 0, colorTemp.a);");
            }
            fragBuilder->codeAppendf("%s = %s * colorTemp;", outputColor, inputColor);

            break;
        }

        case kHardStopRightEdged_ColorType: {
            const char* t      = gradientTValue;
            const char* colors = uniformHandler->getUniformCStr(fColorsUni);

            fragBuilder->codeAppendf("float clamp_t = clamp(%s, 0.0, 1.0);", t);

            // Account for tile mode
            if (SkShader::kRepeat_TileMode == ge.fTileMode) {
                fragBuilder->codeAppendf("clamp_t = fract(%s);", t);
            } else if (SkShader::kMirror_TileMode == ge.fTileMode) {
                fragBuilder->codeAppendf("if (%s < 0.0 || %s > 1.0) {", t, t);
                fragBuilder->codeAppendf("    if (mod(floor(%s), 2.0) == 0.0) {", t);
                fragBuilder->codeAppendf("        clamp_t = fract(%s);", t);
                fragBuilder->codeAppendf("    } else {");
                fragBuilder->codeAppendf("        clamp_t = 1.0 - fract(%s);", t);
                fragBuilder->codeAppendf("    }");
                fragBuilder->codeAppendf("}");
            }

            fragBuilder->codeAppendf("vec4 colorTemp = mix(%s[0], %s[1], clamp_t);", colors,
                                     colors);
            if (SkShader::kClamp_TileMode == ge.fTileMode) {
                fragBuilder->codeAppendf("if (%s > 1.0) {", t);
                fragBuilder->codeAppendf("    colorTemp = %s[2];", colors);
                fragBuilder->codeAppendf("}");
            }

            if (GrGradientEffect::kAfterInterp_PremulType == ge.getPremulType()) {
                fragBuilder->codeAppend("colorTemp.rgb *= colorTemp.a;");
            }
            if (ge.fColorSpaceXform) {
                fragBuilder->codeAppend("colorTemp.rgb = clamp(colorTemp.rgb, 0, colorTemp.a);");
            }
            fragBuilder->codeAppendf("%s = %s * colorTemp;", outputColor, inputColor);

            break;
        }
#endif

        case kTwo_ColorType: {
            const char* t      = gradientTValue;
            const char* colors = uniformHandler->getUniformCStr(fColorsUni);

            fragBuilder->codeAppendf("vec4 colorTemp = mix(%s[0], %s[1], clamp(%s, 0.0, 1.0));",
                                     colors, colors, t);

            // We could skip this step if both colors are known to be opaque. Two
            // considerations:
            // The gradient SkShader reporting opaque is more restrictive than necessary in the two
            // pt case. Make sure the key reflects this optimization (and note that it can use the
            // same shader as thekBeforeIterp case). This same optimization applies to the 3 color
            // case below.
            if (GrGradientEffect::kAfterInterp_PremulType == ge.getPremulType()) {
                fragBuilder->codeAppend("colorTemp.rgb *= colorTemp.a;");
            }
            if (ge.fColorSpaceXform) {
                fragBuilder->codeAppend("colorTemp.rgb = clamp(colorTemp.rgb, 0, colorTemp.a);");
            }

            fragBuilder->codeAppendf("%s = %s * colorTemp;", outputColor, inputColor);

            break;
        }

        case kThree_ColorType: {
            const char* t      = gradientTValue;
            const char* colors = uniformHandler->getUniformCStr(fColorsUni);

            fragBuilder->codeAppendf("float oneMinus2t = 1.0 - (2.0 * %s);", t);
            fragBuilder->codeAppendf("vec4 colorTemp = clamp(oneMinus2t, 0.0, 1.0) * %s[0];",
                                     colors);
            if (!shaderCaps->canUseMinAndAbsTogether()) {
                // The Tegra3 compiler will sometimes never return if we have
                // min(abs(oneMinus2t), 1.0), or do the abs first in a separate expression.
                fragBuilder->codeAppendf("float minAbs = abs(oneMinus2t);");
                fragBuilder->codeAppendf("minAbs = minAbs > 1.0 ? 1.0 : minAbs;");
                fragBuilder->codeAppendf("colorTemp += (1.0 - minAbs) * %s[1];", colors);
            } else {
                fragBuilder->codeAppendf("colorTemp += (1.0 - min(abs(oneMinus2t), 1.0)) * %s[1];",
                                         colors);
            }
            fragBuilder->codeAppendf("colorTemp += clamp(-oneMinus2t, 0.0, 1.0) * %s[2];", colors);

            if (GrGradientEffect::kAfterInterp_PremulType == ge.getPremulType()) {
                fragBuilder->codeAppend("colorTemp.rgb *= colorTemp.a;");
            }
            if (ge.fColorSpaceXform) {
                fragBuilder->codeAppend("colorTemp.rgb = clamp(colorTemp.rgb, 0, colorTemp.a);");
            }

            fragBuilder->codeAppendf("%s = %s * colorTemp;", outputColor, inputColor);

            break;
        }

        case kTexture_ColorType: {
            fColorSpaceHelper.emitCode(uniformHandler, ge.fColorSpaceXform.get());

            const char* fsyuni = uniformHandler->getUniformCStr(fFSYUni);

            fragBuilder->codeAppendf("vec2 coord = vec2(%s, %s);", gradientTValue, fsyuni);
            fragBuilder->codeAppendf("%s = ", outputColor);
            fragBuilder->appendTextureLookupAndModulate(inputColor, texSamplers[0], "coord",
                                                        kVec2f_GrSLType, &fColorSpaceHelper);
            fragBuilder->codeAppend(";");

            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////

inline GrFragmentProcessor::OptimizationFlags GrGradientEffect::OptFlags(bool isOpaque) {
    return isOpaque
                   ? kPreservesOpaqueInput_OptimizationFlag |
                             kCompatibleWithCoverageAsAlpha_OptimizationFlag
                   : kCompatibleWithCoverageAsAlpha_OptimizationFlag;
}

GrGradientEffect::GrGradientEffect(const CreateArgs& args, bool isOpaque)
        : INHERITED(OptFlags(isOpaque)) {
    const SkGradientShaderBase& shader(*args.fShader);

    fIsOpaque = shader.isOpaque();

    fColorType = this->determineColorType(shader);
    fColorSpaceXform = std::move(args.fColorSpaceXform);

    if (kTexture_ColorType != fColorType) {
        SkASSERT(shader.fOrigColors && shader.fOrigColors4f);
        if (args.fGammaCorrect) {
            fColors4f = SkTDArray<SkColor4f>(shader.fOrigColors4f, shader.fColorCount);
        } else {
            fColors = SkTDArray<SkColor>(shader.fOrigColors, shader.fColorCount);
        }

#if GR_GL_USE_ACCURATE_HARD_STOP_GRADIENTS
        if (shader.fOrigPos) {
            fPositions = SkTDArray<SkScalar>(shader.fOrigPos, shader.fColorCount);
        }
#endif
    }

#if GR_GL_USE_ACCURATE_HARD_STOP_GRADIENTS
    fTileMode = args.fTileMode;
#endif

    switch (fColorType) {
        // The two and three color specializations do not currently support tiling.
        case kTwo_ColorType:
        case kThree_ColorType:
#if GR_GL_USE_ACCURATE_HARD_STOP_GRADIENTS
        case kHardStopLeftEdged_ColorType:
        case kHardStopRightEdged_ColorType:
        case kSingleHardStop_ColorType:
#endif
            fRow = -1;

            if (SkGradientShader::kInterpolateColorsInPremul_Flag & shader.getGradFlags()) {
                fPremulType = kBeforeInterp_PremulType;
            } else {
                fPremulType = kAfterInterp_PremulType;
            }

            fCoordTransform.reset(*args.fMatrix);

            break;
        case kTexture_ColorType:
            // doesn't matter how this is set, just be consistent because it is part of the
            // effect key.
            fPremulType = kBeforeInterp_PremulType;

            SkGradientShaderBase::GradientBitmapType bitmapType =
                SkGradientShaderBase::GradientBitmapType::kLegacy;
            if (args.fGammaCorrect) {
                // Try to use F16 if we can
                if (args.fContext->caps()->isConfigTexturable(kRGBA_half_GrPixelConfig)) {
                    bitmapType = SkGradientShaderBase::GradientBitmapType::kHalfFloat;
                } else if (args.fContext->caps()->isConfigTexturable(kSRGBA_8888_GrPixelConfig)) {
                    bitmapType = SkGradientShaderBase::GradientBitmapType::kSRGB;
                } else {
                    // This can happen, but only if someone explicitly creates an unsupported
                    // (eg sRGB) surface. Just fall back to legacy behavior.
                }
            }

            SkBitmap bitmap;
            shader.getGradientTableBitmap(&bitmap, bitmapType);
            SkASSERT(1 == bitmap.height() && SkIsPow2(bitmap.width()));


            GrTextureStripAtlas::Desc desc;
            desc.fWidth  = bitmap.width();
            desc.fHeight = 32;
            desc.fRowHeight = bitmap.height();
            desc.fContext = args.fContext;
            desc.fConfig = SkImageInfo2GrPixelConfig(bitmap.info(), *args.fContext->caps());
            fAtlas = GrTextureStripAtlas::GetAtlas(desc);
            SkASSERT(fAtlas);

            // We always filter the gradient table. Each table is one row of a texture, always
            // y-clamp.
            GrSamplerParams params;
            params.setFilterMode(GrSamplerParams::kBilerp_FilterMode);
            params.setTileModeX(args.fTileMode);

            fRow = fAtlas->lockRow(bitmap);
            if (-1 != fRow) {
                fYCoord = fAtlas->getYOffset(fRow)+SK_ScalarHalf*fAtlas->getNormalizedTexelHeight();
                // This is 1/2 places where auto-normalization is disabled
                fCoordTransform.reset(*args.fMatrix, fAtlas->asTextureProxyRef().get(), false);
                fTextureSampler.reset(fAtlas->asTextureProxyRef(), params);
            } else {
                // In this instance we know the params are:
                //   clampY, bilerp
                // and the proxy is:
                //   exact fit, power of two in both dimensions
                // Only the x-tileMode is unknown. However, given all the other knowns we know
                // that GrMakeCachedBitmapProxy is sufficient (i.e., it won't need to be
                // extracted to a subset or mipmapped).
                sk_sp<GrTextureProxy> proxy = GrMakeCachedBitmapProxy(
                                                                args.fContext->resourceProvider(),
                                                                bitmap);
                if (!proxy) {
                    SkDebugf("Gradient won't draw. Could not create texture.");
                    return;
                }
                // This is 2/2 places where auto-normalization is disabled
                fCoordTransform.reset(*args.fMatrix, proxy.get(), false);
                fTextureSampler.reset(std::move(proxy), params);
                fYCoord = SK_ScalarHalf;
            }

            this->addTextureSampler(&fTextureSampler);

            break;
    }

    this->addCoordTransform(&fCoordTransform);
}

GrGradientEffect::~GrGradientEffect() {
    if (this->useAtlas()) {
        fAtlas->unlockRow(fRow);
    }
}

bool GrGradientEffect::onIsEqual(const GrFragmentProcessor& processor) const {
    const GrGradientEffect& ge = processor.cast<GrGradientEffect>();

    if (this->fColorType != ge.getColorType()) {
        return false;
    }
    SkASSERT(this->useAtlas() == ge.useAtlas());
    if (kTexture_ColorType == fColorType) {
        if (fYCoord != ge.getYCoord()) {
            return false;
        }
    } else {
        if (kSingleHardStop_ColorType == fColorType) {
            if (!SkScalarNearlyEqual(ge.fPositions[1], fPositions[1])) {
                return false;
            }
        }
        if (this->getPremulType() != ge.getPremulType() ||
            this->fColors.count() != ge.fColors.count() ||
            this->fColors4f.count() != ge.fColors4f.count()) {
            return false;
        }

        for (int i = 0; i < this->fColors.count(); i++) {
            if (*this->getColors(i) != *ge.getColors(i)) {
                return false;
            }
        }
        for (int i = 0; i < this->fColors4f.count(); i++) {
            if (*this->getColors4f(i) != *ge.getColors4f(i)) {
                return false;
            }
        }
    }
    return GrColorSpaceXform::Equals(this->fColorSpaceXform.get(), ge.fColorSpaceXform.get());
}

#if GR_TEST_UTILS
GrGradientEffect::RandomGradientParams::RandomGradientParams(SkRandom* random) {
    // Set color count to min of 2 so that we don't trigger the const color optimization and make
    // a non-gradient processor.
    fColorCount = random->nextRangeU(2, kMaxRandomGradientColors);
    fUseColors4f = random->nextBool();

    // if one color, omit stops, otherwise randomly decide whether or not to
    if (fColorCount == 1 || (fColorCount >= 2 && random->nextBool())) {
        fStops = nullptr;
    } else {
        fStops = fStopStorage;
    }

    // if using SkColor4f, attach a random (possibly null) color space (with linear gamma)
    if (fUseColors4f) {
        fColorSpace = GrTest::TestColorSpace(random);
        if (fColorSpace) {
            SkASSERT(SkColorSpace_Base::Type::kXYZ == as_CSB(fColorSpace)->type());
            fColorSpace = static_cast<SkColorSpace_XYZ*>(fColorSpace.get())->makeLinearGamma();
        }
    }

    SkScalar stop = 0.f;
    for (int i = 0; i < fColorCount; ++i) {
        if (fUseColors4f) {
            fColors4f[i].fR = random->nextUScalar1();
            fColors4f[i].fG = random->nextUScalar1();
            fColors4f[i].fB = random->nextUScalar1();
            fColors4f[i].fA = random->nextUScalar1();
        } else {
            fColors[i] = random->nextU();
        }
        if (fStops) {
            fStops[i] = stop;
            stop = i < fColorCount - 1 ? stop + random->nextUScalar1() * (1.f - stop) : 1.f;
        }
    }
    fTileMode = static_cast<SkShader::TileMode>(random->nextULessThan(SkShader::kTileModeCount));
}
#endif

#endif
