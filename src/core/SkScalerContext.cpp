/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPaint.h"
#include "src/core/SkScalerContext.h"

#include "include/core/SkDrawable.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkStrokeRec.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkAutoMalloc.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkBlitter_A8.h"
#include "src/core/SkDescriptor.h"
#include "src/core/SkDrawBase.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkMaskGamma.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkRasterClip.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkStroke.h"
#include "src/core/SkSurfacePriv.h"
#include "src/core/SkTextFormatParams.h"
#include "src/core/SkWriteBuffer.h"
#include "src/utils/SkMatrix22.h"
#include <new>

///////////////////////////////////////////////////////////////////////////////

namespace {
static inline const constexpr bool kSkShowTextBlitCoverage = false;
static inline const constexpr bool kSkScalerContextDumpRec = false;
}

SkScalerContextRec SkScalerContext::PreprocessRec(const SkTypeface& typeface,
                                                  const SkScalerContextEffects& effects,
                                                  const SkDescriptor& desc) {
    SkScalerContextRec rec =
            *static_cast<const SkScalerContextRec*>(desc.findEntry(kRec_SkDescriptorTag, nullptr));

    // Allow the typeface to adjust the rec.
    typeface.onFilterRec(&rec);

    if (effects.fMaskFilter) {
        // Pre-blend is not currently applied to filtered text.
        // The primary filter is blur, for which contrast makes no sense,
        // and for which the destination guess error is more visible.
        // Also, all existing users of blur have calibrated for linear.
        rec.ignorePreBlend();
    }

    SkColor lumColor = rec.getLuminanceColor();

    if (rec.fMaskFormat == SkMask::kA8_Format) {
        U8CPU lum = SkComputeLuminance(SkColorGetR(lumColor),
                                       SkColorGetG(lumColor),
                                       SkColorGetB(lumColor));
        lumColor = SkColorSetRGB(lum, lum, lum);
    }

    // TODO: remove CanonicalColor when we to fix up Chrome layout tests.
    rec.setLuminanceColor(lumColor);

    return rec;
}

SkScalerContext::SkScalerContext(sk_sp<SkTypeface> typeface, const SkScalerContextEffects& effects,
                                 const SkDescriptor* desc)
    : fRec(PreprocessRec(*typeface, effects, *desc))
    , fTypeface(std::move(typeface))
    , fPathEffect(sk_ref_sp(effects.fPathEffect))
    , fMaskFilter(sk_ref_sp(effects.fMaskFilter))
      // Initialize based on our settings. Subclasses can also force this.
    , fGenerateImageFromPath(fRec.fFrameWidth >= 0 || fPathEffect != nullptr)

    , fPreBlend(fMaskFilter ? SkMaskGamma::PreBlend() : SkScalerContext::GetMaskPreBlend(fRec))
{
    if constexpr (kSkScalerContextDumpRec) {
        SkDebugf("SkScalerContext checksum %x count %d length %d\n",
                 desc->getChecksum(), desc->getCount(), desc->getLength());
        SkDebugf("%s", fRec.dump().c_str());
        SkDebugf("  effects %p\n", desc->findEntry(kEffects_SkDescriptorTag, nullptr));
    }
}

SkScalerContext::~SkScalerContext() {}

/**
 * In order to call cachedDeviceLuminance, cachedPaintLuminance, or
 * cachedMaskGamma the caller must hold the mask_gamma_cache_mutex and continue
 * to hold it until the returned pointer is refed or forgotten.
 */
static SkMutex& mask_gamma_cache_mutex() {
    static SkMutex& mutex = *(new SkMutex);
    return mutex;
}

static SkMaskGamma* gLinearMaskGamma = nullptr;
static SkMaskGamma* gMaskGamma = nullptr;
static SkScalar gContrast = SK_ScalarMin;
static SkScalar gPaintGamma = SK_ScalarMin;
static SkScalar gDeviceGamma = SK_ScalarMin;

/**
 * The caller must hold the mask_gamma_cache_mutex() and continue to hold it until
 * the returned SkMaskGamma pointer is refed or forgotten.
 */
static const SkMaskGamma& cached_mask_gamma(SkScalar contrast, SkScalar paintGamma,
                                            SkScalar deviceGamma) {
    mask_gamma_cache_mutex().assertHeld();
    if (0 == contrast && SK_Scalar1 == paintGamma && SK_Scalar1 == deviceGamma) {
        if (nullptr == gLinearMaskGamma) {
            gLinearMaskGamma = new SkMaskGamma;
        }
        return *gLinearMaskGamma;
    }
    if (gContrast != contrast || gPaintGamma != paintGamma || gDeviceGamma != deviceGamma) {
        SkSafeUnref(gMaskGamma);
        gMaskGamma = new SkMaskGamma(contrast, paintGamma, deviceGamma);
        gContrast = contrast;
        gPaintGamma = paintGamma;
        gDeviceGamma = deviceGamma;
    }
    return *gMaskGamma;
}

/**
 * Expands fDeviceGamma, fPaintGamma, fContrast, and fLumBits into a mask pre-blend.
 */
SkMaskGamma::PreBlend SkScalerContext::GetMaskPreBlend(const SkScalerContextRec& rec) {
    SkAutoMutexExclusive ama(mask_gamma_cache_mutex());

    const SkMaskGamma& maskGamma = cached_mask_gamma(rec.getContrast(),
                                                     rec.getPaintGamma(),
                                                     rec.getDeviceGamma());

    // TODO: remove CanonicalColor when we to fix up Chrome layout tests.
    return maskGamma.preBlend(rec.getLuminanceColor());
}

size_t SkScalerContext::GetGammaLUTSize(SkScalar contrast, SkScalar paintGamma,
                                        SkScalar deviceGamma, int* width, int* height) {
    SkAutoMutexExclusive ama(mask_gamma_cache_mutex());
    const SkMaskGamma& maskGamma = cached_mask_gamma(contrast,
                                                     paintGamma,
                                                     deviceGamma);

    maskGamma.getGammaTableDimensions(width, height);
    size_t size = (*width)*(*height)*sizeof(uint8_t);

    return size;
}

bool SkScalerContext::GetGammaLUTData(SkScalar contrast, SkScalar paintGamma, SkScalar deviceGamma,
                                      uint8_t* data) {
    SkAutoMutexExclusive ama(mask_gamma_cache_mutex());
    const SkMaskGamma& maskGamma = cached_mask_gamma(contrast,
                                                     paintGamma,
                                                     deviceGamma);
    const uint8_t* gammaTables = maskGamma.getGammaTables();
    if (!gammaTables) {
        return false;
    }

    int width, height;
    maskGamma.getGammaTableDimensions(&width, &height);
    size_t size = width*height * sizeof(uint8_t);
    memcpy(data, gammaTables, size);
    return true;
}

