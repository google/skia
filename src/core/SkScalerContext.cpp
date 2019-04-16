/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPaint.h"
#include "SkScalerContext.h"

#include "SkAutoMalloc.h"
#include "SkAutoPixmapStorage.h"
#include "SkColorData.h"
#include "SkDescriptor.h"
#include "SkDraw.h"
#include "SkFontMetrics.h"
#include "SkFontPriv.h"
#include "SkGlyph.h"
#include "SkMakeUnique.h"
#include "SkMaskFilter.h"
#include "SkMaskGamma.h"
#include "SkMatrix22.h"
#include "SkPaintPriv.h"
#include "SkPathEffect.h"
#include "SkPathPriv.h"
#include "SkRasterClip.h"
#include "SkReadBuffer.h"
#include "SkRectPriv.h"
#include "SkStroke.h"
#include "SkStrokeRec.h"
#include "SkSurfacePriv.h"
#include "SkTextFormatParams.h"
#include "SkTo.h"
#include "SkWriteBuffer.h"
#include <new>

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
    #define DUMP_RECx
#endif

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
    , fGenerateImageFromPath(fRec.fFrameWidth > 0 || fPathEffect != nullptr)

    , fPreBlend(fMaskFilter ? SkMaskGamma::PreBlend() : SkScalerContext::GetMaskPreBlend(fRec))
{
#ifdef DUMP_REC
    SkDebugf("SkScalerContext checksum %x count %d length %d\n",
             desc->getChecksum(), desc->getCount(), desc->getLength());
    SkDebugf("%s", fRec.dump().c_str());
    SkDebugf("  effects %x\n", desc->findEntry(kEffects_SkDescriptorTag, nullptr));
#endif
}

SkScalerContext::~SkScalerContext() {}

/**
 * In order to call cachedDeviceLuminance, cachedPaintLuminance, or
 * cachedMaskGamma the caller must hold the gMaskGammaCacheMutex and continue
 * to hold it until the returned pointer is refed or forgotten.
 */
SK_DECLARE_STATIC_MUTEX(gMaskGammaCacheMutex);

static SkMaskGamma* gLinearMaskGamma = nullptr;
static SkMaskGamma* gMaskGamma = nullptr;
static SkScalar gContrast = SK_ScalarMin;
static SkScalar gPaintGamma = SK_ScalarMin;
static SkScalar gDeviceGamma = SK_ScalarMin;

/**
 * The caller must hold the gMaskGammaCacheMutex and continue to hold it until
 * the returned SkMaskGamma pointer is refed or forgotten.
 */
static const SkMaskGamma& cached_mask_gamma(SkScalar contrast, SkScalar paintGamma,
                                            SkScalar deviceGamma) {
    gMaskGammaCacheMutex.assertHeld();
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
    SkAutoMutexAcquire ama(gMaskGammaCacheMutex);

    const SkMaskGamma& maskGamma = cached_mask_gamma(rec.getContrast(),
                                                     rec.getPaintGamma(),
                                                     rec.getDeviceGamma());

    // TODO: remove CanonicalColor when we to fix up Chrome layout tests.
    return maskGamma.preBlend(rec.getLuminanceColor());
}

size_t SkScalerContext::GetGammaLUTSize(SkScalar contrast, SkScalar paintGamma,
                                        SkScalar deviceGamma, int* width, int* height) {
    SkAutoMutexAcquire ama(gMaskGammaCacheMutex);
    const SkMaskGamma& maskGamma = cached_mask_gamma(contrast,
                                                     paintGamma,
                                                     deviceGamma);

    maskGamma.getGammaTableDimensions(width, height);
    size_t size = (*width)*(*height)*sizeof(uint8_t);

    return size;
}

