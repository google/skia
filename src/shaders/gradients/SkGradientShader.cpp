/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algorithm>
#include "Sk4fLinearGradient.h"
#include "SkColorSpace_XYZ.h"
#include "SkColorSpaceXformer.h"
#include "SkFloatBits.h"
#include "SkGradientBitmapCache.h"
#include "SkGradientShaderPriv.h"
#include "SkHalf.h"
#include "SkLinearGradient.h"
#include "SkMallocPixelRef.h"
#include "SkRadialGradient.h"
#include "SkReadBuffer.h"
#include "SkSweepGradient.h"
#include "SkTwoPointConicalGradient.h"
#include "SkWriteBuffer.h"
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
    , fColorsAreOpaque(true)
{
    fPtsToUnit.getType();  // Precache so reads are threadsafe.
    SkASSERT(desc.fCount > 1);

    fGradFlags = static_cast<uint8_t>(desc.fGradFlags);

    SkASSERT((unsigned)desc.fTileMode < SkShader::kTileModeCount);
    fTileMode = desc.fTileMode;

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

    size_t storageSize = fColorCount * (sizeof(SkColor4f) + (desc.fPos ? sizeof(SkScalar) : 0));
    fOrigColors4f      = reinterpret_cast<SkColor4f*>(fStorage.reset(storageSize));
    fOrigPos           = desc.fPos ? reinterpret_cast<SkScalar*>(fOrigColors4f + fColorCount)
                                   : nullptr;

    // Now copy over the colors, adding the dummies as needed
    SkColor4f* origColors = fOrigColors4f;
    if (dummyFirst) {
        *origColors++ = desc.fColors[0];
    }
    for (int i = 0; i < desc.fCount; ++i) {
        origColors[i] = desc.fColors[i];
        fColorsAreOpaque = fColorsAreOpaque && (desc.fColors[i].fA == 1);
    }
    if (dummyLast) {
        origColors += desc.fCount;
        *origColors = desc.fColors[desc.fCount - 1];
    }

    if (!desc.fColorSpace) {
        // This happens if we were constructed from SkColors, so our colors are really sRGB
        fColorSpace = SkColorSpace::MakeSRGBLinear();
    } else {
        // The color space refers to the float colors, so it must be linear gamma
        // TODO: GPU code no longer requires this (see GrGradientEffect). Remove this restriction?
        SkASSERT(desc.fColorSpace->gammaIsLinear());
        fColorSpace = desc.fColorSpace;
    }

    if (desc.fPos) {
        SkScalar prev = 0;
        SkScalar* origPosPtr = fOrigPos;
        *origPosPtr++ = prev; // force the first pos to 0

        int startIndex = dummyFirst ? 0 : 1;
        int count = desc.fCount + dummyLast;

        bool uniformStops = true;
        const SkScalar uniformStep = desc.fPos[startIndex] - prev;
        for (int i = startIndex; i < count; i++) {
            // Pin the last value to 1.0, and make sure pos is monotonic.
            auto curr = (i == desc.fCount) ? 1 : SkScalarPin(desc.fPos[i], prev, 1);
            uniformStops &= SkScalarNearlyEqual(uniformStep, curr - prev);

            *origPosPtr++ = prev = curr;
        }

        // If the stops are uniform, treat them as implicit.
        if (uniformStops) {
            fOrigPos = nullptr;
        }
    }
}

SkGradientShaderBase::~SkGradientShaderBase() {}

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

bool SkGradientShaderBase::onAppendStages(const StageRec& rec) const {
    SkRasterPipeline* p = rec.fPipeline;
    SkArenaAlloc* alloc = rec.fAlloc;
    SkColorSpace* dstCS = rec.fDstCS;

    SkMatrix matrix;
    if (!this->computeTotalInverse(rec.fCTM, rec.fLocalM, &matrix)) {
        return false;
    }
    matrix.postConcat(fPtsToUnit);

    SkRasterPipeline_<256> postPipeline;

    p->append_seed_shader();
    p->append_matrix(alloc, matrix);
    this->appendGradientStages(alloc, p, &postPipeline);

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
                SkASSERT(t_l <= t_r);
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
    // TODO: use linear colors?
    for (int i = 0; i < n; ++i) {
        SkColor c = this->getLegacyColor(i);
        r += SkColorGetR(c);
        g += SkColorGetG(c);
        b += SkColorGetB(c);
    }
    *lum = SkColorSetRGB(rounded_divide(r, n), rounded_divide(g, n), rounded_divide(b, n));
    return true;
}