SkGlyph SkScalerContext::makeGlyph(SkPackedGlyphID packedID, SkArenaAlloc* alloc) {
    return internalMakeGlyph(packedID, fRec.fMaskFormat, alloc);
}

/** Return the closest D for the given S. Returns std::numeric_limits<D>::max() for NaN. */
template <typename D, typename S> static constexpr D sk_saturate_cast(S s) {
    static_assert(std::is_integral_v<D>);
    s = s < std::numeric_limits<D>::max() ? s : std::numeric_limits<D>::max();
    s = s > std::numeric_limits<D>::min() ? s : std::numeric_limits<D>::min();
    return (D)s;
}
void SkScalerContext::SaturateGlyphBounds(SkGlyph* glyph, SkRect&& r) {
    r.roundOut(&r);
    glyph->fLeft    = sk_saturate_cast<int16_t>(r.fLeft);
    glyph->fTop     = sk_saturate_cast<int16_t>(r.fTop);
    glyph->fWidth   = sk_saturate_cast<uint16_t>(r.width());
    glyph->fHeight  = sk_saturate_cast<uint16_t>(r.height());
}
void SkScalerContext::SaturateGlyphBounds(SkGlyph* glyph, SkIRect const & r) {
    glyph->fLeft    = sk_saturate_cast<int16_t>(r.fLeft);
    glyph->fTop     = sk_saturate_cast<int16_t>(r.fTop);
    glyph->fWidth   = sk_saturate_cast<uint16_t>(r.width64());
    glyph->fHeight  = sk_saturate_cast<uint16_t>(r.height64());
}

void SkScalerContext::GenerateMetricsFromPath(
    SkGlyph* glyph, const SkPath& devPath, SkMask::Format format,
    const bool verticalLCD, const bool a8FromLCD, const bool hairline)
{
    // Only BW, A8, and LCD16 can be produced from paths.
    if (glyph->fMaskFormat != SkMask::kBW_Format &&
        glyph->fMaskFormat != SkMask::kA8_Format &&
        glyph->fMaskFormat != SkMask::kLCD16_Format)
    {
        glyph->fMaskFormat = SkMask::kA8_Format;
    }

    SkRect bounds = devPath.getBounds();
    if (!bounds.isEmpty()) {
        const bool fromLCD = (glyph->fMaskFormat == SkMask::kLCD16_Format) ||
                             (glyph->fMaskFormat == SkMask::kA8_Format && a8FromLCD);

        const bool needExtraWidth  = (fromLCD && !verticalLCD) || hairline;
        const bool needExtraHeight = (fromLCD &&  verticalLCD) || hairline;
        if (needExtraWidth) {
            bounds.roundOut(&bounds);
            bounds.outset(1, 0);
        }
        if (needExtraHeight) {
            bounds.roundOut(&bounds);
            bounds.outset(0, 1);
        }
    }
    SaturateGlyphBounds(glyph, std::move(bounds));
}

SkGlyph SkScalerContext::internalMakeGlyph(SkPackedGlyphID packedID, SkMask::Format format, SkArenaAlloc* alloc) {
    auto zeroBounds = [](SkGlyph& glyph) {
        glyph.fLeft     = 0;
        glyph.fTop      = 0;
        glyph.fWidth    = 0;
        glyph.fHeight   = 0;
    };

    SkGlyph glyph{packedID};
    glyph.fMaskFormat = format; // subclass may return a different value
    GlyphMetrics mx = this->generateMetrics(glyph, alloc);
    SkASSERT(!mx.neverRequestPath || !mx.computeFromPath);

    glyph.fAdvanceX = mx.advance.fX;
    glyph.fAdvanceY = mx.advance.fY;
    glyph.fMaskFormat = mx.maskFormat;
    glyph.fScalerContextBits = mx.extraBits;

    if (mx.computeFromPath || (fGenerateImageFromPath && !mx.neverRequestPath)) {
        SkDEBUGCODE(glyph.fAdvancesBoundsFormatAndInitialPathDone = true;)
        this->internalGetPath(glyph, alloc);
        const SkPath* devPath = glyph.path();
        if (devPath) {
            const bool doVert = SkToBool(fRec.fFlags & SkScalerContext::kLCD_Vertical_Flag);
            const bool a8LCD = SkToBool(fRec.fFlags & SkScalerContext::kGenA8FromLCD_Flag);
            const bool hairline = glyph.pathIsHairline();
            GenerateMetricsFromPath(&glyph, *devPath, format, doVert, a8LCD, hairline);
        }
    } else {
        SaturateGlyphBounds(&glyph, std::move(mx.bounds));
        if (mx.neverRequestPath) {
            glyph.setPath(alloc, nullptr, false);
        }
    }
    SkDEBUGCODE(glyph.fAdvancesBoundsFormatAndInitialPathDone = true;)

    // if either dimension is empty, zap the image bounds of the glyph
    if (0 == glyph.fWidth || 0 == glyph.fHeight) {
        zeroBounds(glyph);
        return glyph;
    }

    if (fMaskFilter) {
        // only want the bounds from the filter
        SkMask src(nullptr, glyph.iRect(), glyph.rowBytes(), glyph.maskFormat());
        SkMaskBuilder dst;
        SkMatrix matrix;

        fRec.getMatrixFrom2x2(&matrix);

        if (as_MFB(fMaskFilter)->filterMask(&dst, src, matrix, nullptr)) {
            if (dst.fBounds.isEmpty()) {
                zeroBounds(glyph);
                return glyph;
            }
            SkASSERT(dst.fImage == nullptr);
            SaturateGlyphBounds(&glyph, dst.fBounds);
            glyph.fMaskFormat = dst.fFormat;
        }
    }
    return glyph;
}

static void applyLUTToA8Mask(const SkMask& mask, const uint8_t* lut) {
    uint8_t* SK_RESTRICT dst = (uint8_t*)mask.fImage;
    unsigned rowBytes = mask.fRowBytes;

    for (int y = mask.fBounds.height() - 1; y >= 0; --y) {
        for (int x = mask.fBounds.width() - 1; x >= 0; --x) {
            dst[x] = lut[dst[x]];
        }
        dst += rowBytes;
    }
}