bool SkScalerContext::GetGammaLUTData(SkScalar contrast, SkScalar paintGamma, SkScalar deviceGamma,
                                      uint8_t* data) {
    SkAutoMutexAcquire ama(gMaskGammaCacheMutex);
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

void SkScalerContext::getAdvance(SkGlyph* glyph) {
    if (generateAdvance(glyph)) {
        glyph->fMaskFormat = MASK_FORMAT_JUST_ADVANCE;
    } else {
        this->getMetrics(glyph);
        SkASSERT(glyph->fMaskFormat != MASK_FORMAT_UNKNOWN);
    }
}

void SkScalerContext::getMetrics(SkGlyph* glyph) {
    bool generatingImageFromPath = fGenerateImageFromPath;
    if (!generatingImageFromPath) {
        generateMetrics(glyph);
        SkASSERT(glyph->fMaskFormat != MASK_FORMAT_UNKNOWN);
    } else {
        SkPath devPath;
        generatingImageFromPath = this->internalGetPath(glyph->getPackedID(), &devPath);
        if (!generatingImageFromPath) {
            generateMetrics(glyph);
            SkASSERT(glyph->fMaskFormat != MASK_FORMAT_UNKNOWN);
        } else {
            uint8_t originMaskFormat = glyph->fMaskFormat;
            if (!generateAdvance(glyph)) {
                generateMetrics(glyph);
            }

            if (originMaskFormat != MASK_FORMAT_UNKNOWN) {
                glyph->fMaskFormat = originMaskFormat;
            } else {
                glyph->fMaskFormat = fRec.fMaskFormat;
            }

            // If we are going to create the mask, then we cannot keep the color
            if (SkMask::kARGB32_Format == glyph->fMaskFormat) {
                glyph->fMaskFormat = SkMask::kA8_Format;
            }

            const SkIRect ir = devPath.getBounds().roundOut();
            if (ir.isEmpty() || !SkRectPriv::Is16Bit(ir)) {
                goto SK_ERROR;
            }
            glyph->fLeft    = ir.fLeft;
            glyph->fTop     = ir.fTop;
            glyph->fWidth   = SkToU16(ir.width());
            glyph->fHeight  = SkToU16(ir.height());

            if (glyph->fWidth > 0) {
                switch (glyph->fMaskFormat) {
                case SkMask::kLCD16_Format:
                    glyph->fWidth += 2;
                    glyph->fLeft -= 1;
                    break;
                default:
                    break;
                }
            }
        }
    }

    // if either dimension is empty, zap the image bounds of the glyph
    if (0 == glyph->fWidth || 0 == glyph->fHeight) {
        glyph->fWidth   = 0;
        glyph->fHeight  = 0;
        glyph->fTop     = 0;
        glyph->fLeft    = 0;
        glyph->fMaskFormat = 0;
        return;
    }

    if (fMaskFilter) {
        SkMask      src = glyph->mask(),
                    dst;
        SkMatrix    matrix;

        fRec.getMatrixFrom2x2(&matrix);

        src.fImage = nullptr;  // only want the bounds from the filter
        if (as_MFB(fMaskFilter)->filterMask(&dst, src, matrix, nullptr)) {
            if (dst.fBounds.isEmpty() || !SkRectPriv::Is16Bit(dst.fBounds)) {
                goto SK_ERROR;
            }
            SkASSERT(dst.fImage == nullptr);
            glyph->fLeft    = dst.fBounds.fLeft;
            glyph->fTop     = dst.fBounds.fTop;
            glyph->fWidth   = SkToU16(dst.fBounds.width());
            glyph->fHeight  = SkToU16(dst.fBounds.height());
            glyph->fMaskFormat = dst.fFormat;
        }
    }
    return;

SK_ERROR:
    // draw nothing 'cause we failed
    glyph->fLeft     = 0;
    glyph->fTop      = 0;
    glyph->fWidth    = 0;
    glyph->fHeight   = 0;
    // put a valid value here, in case it was earlier set to
    // MASK_FORMAT_JUST_ADVANCE
    glyph->fMaskFormat = fRec.fMaskFormat;
}

#define SK_SHOW_TEXT_BLIT_COVERAGE 0

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

template<bool APPLY_PREBLEND>
static void pack4xHToLCD16(const SkPixmap& src, const SkMask& dst,
                           const SkMaskGamma::PreBlend& maskPreBlend) {
#define SAMPLES_PER_PIXEL 4
#define LCD_PER_PIXEL 3
    SkASSERT(kAlpha_8_SkColorType == src.colorType());
    SkASSERT(SkMask::kLCD16_Format == dst.fFormat);

    const int sample_width = src.width();
    const int height = src.height();

    uint16_t* dstP = (uint16_t*)dst.fImage;
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
    // TODO: caculate these at runtime so parameters can be adjusted (esp contrast).
    static const unsigned int coefficients[LCD_PER_PIXEL][SAMPLES_PER_PIXEL*3] = {
        //The red subpixel is centered inside the first sample (at 1/6 pixel), and is shifted.
        { 0x03, 0x0b, 0x1c, 0x33,  0x40, 0x39, 0x24, 0x10,  0x05, 0x01, 0x00, 0x00, },
        //The green subpixel is centered between two samples (at 1/2 pixel), so is symetric
        { 0x00, 0x02, 0x08, 0x16,  0x2b, 0x3d, 0x3d, 0x2b,  0x16, 0x08, 0x02, 0x00, },
        //The blue subpixel is centered inside the last sample (at 5/6 pixel), and is shifted.
        { 0x00, 0x00, 0x01, 0x05,  0x10, 0x24, 0x39, 0x40,  0x33, 0x1c, 0x0b, 0x03, },
    };

    for (int y = 0; y < height; ++y) {
        const uint8_t* srcP = src.addr8(0, y);

        // TODO: this fir filter implementation is straight forward, but slow.
        // It should be possible to make it much faster.
        for (int sample_x = -4, pixel_x = 0; sample_x < sample_width + 4; sample_x += 4, ++pixel_x) {
            int fir[LCD_PER_PIXEL] = { 0 };
            for (int sample_index = SkMax32(0, sample_x - 4), coeff_index = sample_index - (sample_x - 4)
                ; sample_index < SkMin32(sample_x + 8, sample_width)
                ; ++sample_index, ++coeff_index)
            {
                int sample_value = srcP[sample_index];
                for (int subpxl_index = 0; subpxl_index < LCD_PER_PIXEL; ++subpxl_index) {
                    fir[subpxl_index] += coefficients[subpxl_index][coeff_index] * sample_value;
                }
            }
            for (int subpxl_index = 0; subpxl_index < LCD_PER_PIXEL; ++subpxl_index) {
                fir[subpxl_index] /= 0x100;
                fir[subpxl_index] = SkMin32(fir[subpxl_index], 255);
            }

            U8CPU r = sk_apply_lut_if<APPLY_PREBLEND>(fir[0], maskPreBlend.fR);
            U8CPU g = sk_apply_lut_if<APPLY_PREBLEND>(fir[1], maskPreBlend.fG);
            U8CPU b = sk_apply_lut_if<APPLY_PREBLEND>(fir[2], maskPreBlend.fB);
#if SK_SHOW_TEXT_BLIT_COVERAGE
            r = SkMax32(r, 10); g = SkMax32(g, 10); b = SkMax32(b, 10);
#endif
            dstP[pixel_x] = SkPack888ToRGB16(r, g, b);
        }
        dstP = (uint16_t*)((char*)dstP + dstRB);
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

static void packA8ToA1(const SkMask& mask, const uint8_t* src, size_t srcRB) {
    const int height = mask.fBounds.height();
    const int width = mask.fBounds.width();
    const int octs = width >> 3;
    const int leftOverBits = width & 7;

    uint8_t* dst = mask.fImage;
    const int dstPad = mask.fRowBytes - SkAlign8(width)/8;
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

static void generateMask(const SkMask& mask, const SkPath& path,
                         const SkMaskGamma::PreBlend& maskPreBlend) {
    SkPaint paint;

    int srcW = mask.fBounds.width();
    int srcH = mask.fBounds.height();
    int dstW = srcW;
    int dstH = srcH;
    int dstRB = mask.fRowBytes;

    SkMatrix matrix;
    matrix.setTranslate(-SkIntToScalar(mask.fBounds.fLeft),
                        -SkIntToScalar(mask.fBounds.fTop));

    paint.setAntiAlias(SkMask::kBW_Format != mask.fFormat);
    switch (mask.fFormat) {
        case SkMask::kBW_Format:
            dstRB = 0;  // signals we need a copy
            break;
        case SkMask::kA8_Format:
            break;
        case SkMask::kLCD16_Format:
            // TODO: trigger off LCD orientation
            dstW = 4*dstW - 8;
            matrix.setTranslate(-SkIntToScalar(mask.fBounds.fLeft + 1),
                                -SkIntToScalar(mask.fBounds.fTop));
            matrix.postScale(SkIntToScalar(4), SK_Scalar1);
            dstRB = 0;  // signals we need a copy
            break;
        default:
            SkDEBUGFAIL("unexpected mask format");
    }

    SkRasterClip clip;
    clip.setRect(SkIRect::MakeWH(dstW, dstH));

    const SkImageInfo info = SkImageInfo::MakeA8(dstW, dstH);
    SkAutoPixmapStorage dst;

    if (0 == dstRB) {
        if (!dst.tryAlloc(info)) {
            // can't allocate offscreen, so empty the mask and return
            sk_bzero(mask.fImage, mask.computeImageSize());
            return;
        }
    } else {
        dst.reset(info, mask.fImage, dstRB);
    }
    sk_bzero(dst.writable_addr(), dst.computeByteSize());

    SkDraw  draw;
    draw.fDst   = dst;
    draw.fRC    = &clip;
    draw.fMatrix = &matrix;
    draw.drawPath(path, paint);

    switch (mask.fFormat) {
        case SkMask::kBW_Format:
            packA8ToA1(mask, dst.addr8(0, 0), dst.rowBytes());
            break;
        case SkMask::kA8_Format:
            if (maskPreBlend.isApplicable()) {
                applyLUTToA8Mask(mask, maskPreBlend.fG);
            }
            break;
        case SkMask::kLCD16_Format:
            if (maskPreBlend.isApplicable()) {
                pack4xHToLCD16<true>(dst, mask, maskPreBlend);
            } else {
                pack4xHToLCD16<false>(dst, mask, maskPreBlend);
            }
            break;
        default:
            break;
    }
}

void SkScalerContext::getImage(const SkGlyph& origGlyph) {
    const SkGlyph*  glyph = &origGlyph;
    SkGlyph  tmpGlyph{origGlyph.getPackedID()};

    // in case we need to call generateImage on a mask-format that is different
    // (i.e. larger) than what our caller allocated by looking at origGlyph.
    SkAutoMalloc tmpGlyphImageStorage;

    if (fMaskFilter) {   // restore the prefilter bounds

        // need the original bounds, sans our maskfilter
        sk_sp<SkMaskFilter> mf = std::move(fMaskFilter);
        this->getMetrics(&tmpGlyph);
        fMaskFilter = std::move(mf);

        // we need the prefilter bounds to be <= filter bounds
        SkASSERT(tmpGlyph.fWidth <= origGlyph.fWidth);
        SkASSERT(tmpGlyph.fHeight <= origGlyph.fHeight);

        if (tmpGlyph.fMaskFormat == origGlyph.fMaskFormat) {
            tmpGlyph.fImage = origGlyph.fImage;
        } else {
            tmpGlyphImageStorage.reset(tmpGlyph.computeImageSize());
            tmpGlyph.fImage = tmpGlyphImageStorage.get();
        }
        glyph = &tmpGlyph;
    }

    if (!fGenerateImageFromPath) {
        generateImage(*glyph);
    } else {
        SkPath devPath;
        SkMask mask = glyph->mask();

        if (!this->internalGetPath(glyph->getPackedID(), &devPath)) {
            generateImage(*glyph);
        } else {
            SkASSERT(SkMask::kARGB32_Format != origGlyph.fMaskFormat);
            SkASSERT(SkMask::kARGB32_Format != mask.fFormat);
            generateMask(mask, devPath, fPreBlend);
        }
    }

    if (fMaskFilter) {
        // the src glyph image shouldn't be 3D
        SkASSERT(SkMask::k3D_Format != glyph->fMaskFormat);

        SkMask      srcM = glyph->mask(),
                    dstM;
        SkMatrix    matrix;

        fRec.getMatrixFrom2x2(&matrix);

        if (as_MFB(fMaskFilter)->filterMask(&dstM, srcM, matrix, nullptr)) {
            int width = SkMin32(origGlyph.fWidth, dstM.fBounds.width());
            int height = SkMin32(origGlyph.fHeight, dstM.fBounds.height());
            int dstRB = origGlyph.rowBytes();
            int srcRB = dstM.fRowBytes;

            const uint8_t* src = (const uint8_t*)dstM.fImage;
            uint8_t* dst = (uint8_t*)origGlyph.fImage;

            if (SkMask::k3D_Format == dstM.fFormat) {
                // we have to copy 3 times as much
                height *= 3;
            }

            // clean out our glyph, since it may be larger than dstM
            //sk_bzero(dst, height * dstRB);

            while (--height >= 0) {
                memcpy(dst, src, width);
                src += srcRB;
                dst += dstRB;
            }
            SkMask::FreeImage(dstM.fImage);
        }
    }
}

bool SkScalerContext::getPath(SkPackedGlyphID glyphID, SkPath* path) {
    return this->internalGetPath(glyphID, path);
}

void SkScalerContext::getFontMetrics(SkFontMetrics* fm) {
    SkASSERT(fm);
    this->generateFontMetrics(fm);
}

///////////////////////////////////////////////////////////////////////////////

bool SkScalerContext::internalGetPath(SkPackedGlyphID glyphID, SkPath* devPath) {
    SkPath  path;
    if (!generatePath(glyphID.code(), &path)) {
        return false;
    }

    if (fRec.fFlags & SkScalerContext::kSubpixelPositioning_Flag) {
        SkFixed dx = glyphID.getSubXFixed();
        SkFixed dy = glyphID.getSubYFixed();
        if (dx | dy) {
            path.offset(SkFixedToScalar(dx), SkFixedToScalar(dy));
        }
    }

    if (fRec.fFrameWidth > 0 || fPathEffect != nullptr) {
        // need the path in user-space, with only the point-size applied
        // so that our stroking and effects will operate the same way they
        // would if the user had extracted the path themself, and then
        // called drawPath
        SkPath      localPath;
        SkMatrix    matrix, inverse;

        fRec.getMatrixFrom2x2(&matrix);
        if (!matrix.invert(&inverse)) {
            // assume devPath is already empty.
            return true;
        }
        path.transform(inverse, &localPath);
        // now localPath is only affected by the paint settings, and not the canvas matrix

        SkStrokeRec rec(SkStrokeRec::kFill_InitStyle);

        if (fRec.fFrameWidth > 0) {
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
            if (fPathEffect->filterPath(&effectPath, localPath, &rec, nullptr)) {
                localPath.swap(effectPath);
            }
        }

        if (rec.needToApply()) {
            SkPath strokePath;
            if (rec.applyToPath(&strokePath, localPath)) {
                localPath.swap(strokePath);
            }
        }

        // now return stuff to the caller
        if (devPath) {
            localPath.transform(matrix, devPath);
        }
    } else {   // nothing tricky to do
        if (devPath) {
            devPath->swap(path);
        }
    }

    if (devPath) {
        devPath->updateBoundsCache();
    }
    return true;
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
        case kFull_PreMatrixScale:
            s->fX = SkScalarAbs(GA.get(SkMatrix::kMScaleX));
            s->fY = SkScalarAbs(GA.get(SkMatrix::kMScaleY));
            break;
        case kVertical_PreMatrixScale: {
            SkScalar yScale = SkScalarAbs(GA.get(SkMatrix::kMScaleY));
            s->fX = yScale;
            s->fY = yScale;
            break;
        }
        case kVerticalInteger_PreMatrixScale: {
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
            (kFull_PreMatrixScale == preMatrixScale) ||
            (kVertical_PreMatrixScale == preMatrixScale && A.getScaleX() == A.getScaleY())))
    {
        // If GA == A and kFull_PreMatrixScale, sA is identity.
        // If GA == A and kVertical_PreMatrixScale and A.scaleX == A.scaleY, sA is identity.
        sA->reset();
    } else if (!skewedOrFlipped && kVertical_PreMatrixScale == preMatrixScale) {
        // If GA == A and kVertical_PreMatrixScale, sA.scaleY is SK_Scalar1.
        sA->reset();
        sA->setScaleX(A.getScaleX() / s->fY);
    } else {
        // TODO: like kVertical_PreMatrixScale, kVerticalInteger_PreMatrixScale with int scales.
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

    if (0 == fPost2x2[1][0]) {
        // The x axis is mapped onto the x axis.
        return kX_SkAxisAlignment;
    }
    if (0 == fPost2x2[0][0]) {
        // The x axis is mapped onto the y axis.
        return kY_SkAxisAlignment;
    }
    return kNone_SkAxisAlignment;
}

void SkScalerContextRec::setLuminanceColor(SkColor c) {
    fLumBits = SkMaskGamma::CanonicalColor(
            SkColorSetRGB(SkColorGetR(c), SkColorGetG(c), SkColorGetB(c)));
}

///////////////////////////////////////////////////////////////////////////////

class SkScalerContext_Empty : public SkScalerContext {
public:
    SkScalerContext_Empty(sk_sp<SkTypeface> typeface, const SkScalerContextEffects& effects,
                          const SkDescriptor* desc)
        : SkScalerContext(std::move(typeface), effects, desc) {}

protected:
    unsigned generateGlyphCount() override {
        return 0;
    }
    uint16_t generateCharToGlyph(SkUnichar uni) override {
        return 0;
    }
    bool generateAdvance(SkGlyph* glyph) override {
        glyph->zeroMetrics();
        return true;
    }
    void generateMetrics(SkGlyph* glyph) override {
        glyph->fMaskFormat = fRec.fMaskFormat;
        glyph->zeroMetrics();
    }
    void generateImage(const SkGlyph& glyph) override {}
    bool generatePath(SkGlyphID glyph, SkPath* path) override {
        path->reset();
        return false;
    }
    void generateFontMetrics(SkFontMetrics* metrics) override {
        if (metrics) {
            sk_bzero(metrics, sizeof(*metrics));
        }
    }
};

extern SkScalerContext* SkCreateColorScalerContext(const SkDescriptor* desc);

std::unique_ptr<SkScalerContext> SkTypeface::createScalerContext(
    const SkScalerContextEffects& effects, const SkDescriptor* desc, bool allowFailure) const
{
    std::unique_ptr<SkScalerContext> c(this->onCreateScalerContext(effects, desc));
    if (!c && !allowFailure) {
        c = skstd::make_unique<SkScalerContext_Empty>(sk_ref_sp(const_cast<SkTypeface*>(this)),
                                                      effects, desc);
    }

    // !allowFailure implies c != nullptr
    SkASSERT(c || allowFailure);

    return c;
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

    SkTypeface* typeface = font.getTypefaceOrDefault();

    rec->fFontID = typeface->uniqueID();
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

    if (style != SkPaint::kFill_Style && strokeWidth > 0) {
        rec->fFrameWidth = strokeWidth;
        rec->fMiterLimit = paint.getStrokeMiter();
        rec->fStrokeJoin = SkToU8(paint.getStrokeJoin());
        rec->fStrokeCap = SkToU8(paint.getStrokeCap());

        if (style == SkPaint::kStrokeAndFill_Style) {
            flags |= SkScalerContext::kFrameAndFill_Flag;
        }
    } else {
        rec->fFrameWidth = 0;
        rec->fMiterLimit = 0;
        rec->fStrokeJoin = 0;
        rec->fStrokeCap = 0;
    }

    rec->fMaskFormat = SkToU8(compute_mask_format(font));

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
    rec->fFlags = SkToU16(flags);

    // if linear-text is on, then we force hinting to be off (since that's sort of
    // the point of linear-text.
    SkFontHinting hinting = (SkFontHinting)font.getHinting();
    if (font.isLinearMetrics()) {
        hinting = kNo_SkFontHinting;
    }

    // these modify fFlags, so do them after assigning fFlags
    rec->setHinting(hinting);

    rec->setLuminanceColor(SkPaintPriv::ComputeLuminanceColor(paint));

    // For now always set the paint gamma equal to the device gamma.
    // The math in SkMaskGamma can handle them being different,
    // but it requires superluminous masks when
    // Ex : deviceGamma(x) < paintGamma(x) and x is sufficiently large.
    rec->setDeviceGamma(SK_GAMMA_EXPONENT);
    rec->setPaintGamma(SK_GAMMA_EXPONENT);

#ifdef SK_GAMMA_CONTRAST
    rec->setContrast(SK_GAMMA_CONTRAST);
#else
    // A value of 0.5 for SK_GAMMA_CONTRAST appears to be a good compromise.
    // With lower values small text appears washed out (though correctly so).
    // With higher values lcd fringing is worse and the smoothing effect of
    // partial coverage is diminished.
    rec->setContrast(0.5f);
#endif

    if (!SkToBool(scalerContextFlags & SkScalerContextFlags::kFakeGamma)) {
        rec->ignoreGamma();
    }
    if (!SkToBool(scalerContextFlags & SkScalerContextFlags::kBoostContrast)) {
        rec->setContrast(0);
    }

    new (effects) SkScalerContextEffects{paint};
}

SkDescriptor* SkScalerContext::MakeDescriptorForPaths(SkFontID typefaceID,
                                                      SkAutoDescriptor* ad) {
    SkScalerContextRec rec;
    memset(&rec, 0, sizeof(rec));
    rec.fFontID = typefaceID;
    rec.fTextSize = SkFontPriv::kCanonicalTextSizeForPaths;
    rec.fPreScaleX = rec.fPost2x2[0][0] = rec.fPost2x2[1][1] = SK_Scalar1;
    return AutoDescriptorGivenRecAndEffects(rec, SkScalerContextEffects(), ad);
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
    desc->init();
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
    SkBinaryWriteBuffer buf;

    ad->reset(calculate_size_and_flatten(rec, effects, &buf));
    generate_descriptor(rec, buf, ad->getDesc());

    return ad->getDesc();
}

std::unique_ptr<SkDescriptor> SkScalerContext::DescriptorGivenRecAndEffects(
    const SkScalerContextRec& rec,
    const SkScalerContextEffects& effects)
{
    SkBinaryWriteBuffer buf;

    auto desc = SkDescriptor::Alloc(calculate_size_and_flatten(rec, effects, &buf));
    generate_descriptor(rec, buf, desc.get());

    return desc;
}

void SkScalerContext::DescriptorBufferGiveRec(const SkScalerContextRec& rec, void* buffer) {
    generate_descriptor(rec, SkBinaryWriteBuffer{}, (SkDescriptor*)buffer);
}

bool SkScalerContext::CheckBufferSizeForRec(const SkScalerContextRec& rec,
                                            const SkScalerContextEffects& effects,
                                            size_t size) {
    SkBinaryWriteBuffer buf;
    return size >= calculate_size_and_flatten(rec, effects, &buf);
}