SkGradientShaderBase::AutoXformColors::AutoXformColors(const SkGradientShaderBase& grad,
                                                       SkColorSpaceXformer* xformer)
    : fColors(grad.fColorCount) {
    // TODO: stay in 4f to preserve precision?

    SkAutoSTMalloc<8, SkColor> origColors(grad.fColorCount);
    for (int i = 0; i < grad.fColorCount; ++i) {
        origColors[i] = grad.getLegacyColor(i);
    }

    xformer->apply(fColors.get(), origColors.get(), grad.fColorCount);
}

static constexpr int kGradientTextureSize = 256;

void SkGradientShaderBase::initLinearBitmap(SkBitmap* bitmap, GradientBitmapType bitmapType) const {
    const bool interpInPremul = SkToBool(fGradFlags &
                                         SkGradientShader::kInterpolateColorsInPremul_Flag);
    SkHalf* pixelsF16 = reinterpret_cast<SkHalf*>(bitmap->getPixels());
    uint32_t* pixels32 = reinterpret_cast<uint32_t*>(bitmap->getPixels());

    typedef std::function<void(const Sk4f&, int)> pixelWriteFn_t;

    pixelWriteFn_t writeF16Pixel = [&](const Sk4f& x, int index) {
        Sk4h c = SkFloatToHalf_finite_ftz(x);
        pixelsF16[4*index+0] = c[0];
        pixelsF16[4*index+1] = c[1];
        pixelsF16[4*index+2] = c[2];
        pixelsF16[4*index+3] = c[3];
    };
    pixelWriteFn_t writeS32Pixel = [&](const Sk4f& c, int index) {
        pixels32[index] = Sk4f_toS32(c);
    };
    pixelWriteFn_t writeL32Pixel = [&](const Sk4f& c, int index) {
        pixels32[index] = Sk4f_toL32(c);
    };

    pixelWriteFn_t writeSizedPixel =
        (bitmapType == GradientBitmapType::kHalfFloat) ? writeF16Pixel :
        (bitmapType == GradientBitmapType::kSRGB     ) ? writeS32Pixel : writeL32Pixel;
    pixelWriteFn_t writeUnpremulPixel = [&](const Sk4f& c, int index) {
        writeSizedPixel(c * Sk4f(c[3], c[3], c[3], 1.0f), index);
    };

    pixelWriteFn_t writePixel = interpInPremul ? writeSizedPixel : writeUnpremulPixel;

    // When not in legacy mode, we just want the original 4f colors - so we pass in
    // our own CS for identity/no transform.
    auto* cs = bitmapType != GradientBitmapType::kLegacy ? fColorSpace.get() : nullptr;

    int prevIndex = 0;
    for (int i = 1; i < fColorCount; i++) {
        // Historically, stops have been mapped to [0, 256], with 256 then nudged to the
        // next smaller value, then truncate for the texture index. This seems to produce
        // the best results for some common distributions, so we preserve the behavior.
        int nextIndex = SkTMin(this->getPos(i) * kGradientTextureSize,
                               SkIntToScalar(kGradientTextureSize - 1));

        if (nextIndex > prevIndex) {
            SkColor4f color0 = this->getXformedColor(i - 1, cs),
                      color1 = this->getXformedColor(i    , cs);
            Sk4f          c0 = Sk4f::Load(color0.vec()),
                          c1 = Sk4f::Load(color1.vec());

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
    SkASSERT(prevIndex == kGradientTextureSize - 1);
}

SkColor4f SkGradientShaderBase::getXformedColor(size_t i, SkColorSpace* dstCS) const {
    if (dstCS) {
        return to_colorspace(fOrigColors4f[i], fColorSpace.get(), dstCS);
    }

    // Legacy/srgb color.
    // We quantize upfront to ensure stable SkColor round-trips.
    auto rgb255 = sk_linear_to_srgb(Sk4f::Load(fOrigColors4f[i].vec()));
    auto rgb    = SkNx_cast<float>(rgb255) * (1/255.0f);
    return { rgb[0], rgb[1], rgb[2], fOrigColors4f[i].fA };
}

SK_DECLARE_STATIC_MUTEX(gGradientCacheMutex);
/*
 *  Because our caller might rebuild the same (logically the same) gradient
 *  over and over, we'd like to return exactly the same "bitmap" if possible,
 *  allowing the client to utilize a cache of our bitmap (e.g. with a GPU).
 *  To do that, we maintain a private cache of built-bitmaps, based on our
 *  colors and positions.
 */
void SkGradientShaderBase::getGradientTableBitmap(SkBitmap* bitmap,
                                                  GradientBitmapType bitmapType) const {
    // build our key: [numColors + colors[] + {positions[]} + flags + colorType ]
    static_assert(sizeof(SkColor4f) % sizeof(int32_t) == 0, "");
    const int colorsAsIntCount = fColorCount * sizeof(SkColor4f) / sizeof(int32_t);
    int count = 1 + colorsAsIntCount + 1 + 1;
    if (fColorCount > 2) {
        count += fColorCount - 1;
    }

    SkAutoSTMalloc<64, int32_t> storage(count);
    int32_t* buffer = storage.get();

    *buffer++ = fColorCount;
    memcpy(buffer, fOrigColors4f, fColorCount * sizeof(SkColor4f));
    buffer += colorsAsIntCount;
    if (fColorCount > 2) {
        for (int i = 1; i < fColorCount; i++) {
            *buffer++ = SkFloat2Bits(this->getPos(i));
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
        // For these cases we use the bitmap cache, but not the GradientShaderCache. So just
        // allocate and populate the bitmap's data directly.

        SkImageInfo info;
        switch (bitmapType) {
        case GradientBitmapType::kLegacy:
            info = SkImageInfo::Make(kGradientTextureSize, 1, kRGBA_8888_SkColorType,
                                     kPremul_SkAlphaType);
            break;
        case GradientBitmapType::kSRGB:
            info = SkImageInfo::Make(kGradientTextureSize, 1, kRGBA_8888_SkColorType,
                                     kPremul_SkAlphaType, SkColorSpace::MakeSRGB());
            break;
        case GradientBitmapType::kHalfFloat:
            info = SkImageInfo::Make(kGradientTextureSize, 1, kRGBA_F16_SkColorType,
                                     kPremul_SkAlphaType, SkColorSpace::MakeSRGBLinear());
            break;
        }

        bitmap->allocPixels(info);
        this->initLinearBitmap(bitmap, bitmapType);
        gCache->add(storage.get(), size, *bitmap);
    }
}

void SkGradientShaderBase::commonAsAGradient(GradientInfo* info) const {
    if (info) {
        if (info->fColorCount >= fColorCount) {
            if (info->fColors) {
                for (int i = 0; i < fColorCount; ++i) {
                    info->fColors[i] = this->getLegacyColor(i);
                }
            }
            if (info->fColorOffsets) {
                for (int i = 0; i < fColorCount; ++i) {
                    info->fColorOffsets[i] = this->getPos(i);
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
        str->appendHex(this->getLegacyColor(i), 8);
        if (i < fColorCount-1) {
            str->append(", ");
        }
    }

    if (fColorCount > 2) {
        str->append(" points: (");
        for (int i = 0; i < fColorCount; ++i) {
            str->appendScalar(this->getPos(i));
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

    SkGradientShaderBase::Descriptor desc;
    desc_init(&desc, opt.fColors, std::move(colorSpace), opt.fPos, opt.fCount, mode, flags,
              localMatrix);
    return SkTwoPointConicalGradient::Create(start, startRadius, end, endRadius, desc);
}

sk_sp<SkShader> SkGradientShader::MakeSweep(SkScalar cx, SkScalar cy,
                                            const SkColor colors[],
                                            const SkScalar pos[],
                                            int colorCount,
                                            SkShader::TileMode mode,
                                            SkScalar startAngle,
                                            SkScalar endAngle,
                                            uint32_t flags,
                                            const SkMatrix* localMatrix) {
    ColorConverter converter(colors, colorCount);
    return MakeSweep(cx, cy, converter.fColors4f.begin(), nullptr, pos, colorCount,
                     mode, startAngle, endAngle, flags, localMatrix);
}

sk_sp<SkShader> SkGradientShader::MakeSweep(SkScalar cx, SkScalar cy,
                                            const SkColor4f colors[],
                                            sk_sp<SkColorSpace> colorSpace,
                                            const SkScalar pos[],
                                            int colorCount,
                                            SkShader::TileMode mode,
                                            SkScalar startAngle,
                                            SkScalar endAngle,
                                            uint32_t flags,
                                            const SkMatrix* localMatrix) {
    if (!valid_grad(colors, pos, colorCount, mode)) {
        return nullptr;
    }
    if (1 == colorCount) {
        return SkShader::MakeColorShader(colors[0], std::move(colorSpace));
    }
    if (startAngle >= endAngle) {
        return nullptr;
    }
    if (localMatrix && !localMatrix->invert(nullptr)) {
        return nullptr;
    }

    if (startAngle <= 0 && endAngle >= 360) {
        // If the t-range includes [0,1], then we can always use clamping (presumably faster).
        mode = SkShader::kClamp_TileMode;
    }

    ColorStopOptimizer opt(colors, pos, colorCount, mode);

    SkGradientShaderBase::Descriptor desc;
    desc_init(&desc, opt.fColors, std::move(colorSpace), opt.fPos, opt.fCount, mode, flags,
              localMatrix);

    const SkScalar t0 = startAngle / 360,
                   t1 =   endAngle / 360;

    return sk_make_sp<SkSweepGradient>(SkPoint::Make(cx, cy), t0, t1, desc);
}

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkGradientShader)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLinearGradient)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkRadialGradient)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkSweepGradient)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkTwoPointConicalGradient)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END

///////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "GrColorSpaceXform.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrShaderCaps.h"
#include "GrTextureStripAtlas.h"
#include "gl/GrGLContext.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "SkGr.h"

void GrGradientEffect::GLSLProcessor::emitUniforms(GrGLSLUniformHandler* uniformHandler,
                                                   const GrGradientEffect& ge) {
    switch (ge.fStrategy) {
        case GrGradientEffect::InterpolationStrategy::kThreshold:
        case GrGradientEffect::InterpolationStrategy::kThresholdClamp0:
        case GrGradientEffect::InterpolationStrategy::kThresholdClamp1:
            fThresholdUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                       kFloat_GrSLType,
                                                       kHigh_GrSLPrecision,
                                                       "Threshold");
            // fall through
        case GrGradientEffect::InterpolationStrategy::kSingle:
            fIntervalsUni = uniformHandler->addUniformArray(kFragment_GrShaderFlag,
                                                            kHalf4_GrSLType,
                                                            "Intervals",
                                                            ge.fIntervals.count());
            break;
        case GrGradientEffect::InterpolationStrategy::kTexture:
            fFSYUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kHalf_GrSLType,
                                                 "GradientYCoordFS");
            break;
    }
}

void GrGradientEffect::GLSLProcessor::onSetData(const GrGLSLProgramDataManager& pdman,
                                                const GrFragmentProcessor& processor) {
    const GrGradientEffect& e = processor.cast<GrGradientEffect>();

    switch (e.fStrategy) {
        case GrGradientEffect::InterpolationStrategy::kThreshold:
        case GrGradientEffect::InterpolationStrategy::kThresholdClamp0:
        case GrGradientEffect::InterpolationStrategy::kThresholdClamp1:
            pdman.set1f(fThresholdUni, e.fThreshold);
            // fall through
        case GrGradientEffect::InterpolationStrategy::kSingle:
            pdman.set4fv(fIntervalsUni, e.fIntervals.count(),
                         reinterpret_cast<const float*>(e.fIntervals.begin()));
            break;
        case GrGradientEffect::InterpolationStrategy::kTexture:
            if (e.fYCoord != fCachedYCoord) {
                pdman.set1f(fFSYUni, e.fYCoord);
                fCachedYCoord = e.fYCoord;
            }
            break;
    }
}

void GrGradientEffect::onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const {
    b->add32(GLSLProcessor::GenBaseGradientKey(*this));
}

uint32_t GrGradientEffect::GLSLProcessor::GenBaseGradientKey(const GrProcessor& processor) {
    const GrGradientEffect& e = processor.cast<GrGradientEffect>();

    // Build a key using the following bit allocation:
                static constexpr uint32_t kStrategyBits = 3;
                static constexpr uint32_t kPremulBits   = 1;
    SkDEBUGCODE(static constexpr uint32_t kWrapModeBits = 2;)

    uint32_t key = static_cast<uint32_t>(e.fStrategy);
    SkASSERT(key < (1 << kStrategyBits));

    // This is already baked into the table for texture gradients,
    // and only changes behavior for analytical gradients.
    if (e.fStrategy != InterpolationStrategy::kTexture &&
        e.fPremulType == GrGradientEffect::kBeforeInterp_PremulType) {
        key |= 1 << kStrategyBits;
        SkASSERT(key < (1 << (kStrategyBits + kPremulBits)));
    }

    key |= static_cast<uint32_t>(e.fWrapMode) << (kStrategyBits + kPremulBits);
    SkASSERT(key < (1 << (kStrategyBits + kPremulBits + kWrapModeBits)));

    return key;
}

void GrGradientEffect::GLSLProcessor::emitAnalyticalColor(GrGLSLFPFragmentBuilder* fragBuilder,
                                                          GrGLSLUniformHandler* uniformHandler,
                                                          const GrShaderCaps* shaderCaps,
                                                          const GrGradientEffect& ge,
                                                          const char* t,
                                                          const char* outputColor,
                                                          const char* inputColor) {
    // First, apply tiling rules.
    switch (ge.fWrapMode) {
        case GrSamplerState::WrapMode::kClamp:
            switch (ge.fStrategy) {
                case GrGradientEffect::InterpolationStrategy::kThresholdClamp0:
                    // allow t > 1, in order to hit the clamp interval (1, inf)
                    fragBuilder->codeAppendf("half tiled_t = max(%s, 0.0);", t);
                    break;
                case GrGradientEffect::InterpolationStrategy::kThresholdClamp1:
                    // allow t < 0, in order to hit the clamp interval (-inf, 0)
                    fragBuilder->codeAppendf("half tiled_t = min(%s, 1.0);", t);
                    break;
                default:
                    // regular [0, 1] clamping
                    fragBuilder->codeAppendf("half tiled_t = clamp(%s, 0.0, 1.0);", t);
            }
            break;
        case GrSamplerState::WrapMode::kRepeat:
            fragBuilder->codeAppendf("half tiled_t = fract(%s);", t);
            break;
        case GrSamplerState::WrapMode::kMirrorRepeat:
            fragBuilder->codeAppendf("half t_1 = %s - 1.0;", t);
            fragBuilder->codeAppendf("half tiled_t = t_1 - 2.0 * floor(t_1 * 0.5) - 1.0;");
            if (shaderCaps->mustDoOpBetweenFloorAndAbs()) {
                // At this point the expected value of tiled_t should between -1 and 1, so this
                // clamp has no effect other than to break up the floor and abs calls and make sure
                // the compiler doesn't merge them back together.
                fragBuilder->codeAppendf("tiled_t = clamp(tiled_t, -1.0, 1.0);");
            }
            fragBuilder->codeAppendf("tiled_t = abs(tiled_t);");
            break;
    }

    // Calculate the color.
    const char* intervals = uniformHandler->getUniformCStr(fIntervalsUni);

    switch (ge.fStrategy) {
        case GrGradientEffect::InterpolationStrategy::kSingle:
            SkASSERT(ge.fIntervals.count() == 2);
            fragBuilder->codeAppendf(
                "half4 color_scale = %s[0],"
                "      color_bias  = %s[1];"
                , intervals, intervals
            );
            break;
        case GrGradientEffect::InterpolationStrategy::kThreshold:
        case GrGradientEffect::InterpolationStrategy::kThresholdClamp0:
        case GrGradientEffect::InterpolationStrategy::kThresholdClamp1:
        {
            SkASSERT(ge.fIntervals.count() == 4);
            const char* threshold = uniformHandler->getUniformCStr(fThresholdUni);
            fragBuilder->codeAppendf(
                "half4 color_scale, color_bias;"
                "if (tiled_t < %s) {"
                "    color_scale = %s[0];"
                "    color_bias  = %s[1];"
                "} else {"
                "    color_scale = %s[2];"
                "    color_bias  = %s[3];"
                "}"
                , threshold, intervals, intervals, intervals, intervals
            );
        }   break;
        default:
            SkASSERT(false);
            break;
    }

    fragBuilder->codeAppend("half4 colorTemp = tiled_t * color_scale + color_bias;");

    // We could skip this step if all colors are known to be opaque. Two considerations:
    // The gradient SkShader reporting opaque is more restrictive than necessary in the two
    // pt case. Make sure the key reflects this optimization (and note that it can use the
    // same shader as the kBeforeInterp case).
    if (ge.fPremulType == GrGradientEffect::kAfterInterp_PremulType) {
        fragBuilder->codeAppend("colorTemp.rgb *= colorTemp.a;");
    }

    // If the input colors were floats, or there was a color space xform, we may end up out of
    // range. The simplest solution is to always clamp our (premul) value here. We only need to
    // clamp RGB, but that causes hangs on the Tegra3 Nexus7. Clamping RGBA avoids the problem.
    fragBuilder->codeAppend("colorTemp = clamp(colorTemp, 0, colorTemp.a);");

    fragBuilder->codeAppendf("%s = %s * colorTemp;", outputColor, inputColor);
}

void GrGradientEffect::GLSLProcessor::emitColor(GrGLSLFPFragmentBuilder* fragBuilder,
                                                GrGLSLUniformHandler* uniformHandler,
                                                const GrShaderCaps* shaderCaps,
                                                const GrGradientEffect& ge,
                                                const char* gradientTValue,
                                                const char* outputColor,
                                                const char* inputColor,
                                                const TextureSamplers& texSamplers) {
    if (ge.fStrategy != InterpolationStrategy::kTexture) {
        this->emitAnalyticalColor(fragBuilder, uniformHandler, shaderCaps, ge, gradientTValue,
                                  outputColor, inputColor);
        return;
    }

    const char* fsyuni = uniformHandler->getUniformCStr(fFSYUni);

    fragBuilder->codeAppendf("half2 coord = half2(%s, %s);", gradientTValue, fsyuni);
    fragBuilder->codeAppendf("%s = ", outputColor);
    fragBuilder->appendTextureLookupAndModulate(inputColor, texSamplers[0], "coord",
                                                kFloat2_GrSLType);
    fragBuilder->codeAppend(";");
}

/////////////////////////////////////////////////////////////////////

inline GrFragmentProcessor::OptimizationFlags GrGradientEffect::OptFlags(bool isOpaque) {
    return isOpaque
                   ? kPreservesOpaqueInput_OptimizationFlag |
                             kCompatibleWithCoverageAsAlpha_OptimizationFlag
                   : kCompatibleWithCoverageAsAlpha_OptimizationFlag;
}

void GrGradientEffect::addInterval(const SkGradientShaderBase& shader, size_t idx0, size_t idx1,
                                   SkColorSpace* dstCS) {
    SkASSERT(idx0 <= idx1);
    const auto  c4f0 = shader.getXformedColor(idx0, dstCS),
                c4f1 = shader.getXformedColor(idx1, dstCS);
    const auto    c0 = (fPremulType == kBeforeInterp_PremulType)
                     ? c4f0.premul().to4f() :  Sk4f::Load(c4f0.vec()),
                  c1 = (fPremulType == kBeforeInterp_PremulType)
                     ? c4f1.premul().to4f() :  Sk4f::Load(c4f1.vec());
    const auto    t0 = shader.getPos(idx0),
                  t1 = shader.getPos(idx1),
                  dt = t1 - t0;
    SkASSERT(dt >= 0);
    // dt can be 0 for clamp intervals => in this case we want a scale == 0
    const auto scale = SkScalarNearlyZero(dt) ? 0 : (c1 - c0) / dt,
                bias = c0 - t0 * scale;

    // Intervals are stored as (scale, bias) tuples.
    SkASSERT(!(fIntervals.count() & 1));
    fIntervals.emplace_back(scale[0], scale[1], scale[2], scale[3]);
    fIntervals.emplace_back( bias[0],  bias[1],  bias[2],  bias[3]);
}

GrGradientEffect::GrGradientEffect(ClassID classID, const CreateArgs& args, bool isOpaque)
    : INHERITED(classID, OptFlags(isOpaque))
    , fWrapMode(args.fWrapMode)
    , fRow(-1)
    , fIsOpaque(args.fShader->isOpaque())
    , fStrategy(InterpolationStrategy::kTexture)
    , fThreshold(0) {

    const SkGradientShaderBase& shader(*args.fShader);

    fPremulType = (args.fShader->getGradFlags() & SkGradientShader::kInterpolateColorsInPremul_Flag)
                ? kBeforeInterp_PremulType : kAfterInterp_PremulType;

    // First, determine the interpolation strategy and params.
    switch (shader.fColorCount) {
        case 2:
            SkASSERT(!shader.fOrigPos);
            fStrategy = InterpolationStrategy::kSingle;
            this->addInterval(shader, 0, 1, args.fDstColorSpace);
            break;
        case 3:
            fThreshold = shader.getPos(1);

            if (shader.fOrigPos) {
                SkASSERT(SkScalarNearlyEqual(shader.fOrigPos[0], 0));
                SkASSERT(SkScalarNearlyEqual(shader.fOrigPos[2], 1));
                if (SkScalarNearlyEqual(shader.fOrigPos[1], 0)) {
                    // hard stop on the left edge.
                    if (fWrapMode == GrSamplerState::WrapMode::kClamp) {
                        fStrategy = InterpolationStrategy::kThresholdClamp1;
                        // Clamp interval (scale == 0, bias == colors[0]).
                        this->addInterval(shader, 0, 0, args.fDstColorSpace);
                    } else {
                        // We can ignore the hard stop when not clamping.
                        fStrategy = InterpolationStrategy::kSingle;
                    }
                    this->addInterval(shader, 1, 2, args.fDstColorSpace);
                    break;
                }

                if (SkScalarNearlyEqual(shader.fOrigPos[1], 1)) {
                    // hard stop on the right edge.
                    this->addInterval(shader, 0, 1, args.fDstColorSpace);
                    if (fWrapMode == GrSamplerState::WrapMode::kClamp) {
                        fStrategy = InterpolationStrategy::kThresholdClamp0;
                        // Clamp interval (scale == 0, bias == colors[2]).
                        this->addInterval(shader, 2, 2, args.fDstColorSpace);
                    } else {
                        // We can ignore the hard stop when not clamping.
                        fStrategy = InterpolationStrategy::kSingle;
                    }
                    break;
                }
            }

            // Two arbitrary interpolation intervals.
            fStrategy = InterpolationStrategy::kThreshold;
            this->addInterval(shader, 0, 1, args.fDstColorSpace);
            this->addInterval(shader, 1, 2, args.fDstColorSpace);
            break;
        case 4:
            if (shader.fOrigPos && SkScalarNearlyEqual(shader.fOrigPos[1], shader.fOrigPos[2])) {
                SkASSERT(SkScalarNearlyEqual(shader.fOrigPos[0], 0));
                SkASSERT(SkScalarNearlyEqual(shader.fOrigPos[3], 1));

                // Single hard stop => two arbitrary interpolation intervals.
                fStrategy = InterpolationStrategy::kThreshold;
                fThreshold = shader.getPos(1);
                this->addInterval(shader, 0, 1, args.fDstColorSpace);
                this->addInterval(shader, 2, 3, args.fDstColorSpace);
            }
            break;
        default:
            break;
    }

    // Now that we've locked down a strategy, adjust any dependent params.
    if (fStrategy != InterpolationStrategy::kTexture) {
        // Analytical cases.
        fCoordTransform.reset(*args.fMatrix);
    } else {
        SkGradientShaderBase::GradientBitmapType bitmapType =
            SkGradientShaderBase::GradientBitmapType::kLegacy;
        if (args.fDstColorSpace) {
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
        GrSamplerState samplerState(args.fWrapMode, GrSamplerState::Filter::kBilerp);

        fRow = fAtlas->lockRow(bitmap);
        if (-1 != fRow) {
            fYCoord = fAtlas->getYOffset(fRow)+SK_ScalarHalf*fAtlas->getNormalizedTexelHeight();
            // This is 1/2 places where auto-normalization is disabled
            fCoordTransform.reset(*args.fMatrix, fAtlas->asTextureProxyRef().get(), false);
            fTextureSampler.reset(fAtlas->asTextureProxyRef(), samplerState);
        } else {
            // In this instance we know the samplerState state is:
            //   clampY, bilerp
            // and the proxy is:
            //   exact fit, power of two in both dimensions
            // Only the x-tileMode is unknown. However, given all the other knowns we know
            // that GrMakeCachedBitmapProxy is sufficient (i.e., it won't need to be
            // extracted to a subset or mipmapped).
            sk_sp<GrTextureProxy> proxy = GrMakeCachedBitmapProxy(
                                                     args.fContext->contextPriv().proxyProvider(),
                                                     bitmap);
            if (!proxy) {
                SkDebugf("Gradient won't draw. Could not create texture.");
                return;
            }
            // This is 2/2 places where auto-normalization is disabled
            fCoordTransform.reset(*args.fMatrix, proxy.get(), false);
            fTextureSampler.reset(std::move(proxy), samplerState);
            fYCoord = SK_ScalarHalf;
        }

        this->addTextureSampler(&fTextureSampler);
    }

    this->addCoordTransform(&fCoordTransform);
}

GrGradientEffect::GrGradientEffect(const GrGradientEffect& that)
        : INHERITED(that.classID(), OptFlags(that.fIsOpaque))
        , fIntervals(that.fIntervals)
        , fWrapMode(that.fWrapMode)
        , fCoordTransform(that.fCoordTransform)
        , fTextureSampler(that.fTextureSampler)
        , fYCoord(that.fYCoord)
        , fAtlas(that.fAtlas)
        , fRow(that.fRow)
        , fIsOpaque(that.fIsOpaque)
        , fStrategy(that.fStrategy)
        , fThreshold(that.fThreshold)
        , fPremulType(that.fPremulType) {
    this->addCoordTransform(&fCoordTransform);
    if (fStrategy == InterpolationStrategy::kTexture) {
        this->addTextureSampler(&fTextureSampler);
    }
    if (this->useAtlas()) {
        fAtlas->lockRow(fRow);
    }
}

GrGradientEffect::~GrGradientEffect() {
    if (this->useAtlas()) {
        fAtlas->unlockRow(fRow);
    }
}

bool GrGradientEffect::onIsEqual(const GrFragmentProcessor& processor) const {
    const GrGradientEffect& ge = processor.cast<GrGradientEffect>();

    if (fWrapMode != ge.fWrapMode || fStrategy != ge.fStrategy) {
        return false;
    }

    SkASSERT(this->useAtlas() == ge.useAtlas());
    if (fStrategy == InterpolationStrategy::kTexture) {
        if (fYCoord != ge.fYCoord) {
            return false;
        }
    } else {
        if (fThreshold != ge.fThreshold ||
            fIntervals != ge.fIntervals ||
            fPremulType != ge.fPremulType) {
            return false;
        }
    }
    return true;
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
            fColorSpace = fColorSpace->makeLinearGamma();
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