static void pack4xHToMask(const SkPixmap& src, SkMaskBuilder& dst,
                          const SkMaskGamma::PreBlend& maskPreBlend,
                          const bool doBGR, const bool doVert) {
#define SAMPLES_PER_PIXEL 4
#define LCD_PER_PIXEL 3
    SkASSERT(kAlpha_8_SkColorType == src.colorType());

    const bool toA8 = SkMask::kA8_Format == dst.fFormat;
    SkASSERT(SkMask::kLCD16_Format == dst.fFormat || toA8);

    // doVert in this function means swap x and y when writing to dst.
    if (doVert) {
        SkASSERT(src.width() == (dst.fBounds.height() - 2) * 4);
        SkASSERT(src.height() == dst.fBounds.width());
    } else {
        SkASSERT(src.width() == (dst.fBounds.width() - 2) * 4);
        SkASSERT(src.height() == dst.fBounds.height());
    }

    const int sample_width = src.width();
    const int height = src.height();

    uint8_t* dstImage = dst.image();
    size_t dstRB = dst.fRowBytes;
    // An N tap FIR is defined by
    // out[n] = coeff[0]*x[n] + coeff[1]*x[n-1] + ... + coeff[N]*x[n-N]
    // or
    // out[n] = sum(i, 0, N, coeff[i]*x[n-i])

    // The strategy is to use one FIR (different coefficients) for each of r, g, and b.
    // This means using every 4th FIR output value of each FIR and discarding the rest.
    // The FIRs are aligned, and the coefficients reach 5 samples to each side of their 'center'.
    // (For r and b this is technically incorrect, but the coeffs outside round to zero anyway.)

    // These are in some fixed point repesentation.
    // Adding up to more than one simulates ink spread.
    // For implementation reasons, these should never add up to more than two.

    // Coefficients determined by a gausian where 5 samples = 3 std deviations (0x110 'contrast').
    // Calculated using tools/generate_fir_coeff.py
    // With this one almost no fringing is ever seen, but it is imperceptibly blurry.
    // The lcd smoothed text is almost imperceptibly different from gray,
    // but is still sharper on small stems and small rounded corners than gray.
    // This also seems to be about as wide as one can get and only have a three pixel kernel.
    // TODO: calculate these at runtime so parameters can be adjusted (esp contrast).
    static const unsigned int coefficients[LCD_PER_PIXEL][SAMPLES_PER_PIXEL*3] = {
        //The red subpixel is centered inside the first sample (at 1/6 pixel), and is shifted.
        { 0x03, 0x0b, 0x1c, 0x33,  0x40, 0x39, 0x24, 0x10,  0x05, 0x01, 0x00, 0x00, },
        //The green subpixel is centered between two samples (at 1/2 pixel), so is symetric
        { 0x00, 0x02, 0x08, 0x16,  0x2b, 0x3d, 0x3d, 0x2b,  0x16, 0x08, 0x02, 0x00, },
        //The blue subpixel is centered inside the last sample (at 5/6 pixel), and is shifted.
        { 0x00, 0x00, 0x01, 0x05,  0x10, 0x24, 0x39, 0x40,  0x33, 0x1c, 0x0b, 0x03, },
    };

    size_t dstPB = toA8 ? sizeof(uint8_t) : sizeof(uint16_t);
    for (int y = 0; y < height; ++y) {
        uint8_t* dstP;
        size_t dstPDelta;
        if (doVert) {
            dstP = SkTAddOffset<uint8_t>(dstImage, y * dstPB);
            dstPDelta = dstRB;
        } else {
            dstP = SkTAddOffset<uint8_t>(dstImage, y * dstRB);
            dstPDelta = dstPB;
        }

        const uint8_t* srcP = src.addr8(0, y);

        // TODO: this fir filter implementation is straight forward, but slow.
        // It should be possible to make it much faster.
        for (int sample_x = -4; sample_x < sample_width + 4; sample_x += 4) {
            int fir[LCD_PER_PIXEL] = { 0 };
            for (int sample_index = std::max(0, sample_x - 4), coeff_index = sample_index - (sample_x - 4)
                ; sample_index < std::min(sample_x + 8, sample_width)
                ; ++sample_index, ++coeff_index)
            {
                int sample_value = srcP[sample_index];
                for (int subpxl_index = 0; subpxl_index < LCD_PER_PIXEL; ++subpxl_index) {
                    fir[subpxl_index] += coefficients[subpxl_index][coeff_index] * sample_value;
                }
            }
            for (int subpxl_index = 0; subpxl_index < LCD_PER_PIXEL; ++subpxl_index) {
                fir[subpxl_index] /= 0x100;
                fir[subpxl_index] = std::min(fir[subpxl_index], 255);
            }

            U8CPU r, g, b;
            if (doBGR) {
                r = fir[2];
                g = fir[1];
                b = fir[0];
            } else {
                r = fir[0];
                g = fir[1];
                b = fir[2];
            }
            if constexpr (kSkShowTextBlitCoverage) {
                r = std::max(r, 10u);
                g = std::max(g, 10u);
                b = std::max(b, 10u);
            }
            if (toA8) {
                U8CPU a = (r + g + b) / 3;
                if (maskPreBlend.isApplicable()) {
                    a = maskPreBlend.fG[a];
                }
                *dstP = a;
            } else {
                if (maskPreBlend.isApplicable()) {
                    r = maskPreBlend.fR[r];
                    g = maskPreBlend.fG[g];
                    b = maskPreBlend.fB[b];
                }
                *(uint16_t*)dstP = SkPack888ToRGB16(r, g, b);
            }
            dstP = SkTAddOffset<uint8_t>(dstP, dstPDelta);
        }
    }
}

static inline int convert_8_to_1(unsigned byte) {
    SkASSERT(byte <= 0xFF);
    return byte >> 7;
}

static uint8_t pack_8_to_1(const uint8_t alpha[8]) {
    unsigned bits = 0;
    for (int i = 0; i < 8; ++i) {
        bits <<= 1;
        bits |= convert_8_to_1(alpha[i]);
    }
    return SkToU8(bits);
}

static void packA8ToA1(SkMaskBuilder& dstMask, const uint8_t* src, size_t srcRB) {
    const int height = dstMask.fBounds.height();
    const int width = dstMask.fBounds.width();
    const int octs = width >> 3;
    const int leftOverBits = width & 7;

    uint8_t* dst = dstMask.image();
    const int dstPad = dstMask.fRowBytes - SkAlign8(width)/8;
    SkASSERT(dstPad >= 0);

    SkASSERT(width >= 0);
    SkASSERT(srcRB >= (size_t)width);
    const size_t srcPad = srcRB - width;

    for (int y = 0; y < height; ++y) {
        for (int i = 0; i < octs; ++i) {
            *dst++ = pack_8_to_1(src);
            src += 8;
        }
        if (leftOverBits > 0) {
            unsigned bits = 0;
            int shift = 7;
            for (int i = 0; i < leftOverBits; ++i, --shift) {
                bits |= convert_8_to_1(*src++) << shift;
            }
            *dst++ = bits;
        }
        src += srcPad;
        dst += dstPad;
    }
}

void SkScalerContext::GenerateImageFromPath(
    SkMaskBuilder& dstMask, const SkPath& path, const SkMaskGamma::PreBlend& maskPreBlend,
    const bool doBGR, const bool verticalLCD, const bool a8FromLCD, const bool hairline)
{
    SkASSERT(dstMask.fFormat == SkMask::kBW_Format ||
             dstMask.fFormat == SkMask::kA8_Format ||
             dstMask.fFormat == SkMask::kLCD16_Format);

    SkPaint paint;
    SkPath strokePath;
    const SkPath* pathToUse = &path;

    int srcW = dstMask.fBounds.width();
    int srcH = dstMask.fBounds.height();
    int dstW = srcW;
    int dstH = srcH;

    SkMatrix matrix;
    matrix.setTranslate(-SkIntToScalar(dstMask.fBounds.fLeft),
                        -SkIntToScalar(dstMask.fBounds.fTop));

    paint.setStroke(hairline);
    paint.setAntiAlias(SkMask::kBW_Format != dstMask.fFormat);

    const bool fromLCD = (dstMask.fFormat == SkMask::kLCD16_Format) ||
                         (dstMask.fFormat == SkMask::kA8_Format && a8FromLCD);
    const bool intermediateDst = fromLCD || dstMask.fFormat == SkMask::kBW_Format;
    if (fromLCD) {
        if (verticalLCD) {
            dstW = 4*dstH - 8;
            dstH = srcW;
            matrix.setAll(0, 4, -SkIntToScalar(dstMask.fBounds.fTop + 1) * 4,
                          1, 0, -SkIntToScalar(dstMask.fBounds.fLeft),
                          0, 0, 1);
        } else {
            dstW = 4*dstW - 8;
            matrix.setAll(4, 0, -SkIntToScalar(dstMask.fBounds.fLeft + 1) * 4,
                          0, 1, -SkIntToScalar(dstMask.fBounds.fTop),
                          0, 0, 1);
        }

        // LCD hairline doesn't line up with the pixels, so do it the expensive way.
        SkStrokeRec rec(SkStrokeRec::kFill_InitStyle);
        if (hairline) {
            rec.setStrokeStyle(1.0f, false);
            rec.setStrokeParams(SkPaint::kButt_Cap, SkPaint::kRound_Join, 0.0f);
        }
        if (rec.needToApply() && rec.applyToPath(&strokePath, path)) {
            pathToUse = &strokePath;
            paint.setStyle(SkPaint::kFill_Style);
        }
    }

    SkRasterClip clip;
    clip.setRect(SkIRect::MakeWH(dstW, dstH));

    const SkImageInfo info = SkImageInfo::MakeA8(dstW, dstH);
    SkAutoPixmapStorage dst;

    if (intermediateDst) {
        if (!dst.tryAlloc(info)) {
            // can't allocate offscreen, so empty the mask and return
            sk_bzero(dstMask.image(), dstMask.computeImageSize());
            return;
        }
    } else {
        dst.reset(info, dstMask.image(), dstMask.fRowBytes);
    }
    sk_bzero(dst.writable_addr(), dst.computeByteSize());

    SkDrawBase  draw;
    draw.fBlitterChooser = SkA8Blitter_Choose;
    draw.fDst            = dst;
    draw.fRC             = &clip;
    draw.fCTM            = &matrix;
    draw.drawPath(*pathToUse, paint);

    switch (dstMask.fFormat) {
        case SkMask::kBW_Format:
            packA8ToA1(dstMask, dst.addr8(0, 0), dst.rowBytes());
            break;
        case SkMask::kA8_Format:
            if (fromLCD) {
                pack4xHToMask(dst, dstMask, maskPreBlend, doBGR, verticalLCD);
            } else if (maskPreBlend.isApplicable()) {
                applyLUTToA8Mask(dstMask, maskPreBlend.fG);
            }
            break;
        case SkMask::kLCD16_Format:
            pack4xHToMask(dst, dstMask, maskPreBlend, doBGR, verticalLCD);
            break;
        default:
            break;
    }
}

void SkScalerContext::getImage(const SkGlyph& origGlyph) {
    SkASSERT(origGlyph.fAdvancesBoundsFormatAndInitialPathDone);

    const SkGlyph* unfilteredGlyph = &origGlyph;
    // in case we need to call generateImage on a mask-format that is different
    // (i.e. larger) than what our caller allocated by looking at origGlyph.
    SkAutoMalloc tmpGlyphImageStorage;
    SkGlyph tmpGlyph;
    SkSTArenaAlloc<sizeof(SkGlyph::PathData)> tmpGlyphPathDataStorage;
    if (fMaskFilter) {
        // need the original bounds, sans our maskfilter
        sk_sp<SkMaskFilter> mf = std::move(fMaskFilter);
        tmpGlyph = this->makeGlyph(origGlyph.getPackedID(), &tmpGlyphPathDataStorage);
        fMaskFilter = std::move(mf);

        // Use the origGlyph storage for the temporary unfiltered mask if it will fit.
        if (tmpGlyph.fMaskFormat == origGlyph.fMaskFormat &&
            tmpGlyph.imageSize() <= origGlyph.imageSize())
        {
            tmpGlyph.fImage = origGlyph.fImage;
        } else {
            tmpGlyphImageStorage.reset(tmpGlyph.imageSize());
            tmpGlyph.fImage = tmpGlyphImageStorage.get();
        }
        unfilteredGlyph = &tmpGlyph;
    }

    if (!fGenerateImageFromPath) {
        generateImage(*unfilteredGlyph, unfilteredGlyph->fImage);
    } else {
        SkASSERT(origGlyph.setPathHasBeenCalled());
        const SkPath* devPath = origGlyph.path();

        if (!devPath) {
            generateImage(*unfilteredGlyph, unfilteredGlyph->fImage);
        } else {
            SkMaskBuilder mask(static_cast<uint8_t*>(unfilteredGlyph->fImage),
                               unfilteredGlyph->iRect(), unfilteredGlyph->rowBytes(),
                               unfilteredGlyph->maskFormat());
            SkASSERT(SkMask::kARGB32_Format != origGlyph.fMaskFormat);
            SkASSERT(SkMask::kARGB32_Format != mask.fFormat);
            const bool doBGR = SkToBool(fRec.fFlags & SkScalerContext::kLCD_BGROrder_Flag);
            const bool doVert = SkToBool(fRec.fFlags & SkScalerContext::kLCD_Vertical_Flag);
            const bool a8LCD = SkToBool(fRec.fFlags & SkScalerContext::kGenA8FromLCD_Flag);
            const bool hairline = origGlyph.pathIsHairline();
            GenerateImageFromPath(mask, *devPath, fPreBlend, doBGR, doVert, a8LCD, hairline);
        }
    }

    if (fMaskFilter) {
        // k3D_Format should not be mask filtered.
        SkASSERT(SkMask::k3D_Format != unfilteredGlyph->fMaskFormat);

        SkMaskBuilder srcMask;
        SkAutoMaskFreeImage srcMaskOwnedImage(nullptr);
        SkMatrix m;
        fRec.getMatrixFrom2x2(&m);

        if (as_MFB(fMaskFilter)->filterMask(&srcMask, unfilteredGlyph->mask(), m, nullptr)) {
            // Filter succeeded; srcMask.fImage was allocated.
            srcMaskOwnedImage.reset(srcMask.image());
        } else if (unfilteredGlyph->fImage == tmpGlyphImageStorage.get()) {
            // Filter did nothing; unfiltered mask is independent of origGlyph.fImage.
            srcMask = SkMaskBuilder(static_cast<uint8_t*>(unfilteredGlyph->fImage),
                                    unfilteredGlyph->iRect(), unfilteredGlyph->rowBytes(),
                                    unfilteredGlyph->maskFormat());
        } else if (origGlyph.iRect() == unfilteredGlyph->iRect()) {
            // Filter did nothing; the unfiltered mask is in origGlyph.fImage and matches.
            return;
        } else {
            // Filter did nothing; the unfiltered mask is in origGlyph.fImage and conflicts.
            srcMask = SkMaskBuilder(static_cast<uint8_t*>(unfilteredGlyph->fImage),
                                    unfilteredGlyph->iRect(), unfilteredGlyph->rowBytes(),
                                    unfilteredGlyph->maskFormat());
            size_t imageSize = unfilteredGlyph->imageSize();
            tmpGlyphImageStorage.reset(imageSize);
            srcMask.image() = static_cast<uint8_t*>(tmpGlyphImageStorage.get());
            memcpy(srcMask.image(), unfilteredGlyph->fImage, imageSize);
        }

        SkASSERT_RELEASE(srcMask.fFormat == origGlyph.fMaskFormat);
        SkMaskBuilder dstMask = SkMaskBuilder(static_cast<uint8_t*>(origGlyph.fImage),
                                              origGlyph.iRect(), origGlyph.rowBytes(),
                                              origGlyph.maskFormat());
        SkIRect origBounds = dstMask.fBounds;

        // Find the intersection of src and dst while updating the fImages.
        if (srcMask.fBounds.fTop < dstMask.fBounds.fTop) {
            int32_t topDiff = dstMask.fBounds.fTop - srcMask.fBounds.fTop;
            srcMask.image() += srcMask.fRowBytes * topDiff;
            srcMask.bounds().fTop = dstMask.fBounds.fTop;
        }
        if (dstMask.fBounds.fTop < srcMask.fBounds.fTop) {
            int32_t topDiff = srcMask.fBounds.fTop - dstMask.fBounds.fTop;
            dstMask.image() += dstMask.fRowBytes * topDiff;
            dstMask.bounds().fTop = srcMask.fBounds.fTop;
        }

        if (srcMask.fBounds.fLeft < dstMask.fBounds.fLeft) {
            int32_t leftDiff = dstMask.fBounds.fLeft - srcMask.fBounds.fLeft;
            srcMask.image() += leftDiff;
            srcMask.bounds().fLeft = dstMask.fBounds.fLeft;
        }
        if (dstMask.fBounds.fLeft < srcMask.fBounds.fLeft) {
            int32_t leftDiff = srcMask.fBounds.fLeft - dstMask.fBounds.fLeft;
            dstMask.image() += leftDiff;
            dstMask.bounds().fLeft = srcMask.fBounds.fLeft;
        }

        if (srcMask.fBounds.fBottom < dstMask.fBounds.fBottom) {
            dstMask.bounds().fBottom = srcMask.fBounds.fBottom;
        }
        if (dstMask.fBounds.fBottom < srcMask.fBounds.fBottom) {
            srcMask.bounds().fBottom = dstMask.fBounds.fBottom;
        }

        if (srcMask.fBounds.fRight < dstMask.fBounds.fRight) {
            dstMask.bounds().fRight = srcMask.fBounds.fRight;
        }
        if (dstMask.fBounds.fRight < srcMask.fBounds.fRight) {
            srcMask.bounds().fRight = dstMask.fBounds.fRight;
        }

        SkASSERT(srcMask.fBounds == dstMask.fBounds);
        int width = srcMask.fBounds.width();
        int height = srcMask.fBounds.height();
        int dstRB = dstMask.fRowBytes;
        int srcRB = srcMask.fRowBytes;

        const uint8_t* src = srcMask.fImage;
        uint8_t* dst = dstMask.image();

        if (SkMask::k3D_Format == srcMask.fFormat) {
            // we have to copy 3 times as much
            height *= 3;
        }

        // If not filling the full original glyph, clear it out first.
        if (dstMask.fBounds != origBounds) {
            sk_bzero(origGlyph.fImage, origGlyph.fHeight * origGlyph.rowBytes());
        }

        while (--height >= 0) {
            memcpy(dst, src, width);
            src += srcRB;
            dst += dstRB;
        }
    }
}

void SkScalerContext::getPath(SkGlyph& glyph, SkArenaAlloc* alloc) {
    this->internalGetPath(glyph, alloc);
}

sk_sp<SkDrawable> SkScalerContext::getDrawable(SkGlyph& glyph) {
    return this->generateDrawable(glyph);
}
//TODO: make pure virtual
sk_sp<SkDrawable> SkScalerContext::generateDrawable(const SkGlyph&) {
    return nullptr;
}

void SkScalerContext::getFontMetrics(SkFontMetrics* fm) {
    SkASSERT(fm);
    this->generateFontMetrics(fm);
}

///////////////////////////////////////////////////////////////////////////////

void SkScalerContext::internalGetPath(SkGlyph& glyph, SkArenaAlloc* alloc) {
    SkASSERT(glyph.fAdvancesBoundsFormatAndInitialPathDone);

    if (glyph.setPathHasBeenCalled()) {
        return;
    }

    SkPath path;
    SkPath devPath;
    bool hairline = false;

    SkPackedGlyphID glyphID = glyph.getPackedID();
    if (!generatePath(glyph, &path)) {
        glyph.setPath(alloc, (SkPath*)nullptr, hairline);
        return;
    }

    if (fRec.fFlags & SkScalerContext::kSubpixelPositioning_Flag) {
        SkFixed dx = glyphID.getSubXFixed();
        SkFixed dy = glyphID.getSubYFixed();
        if (dx | dy) {
            path.offset(SkFixedToScalar(dx), SkFixedToScalar(dy));
        }
    }

    if (fRec.fFrameWidth < 0 && fPathEffect == nullptr) {
        devPath.swap(path);
    } else {
        // need the path in user-space, with only the point-size applied
        // so that our stroking and effects will operate the same way they
        // would if the user had extracted the path themself, and then
        // called drawPath
        SkPath localPath;
        SkMatrix matrix;
        SkMatrix inverse;

        fRec.getMatrixFrom2x2(&matrix);
        if (!matrix.invert(&inverse)) {
            glyph.setPath(alloc, &devPath, hairline);
        }
        path.transform(inverse, &localPath);
        // now localPath is only affected by the paint settings, and not the canvas matrix

        SkStrokeRec rec(SkStrokeRec::kFill_InitStyle);

        if (fRec.fFrameWidth >= 0) {
            rec.setStrokeStyle(fRec.fFrameWidth,
                               SkToBool(fRec.fFlags & kFrameAndFill_Flag));
            // glyphs are always closed contours, so cap type is ignored,
            // so we just pass something.
            rec.setStrokeParams((SkPaint::Cap)fRec.fStrokeCap,
                                (SkPaint::Join)fRec.fStrokeJoin,
                                fRec.fMiterLimit);
        }

        if (fPathEffect) {
            SkPath effectPath;
            if (fPathEffect->filterPath(&effectPath, localPath, &rec, nullptr, matrix)) {
                localPath.swap(effectPath);
            }
        }

        if (rec.needToApply()) {
            SkPath strokePath;
            if (rec.applyToPath(&strokePath, localPath)) {
                localPath.swap(strokePath);
            }
        }

        // The path effect may have modified 'rec', so wait to here to check hairline status.
        if (rec.isHairlineStyle()) {
            hairline = true;
        }

        localPath.transform(matrix, &devPath);
    }
    glyph.setPath(alloc, &devPath, hairline);
}


void SkScalerContextRec::getMatrixFrom2x2(SkMatrix* dst) const {
    dst->setAll(fPost2x2[0][0], fPost2x2[0][1], 0,
                fPost2x2[1][0], fPost2x2[1][1], 0,
                0,              0,              1);
}

void SkScalerContextRec::getLocalMatrix(SkMatrix* m) const {
    *m = SkFontPriv::MakeTextMatrix(fTextSize, fPreScaleX, fPreSkewX);
}

void SkScalerContextRec::getSingleMatrix(SkMatrix* m) const {
    this->getLocalMatrix(m);

    //  now concat the device matrix
    SkMatrix    deviceMatrix;
    this->getMatrixFrom2x2(&deviceMatrix);
    m->postConcat(deviceMatrix);
}

bool SkScalerContextRec::computeMatrices(PreMatrixScale preMatrixScale, SkVector* s, SkMatrix* sA,
                                         SkMatrix* GsA, SkMatrix* G_inv, SkMatrix* A_out)
{
    // A is the 'total' matrix.
    SkMatrix A;
    this->getSingleMatrix(&A);

    // The caller may find the 'total' matrix useful when dealing directly with EM sizes.
    if (A_out) {
        *A_out = A;
    }

    // GA is the matrix A with rotation removed.
    SkMatrix GA;
    bool skewedOrFlipped = A.getSkewX() || A.getSkewY() || A.getScaleX() < 0 || A.getScaleY() < 0;
    if (skewedOrFlipped) {
        // QR by Givens rotations. G is Q^T and GA is R. G is rotational (no reflections).
        // h is where A maps the horizontal baseline.
        SkPoint h = SkPoint::Make(SK_Scalar1, 0);
        A.mapPoints(&h, 1);

        // G is the Givens Matrix for A (rotational matrix where GA[0][1] == 0).
        SkMatrix G;
        SkComputeGivensRotation(h, &G);

        GA = G;
        GA.preConcat(A);

        // The 'remainingRotation' is G inverse, which is fairly simple since G is 2x2 rotational.
        if (G_inv) {
            G_inv->setAll(
                G.get(SkMatrix::kMScaleX), -G.get(SkMatrix::kMSkewX), G.get(SkMatrix::kMTransX),
                -G.get(SkMatrix::kMSkewY), G.get(SkMatrix::kMScaleY), G.get(SkMatrix::kMTransY),
                G.get(SkMatrix::kMPersp0), G.get(SkMatrix::kMPersp1), G.get(SkMatrix::kMPersp2));
        }
    } else {
        GA = A;
        if (G_inv) {
            G_inv->reset();
        }
    }

    // If the 'total' matrix is singular, set the 'scale' to something finite and zero the matrices.
    // All underlying ports have issues with zero text size, so use the matricies to zero.
    // If one of the scale factors is less than 1/256 then an EM filling square will
    // never affect any pixels.
    // If there are any nonfinite numbers in the matrix, bail out and set the matrices to zero.
    if (SkScalarAbs(GA.get(SkMatrix::kMScaleX)) <= SK_ScalarNearlyZero ||
        SkScalarAbs(GA.get(SkMatrix::kMScaleY)) <= SK_ScalarNearlyZero ||
        !GA.isFinite())
    {
        s->fX = SK_Scalar1;
        s->fY = SK_Scalar1;
        sA->setScale(0, 0);
        if (GsA) {
            GsA->setScale(0, 0);
        }
        if (G_inv) {
            G_inv->reset();
        }
        return false;
    }

    // At this point, given GA, create s.
    switch (preMatrixScale) {
        case PreMatrixScale::kFull:
            s->fX = SkScalarAbs(GA.get(SkMatrix::kMScaleX));
            s->fY = SkScalarAbs(GA.get(SkMatrix::kMScaleY));
            break;
        case PreMatrixScale::kVertical: {
            SkScalar yScale = SkScalarAbs(GA.get(SkMatrix::kMScaleY));
            s->fX = yScale;
            s->fY = yScale;
            break;
        }
        case PreMatrixScale::kVerticalInteger: {
            SkScalar realYScale = SkScalarAbs(GA.get(SkMatrix::kMScaleY));
            SkScalar intYScale = SkScalarRoundToScalar(realYScale);
            if (intYScale == 0) {
                intYScale = SK_Scalar1;
            }
            s->fX = intYScale;
            s->fY = intYScale;
            break;
        }
    }

    // The 'remaining' matrix sA is the total matrix A without the scale.
    if (!skewedOrFlipped && (
            (PreMatrixScale::kFull == preMatrixScale) ||
            (PreMatrixScale::kVertical == preMatrixScale && A.getScaleX() == A.getScaleY())))
    {
        // If GA == A and kFull, sA is identity.
        // If GA == A and kVertical and A.scaleX == A.scaleY, sA is identity.
        sA->reset();
    } else if (!skewedOrFlipped && PreMatrixScale::kVertical == preMatrixScale) {
        // If GA == A and kVertical, sA.scaleY is SK_Scalar1.
        sA->reset();
        sA->setScaleX(A.getScaleX() / s->fY);
    } else {
        // TODO: like kVertical, kVerticalInteger with int scales.
        *sA = A;
        sA->preScale(SkScalarInvert(s->fX), SkScalarInvert(s->fY));
    }

    // The 'remainingWithoutRotation' matrix GsA is the non-rotational part of A without the scale.
    if (GsA) {
        *GsA = GA;
         // G is rotational so reorders with the scale.
        GsA->preScale(SkScalarInvert(s->fX), SkScalarInvert(s->fY));
    }

    return true;
}

SkAxisAlignment SkScalerContext::computeAxisAlignmentForHText() const {
    return fRec.computeAxisAlignmentForHText();
}

SkAxisAlignment SkScalerContextRec::computeAxisAlignmentForHText() const {
    // Why fPost2x2 can be used here.
    // getSingleMatrix multiplies in getLocalMatrix, which consists of
    // * fTextSize (a scale, which has no effect)
    // * fPreScaleX (a scale in x, which has no effect)
    // * fPreSkewX (has no effect, but would on vertical text alignment).
    // In other words, making the text bigger, stretching it along the
    // horizontal axis, or fake italicizing it does not move the baseline.
    if (!SkToBool(fFlags & SkScalerContext::kBaselineSnap_Flag)) {
        return SkAxisAlignment::kNone;
    }

    if (0 == fPost2x2[1][0]) {
        // The x axis is mapped onto the x axis.
        return SkAxisAlignment::kX;
    }
    if (0 == fPost2x2[0][0]) {
        // The x axis is mapped onto the y axis.
        return SkAxisAlignment::kY;
    }
    return SkAxisAlignment::kNone;
}

void SkScalerContextRec::setLuminanceColor(SkColor c) {
    fLumBits = SkMaskGamma::CanonicalColor(
            SkColorSetRGB(SkColorGetR(c), SkColorGetG(c), SkColorGetB(c)));
}

/*
 *  Return the scalar with only limited fractional precision. Used to consolidate matrices
 *  that vary only slightly when we create our key into the font cache, since the font scaler
 *  typically returns the same looking resuts for tiny changes in the matrix.
 */
static SkScalar sk_relax(SkScalar x) {
    SkScalar n = SkScalarRoundToScalar(x * 1024);
    return n / 1024.0f;
}

static SkMask::Format compute_mask_format(const SkFont& font) {
    switch (font.getEdging()) {
        case SkFont::Edging::kAlias:
            return SkMask::kBW_Format;
        case SkFont::Edging::kAntiAlias:
            return SkMask::kA8_Format;
        case SkFont::Edging::kSubpixelAntiAlias:
            return SkMask::kLCD16_Format;
    }
    SkASSERT(false);
    return SkMask::kA8_Format;
}

// Beyond this size, LCD doesn't appreciably improve quality, but it always
// cost more RAM and draws slower, so we set a cap.
#ifndef SK_MAX_SIZE_FOR_LCDTEXT
    #define SK_MAX_SIZE_FOR_LCDTEXT    48
#endif

const SkScalar gMaxSize2ForLCDText = SK_MAX_SIZE_FOR_LCDTEXT * SK_MAX_SIZE_FOR_LCDTEXT;

static bool too_big_for_lcd(const SkScalerContextRec& rec, bool checkPost2x2) {
    if (checkPost2x2) {
        SkScalar area = rec.fPost2x2[0][0] * rec.fPost2x2[1][1] -
                        rec.fPost2x2[1][0] * rec.fPost2x2[0][1];
        area *= rec.fTextSize * rec.fTextSize;
        return area > gMaxSize2ForLCDText;
    } else {
        return rec.fTextSize > SK_MAX_SIZE_FOR_LCDTEXT;
    }
}

// The only reason this is not file static is because it needs the context of SkScalerContext to
// access SkPaint::computeLuminanceColor.
void SkScalerContext::MakeRecAndEffects(const SkFont& font, const SkPaint& paint,
                                        const SkSurfaceProps& surfaceProps,
                                        SkScalerContextFlags scalerContextFlags,
                                        const SkMatrix& deviceMatrix,
                                        SkScalerContextRec* rec,
                                        SkScalerContextEffects* effects) {
    SkASSERT(!deviceMatrix.hasPerspective());

    sk_bzero(rec, sizeof(SkScalerContextRec));

    SkTypeface* typeface = SkFontPriv::GetTypefaceOrDefault(font);

    rec->fTypefaceID = typeface->uniqueID();
    rec->fTextSize = font.getSize();
    rec->fPreScaleX = font.getScaleX();
    rec->fPreSkewX  = font.getSkewX();

    bool checkPost2x2 = false;

    const SkMatrix::TypeMask mask = deviceMatrix.getType();
    if (mask & SkMatrix::kScale_Mask) {
        rec->fPost2x2[0][0] = sk_relax(deviceMatrix.getScaleX());
        rec->fPost2x2[1][1] = sk_relax(deviceMatrix.getScaleY());
        checkPost2x2 = true;
    } else {
        rec->fPost2x2[0][0] = rec->fPost2x2[1][1] = SK_Scalar1;
    }
    if (mask & SkMatrix::kAffine_Mask) {
        rec->fPost2x2[0][1] = sk_relax(deviceMatrix.getSkewX());
        rec->fPost2x2[1][0] = sk_relax(deviceMatrix.getSkewY());
        checkPost2x2 = true;
    } else {
        rec->fPost2x2[0][1] = rec->fPost2x2[1][0] = 0;
    }

    SkPaint::Style  style = paint.getStyle();
    SkScalar        strokeWidth = paint.getStrokeWidth();

    unsigned flags = 0;

    if (font.isEmbolden()) {
#ifdef SK_USE_FREETYPE_EMBOLDEN
        flags |= SkScalerContext::kEmbolden_Flag;
#else
        SkScalar fakeBoldScale = SkScalarInterpFunc(font.getSize(),
                                                    kStdFakeBoldInterpKeys,
                                                    kStdFakeBoldInterpValues,
                                                    kStdFakeBoldInterpLength);
        SkScalar extra = font.getSize() * fakeBoldScale;

        if (style == SkPaint::kFill_Style) {
            style = SkPaint::kStrokeAndFill_Style;
            strokeWidth = extra;    // ignore paint's strokeWidth if it was "fill"
        } else {
            strokeWidth += extra;
        }
#endif
    }

    if (style != SkPaint::kFill_Style && strokeWidth >= 0) {
        rec->fFrameWidth = strokeWidth;
        rec->fMiterLimit = paint.getStrokeMiter();
        rec->fStrokeJoin = SkToU8(paint.getStrokeJoin());
        rec->fStrokeCap = SkToU8(paint.getStrokeCap());

        if (style == SkPaint::kStrokeAndFill_Style) {
            flags |= SkScalerContext::kFrameAndFill_Flag;
        }
    } else {
        rec->fFrameWidth = -1;
        rec->fMiterLimit = 0;
        rec->fStrokeJoin = 0;
        rec->fStrokeCap = 0;
    }

    rec->fMaskFormat = compute_mask_format(font);

    if (SkMask::kLCD16_Format == rec->fMaskFormat) {
        if (too_big_for_lcd(*rec, checkPost2x2)) {
            rec->fMaskFormat = SkMask::kA8_Format;
            flags |= SkScalerContext::kGenA8FromLCD_Flag;
        } else {
            SkPixelGeometry geometry = surfaceProps.pixelGeometry();

            switch (geometry) {
                case kUnknown_SkPixelGeometry:
                    // eeek, can't support LCD
                    rec->fMaskFormat = SkMask::kA8_Format;
                    flags |= SkScalerContext::kGenA8FromLCD_Flag;
                    break;
                case kRGB_H_SkPixelGeometry:
                    // our default, do nothing.
                    break;
                case kBGR_H_SkPixelGeometry:
                    flags |= SkScalerContext::kLCD_BGROrder_Flag;
                    break;
                case kRGB_V_SkPixelGeometry:
                    flags |= SkScalerContext::kLCD_Vertical_Flag;
                    break;
                case kBGR_V_SkPixelGeometry:
                    flags |= SkScalerContext::kLCD_Vertical_Flag;
                    flags |= SkScalerContext::kLCD_BGROrder_Flag;
                    break;
            }
        }
    }

    if (font.isEmbeddedBitmaps()) {
        flags |= SkScalerContext::kEmbeddedBitmapText_Flag;
    }
    if (font.isSubpixel()) {
        flags |= SkScalerContext::kSubpixelPositioning_Flag;
    }
    if (font.isForceAutoHinting()) {
        flags |= SkScalerContext::kForceAutohinting_Flag;
    }
    if (font.isLinearMetrics()) {
        flags |= SkScalerContext::kLinearMetrics_Flag;
    }
    if (font.isBaselineSnap()) {
        flags |= SkScalerContext::kBaselineSnap_Flag;
    }
    if (typeface->glyphMaskNeedsCurrentColor()) {
        flags |= SkScalerContext::kNeedsForegroundColor_Flag;
        rec->fForegroundColor = paint.getColor();
    }
    rec->fFlags = SkToU16(flags);

    // these modify fFlags, so do them after assigning fFlags
    rec->setHinting(font.getHinting());
    rec->setLuminanceColor(SkPaintPriv::ComputeLuminanceColor(paint));

    // For now always set the paint gamma equal to the device gamma.
    // The math in SkMaskGamma can handle them being different,
    // but it requires superluminous masks when
    // Ex : deviceGamma(x) < paintGamma(x) and x is sufficiently large.
    rec->setDeviceGamma(SK_GAMMA_EXPONENT);
    rec->setPaintGamma(SK_GAMMA_EXPONENT);
    rec->setContrast(SK_GAMMA_CONTRAST);

    if (!SkToBool(scalerContextFlags & SkScalerContextFlags::kFakeGamma)) {
        rec->ignoreGamma();
    }
    if (!SkToBool(scalerContextFlags & SkScalerContextFlags::kBoostContrast)) {
        rec->setContrast(0);
    }

    new (effects) SkScalerContextEffects{paint};
}

SkDescriptor* SkScalerContext::CreateDescriptorAndEffectsUsingPaint(
    const SkFont& font, const SkPaint& paint, const SkSurfaceProps& surfaceProps,
    SkScalerContextFlags scalerContextFlags, const SkMatrix& deviceMatrix, SkAutoDescriptor* ad,
    SkScalerContextEffects* effects)
{
    SkScalerContextRec rec;
    MakeRecAndEffects(font, paint, surfaceProps, scalerContextFlags, deviceMatrix, &rec, effects);
    return AutoDescriptorGivenRecAndEffects(rec, *effects, ad);
}

static size_t calculate_size_and_flatten(const SkScalerContextRec& rec,
                                         const SkScalerContextEffects& effects,
                                         SkBinaryWriteBuffer* effectBuffer) {
    size_t descSize = sizeof(rec);
    int entryCount = 1;

    if (effects.fPathEffect || effects.fMaskFilter) {
        if (effects.fPathEffect) { effectBuffer->writeFlattenable(effects.fPathEffect); }
        if (effects.fMaskFilter) { effectBuffer->writeFlattenable(effects.fMaskFilter); }
        entryCount += 1;
        descSize += effectBuffer->bytesWritten();
    }

    descSize += SkDescriptor::ComputeOverhead(entryCount);
    return descSize;
}

static void generate_descriptor(const SkScalerContextRec& rec,
                                const SkBinaryWriteBuffer& effectBuffer,
                                SkDescriptor* desc) {
    desc->addEntry(kRec_SkDescriptorTag, sizeof(rec), &rec);

    if (effectBuffer.bytesWritten() > 0) {
        effectBuffer.writeToMemory(desc->addEntry(kEffects_SkDescriptorTag,
                                                  effectBuffer.bytesWritten(),
                                                  nullptr));
    }

    desc->computeChecksum();
}

SkDescriptor* SkScalerContext::AutoDescriptorGivenRecAndEffects(
    const SkScalerContextRec& rec,
    const SkScalerContextEffects& effects,
    SkAutoDescriptor* ad)
{
    SkBinaryWriteBuffer buf({});

    ad->reset(calculate_size_and_flatten(rec, effects, &buf));
    generate_descriptor(rec, buf, ad->getDesc());

    return ad->getDesc();
}

std::unique_ptr<SkDescriptor> SkScalerContext::DescriptorGivenRecAndEffects(
    const SkScalerContextRec& rec,
    const SkScalerContextEffects& effects)
{
    SkBinaryWriteBuffer buf({});

    auto desc = SkDescriptor::Alloc(calculate_size_and_flatten(rec, effects, &buf));
    generate_descriptor(rec, buf, desc.get());

    return desc;
}

void SkScalerContext::DescriptorBufferGiveRec(const SkScalerContextRec& rec, void* buffer) {
    generate_descriptor(rec, SkBinaryWriteBuffer({}), (SkDescriptor*)buffer);
}

bool SkScalerContext::CheckBufferSizeForRec(const SkScalerContextRec& rec,
                                            const SkScalerContextEffects& effects,
                                            size_t size) {
    SkBinaryWriteBuffer buf({});
    return size >= calculate_size_and_flatten(rec, effects, &buf);
}

std::unique_ptr<SkScalerContext> SkScalerContext::MakeEmpty(
        sk_sp<SkTypeface> typeface, const SkScalerContextEffects& effects,
        const SkDescriptor* desc) {
    class SkScalerContext_Empty : public SkScalerContext {
    public:
        SkScalerContext_Empty(sk_sp<SkTypeface> typeface, const SkScalerContextEffects& effects,
                              const SkDescriptor* desc)
                : SkScalerContext(std::move(typeface), effects, desc) {}

    protected:
        GlyphMetrics generateMetrics(const SkGlyph& glyph, SkArenaAlloc*) override {
            return {glyph.maskFormat()};
        }
        void generateImage(const SkGlyph&, void*) override {}
        bool generatePath(const SkGlyph& glyph, SkPath* path) override {
            path->reset();
            return false;
        }
        void generateFontMetrics(SkFontMetrics* metrics) override {
            if (metrics) {
                sk_bzero(metrics, sizeof(*metrics));
            }
        }
    };

    return std::make_unique<SkScalerContext_Empty>(std::move(typeface), effects, desc);
}




