/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkScalerContext.h"

#include "SkAutoMalloc.h"
#include "SkAutoPixmapStorage.h"
#include "SkColorPriv.h"
#include "SkDescriptor.h"
#include "SkDraw.h"
#include "SkGlyph.h"
#include "SkMakeUnique.h"
#include "SkMaskFilter.h"
#include "SkMaskGamma.h"
#include "SkMatrix22.h"
#include "SkPaintPriv.h"
#include "SkPathEffect.h"
#include "SkRasterClip.h"
#include "SkRasterizer.h"
#include "SkReadBuffer.h"
#include "SkStroke.h"
#include "SkStrokeRec.h"
#include "SkWriteBuffer.h"

#define ComputeBWRowBytes(width)        (((unsigned)(width) + 7) >> 3)

void SkGlyph::toMask(SkMask* mask) const {
    SkASSERT(mask);

    mask->fImage = (uint8_t*)fImage;
    mask->fBounds.set(fLeft, fTop, fLeft + fWidth, fTop + fHeight);
    mask->fRowBytes = this->rowBytes();
    mask->fFormat = static_cast<SkMask::Format>(fMaskFormat);
}

size_t SkGlyph::computeImageSize() const {
    const size_t size = this->rowBytes() * fHeight;

    switch (fMaskFormat) {
        case SkMask::k3D_Format:
            return 3 * size;
        default:
            return size;
    }
}

void SkGlyph::zeroMetrics() {
    fAdvanceX = 0;
    fAdvanceY = 0;
    fWidth    = 0;
    fHeight   = 0;
    fTop      = 0;
    fLeft     = 0;
    fRsbDelta = 0;
    fLsbDelta = 0;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
    #define DUMP_RECx
#endif

SkScalerContext::SkScalerContext(sk_sp<SkTypeface> typeface, const SkScalerContextEffects& effects,
                                 const SkDescriptor* desc)
    : fRec(*static_cast<const Rec*>(desc->findEntry(kRec_SkDescriptorTag, nullptr)))

    , fTypeface(std::move(typeface))
    , fPathEffect(sk_ref_sp(effects.fPathEffect))
    , fMaskFilter(sk_ref_sp(effects.fMaskFilter))
    , fRasterizer(sk_ref_sp(effects.fRasterizer))
      // Initialize based on our settings. Subclasses can also force this.
    , fGenerateImageFromPath(fRec.fFrameWidth > 0 || fPathEffect != nullptr || fRasterizer != nullptr)

    , fPreBlend(fMaskFilter ? SkMaskGamma::PreBlend() : SkScalerContext::GetMaskPreBlend(fRec))
    , fPreBlendForFilter(fMaskFilter ? SkScalerContext::GetMaskPreBlend(fRec)
                                     : SkMaskGamma::PreBlend())
{
#ifdef DUMP_REC
    desc->assertChecksum();
    SkDebugf("SkScalerContext checksum %x count %d length %d\n",
             desc->getChecksum(), desc->getCount(), desc->getLength());
    SkDebugf(" textsize %g prescale %g preskew %g post [%g %g %g %g]\n",
        rec->fTextSize, rec->fPreScaleX, rec->fPreSkewX, rec->fPost2x2[0][0],
        rec->fPost2x2[0][1], rec->fPost2x2[1][0], rec->fPost2x2[1][1]);
    SkDebugf("  frame %g miter %g hints %d framefill %d format %d join %d cap %d\n",
        rec->fFrameWidth, rec->fMiterLimit, rec->fHints, rec->fFrameAndFill,
        rec->fMaskFormat, rec->fStrokeJoin, rec->fStrokeCap);
    SkDebugf("  pathEffect %x maskFilter %x\n",
             desc->findEntry(kPathEffect_SkDescriptorTag, nullptr),
        desc->findEntry(kMaskFilter_SkDescriptorTag, nullptr));
#endif
}

SkScalerContext::~SkScalerContext() {}

void SkScalerContext::getAdvance(SkGlyph* glyph) {
    // mark us as just having a valid advance
    glyph->fMaskFormat = MASK_FORMAT_JUST_ADVANCE;
    // we mark the format before making the call, in case the impl
    // internally ends up calling its generateMetrics, which is OK
    // albeit slower than strictly necessary
    generateAdvance(glyph);
}

void SkScalerContext::getMetrics(SkGlyph* glyph) {
    generateMetrics(glyph);

    // for now we have separate cache entries for devkerning on and off
    // in the future we might share caches, but make our measure/draw
    // code make the distinction. Thus we zap the values if the caller
    // has not asked for them.
    if ((fRec.fFlags & SkScalerContext::kDevKernText_Flag) == 0) {
        // no devkern, so zap the fields
        glyph->fLsbDelta = glyph->fRsbDelta = 0;
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

    if (fGenerateImageFromPath) {
        SkPath      devPath, fillPath;
        SkMatrix    fillToDevMatrix;

        this->internalGetPath(glyph->getPackedID(), &fillPath, &devPath, &fillToDevMatrix);

        if (fRasterizer) {
            SkMask  mask;

            if (fRasterizer->rasterize(fillPath, fillToDevMatrix, nullptr,
                                       fMaskFilter.get(), &mask,
                                       SkMask::kJustComputeBounds_CreateMode)) {
                glyph->fLeft    = mask.fBounds.fLeft;
                glyph->fTop     = mask.fBounds.fTop;
                glyph->fWidth   = SkToU16(mask.fBounds.width());
                glyph->fHeight  = SkToU16(mask.fBounds.height());
            } else {
                goto SK_ERROR;
            }
        } else {
            // just use devPath
            const SkIRect ir = devPath.getBounds().roundOut();

            if (ir.isEmpty() || !ir.is16Bit()) {
                goto SK_ERROR;
            }
            glyph->fLeft    = ir.fLeft;
            glyph->fTop     = ir.fTop;
            glyph->fWidth   = SkToU16(ir.width());
            glyph->fHeight  = SkToU16(ir.height());

            if (glyph->fWidth > 0) {
                switch (fRec.fMaskFormat) {
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

    if (SkMask::kARGB32_Format != glyph->fMaskFormat) {
        glyph->fMaskFormat = fRec.fMaskFormat;
    }

    // If we are going to create the mask, then we cannot keep the color
    if ((fGenerateImageFromPath || fMaskFilter) &&
            SkMask::kARGB32_Format == glyph->fMaskFormat) {
        glyph->fMaskFormat = SkMask::kA8_Format;
    }

    if (fMaskFilter) {
        SkMask      src, dst;
        SkMatrix    matrix;

        glyph->toMask(&src);
        fRec.getMatrixFrom2x2(&matrix);

        src.fImage = nullptr;  // only want the bounds from the filter
        if (fMaskFilter->filterMask(&dst, src, matrix, nullptr)) {
            if (dst.fBounds.isEmpty() || !dst.fBounds.is16Bit()) {
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
    glyph->fLeft    = 0;
    glyph->fTop     = 0;
    glyph->fWidth   = 0;
    glyph->fHeight  = 0;
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
    sk_bzero(dst.writable_addr(), dst.getSafeSize());

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

static void extract_alpha(const SkMask& dst,
                          const SkPMColor* srcRow, size_t srcRB) {
    int width = dst.fBounds.width();
    int height = dst.fBounds.height();
    int dstRB = dst.fRowBytes;
    uint8_t* dstRow = dst.fImage;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            dstRow[x] = SkGetPackedA32(srcRow[x]);
        }
        // zero any padding on each row
        for (int x = width; x < dstRB; ++x) {
            dstRow[x] = 0;
        }
        dstRow += dstRB;
        srcRow = (const SkPMColor*)((const char*)srcRow + srcRB);
    }
}

void SkScalerContext::getImage(const SkGlyph& origGlyph) {
    const SkGlyph*  glyph = &origGlyph;
    SkGlyph         tmpGlyph;

    // in case we need to call generateImage on a mask-format that is different
    // (i.e. larger) than what our caller allocated by looking at origGlyph.
    SkAutoMalloc tmpGlyphImageStorage;

    // If we are going to draw-from-path, then we cannot generate color, since
    // the path only makes a mask. This case should have been caught up in
    // generateMetrics().
    SkASSERT(!fGenerateImageFromPath ||
             SkMask::kARGB32_Format != origGlyph.fMaskFormat);

    if (fMaskFilter) {   // restore the prefilter bounds
        tmpGlyph.initWithGlyphID(origGlyph.getPackedID());

        // need the original bounds, sans our maskfilter
        SkMaskFilter* mf = fMaskFilter.release();   // temp disable
        this->getMetrics(&tmpGlyph);
        fMaskFilter = sk_sp<SkMaskFilter>(mf);      // restore

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

    if (fGenerateImageFromPath) {
        SkPath      devPath, fillPath;
        SkMatrix    fillToDevMatrix;
        SkMask      mask;

        this->internalGetPath(glyph->getPackedID(), &fillPath, &devPath, &fillToDevMatrix);
        glyph->toMask(&mask);

        if (fRasterizer) {
            mask.fFormat = SkMask::kA8_Format;
            sk_bzero(glyph->fImage, mask.computeImageSize());

            if (!fRasterizer->rasterize(fillPath, fillToDevMatrix, nullptr,
                                        fMaskFilter.get(), &mask,
                                        SkMask::kJustRenderImage_CreateMode)) {
                return;
            }
            if (fPreBlend.isApplicable()) {
                applyLUTToA8Mask(mask, fPreBlend.fG);
            }
        } else {
            SkASSERT(SkMask::kARGB32_Format != mask.fFormat);
            generateMask(mask, devPath, fPreBlend);
        }
    } else {
        generateImage(*glyph);
    }

    if (fMaskFilter) {
        SkMask      srcM, dstM;
        SkMatrix    matrix;

        // the src glyph image shouldn't be 3D
        SkASSERT(SkMask::k3D_Format != glyph->fMaskFormat);

        SkAutoSMalloc<32*32> a8storage;
        glyph->toMask(&srcM);
        if (SkMask::kARGB32_Format == srcM.fFormat) {
            // now we need to extract the alpha-channel from the glyph's image
            // and copy it into a temp buffer, and then point srcM at that temp.
            srcM.fFormat = SkMask::kA8_Format;
            srcM.fRowBytes = SkAlign4(srcM.fBounds.width());
            size_t size = srcM.computeImageSize();
            a8storage.reset(size);
            srcM.fImage = (uint8_t*)a8storage.get();
            extract_alpha(srcM,
                          (const SkPMColor*)glyph->fImage, glyph->rowBytes());
        }

        fRec.getMatrixFrom2x2(&matrix);

        if (fMaskFilter->filterMask(&dstM, srcM, matrix, nullptr)) {
            int width = SkFastMin32(origGlyph.fWidth, dstM.fBounds.width());
            int height = SkFastMin32(origGlyph.fHeight, dstM.fBounds.height());
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

            if (fPreBlendForFilter.isApplicable()) {
                applyLUTToA8Mask(srcM, fPreBlendForFilter.fG);
            }
        }
    }
}

void SkScalerContext::getPath(SkPackedGlyphID glyphID, SkPath* path) {
    this->internalGetPath(glyphID, nullptr, path, nullptr);
}

void SkScalerContext::getFontMetrics(SkPaint::FontMetrics* fm) {
    SkASSERT(fm);
    this->generateFontMetrics(fm);
}

SkUnichar SkScalerContext::generateGlyphToChar(uint16_t glyph) {
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

void SkScalerContext::internalGetPath(SkPackedGlyphID glyphID, SkPath* fillPath,
                                      SkPath* devPath, SkMatrix* fillToDevMatrix) {
    SkPath  path;
    generatePath(glyphID.code(), &path);

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
            // assume fillPath and devPath are already empty.
            return;
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
        if (fillToDevMatrix) {
            *fillToDevMatrix = matrix;
        }
        if (devPath) {
            localPath.transform(matrix, devPath);
        }
        if (fillPath) {
            fillPath->swap(localPath);
        }
    } else {   // nothing tricky to do
        if (fillToDevMatrix) {
            fillToDevMatrix->reset();
        }
        if (devPath) {
            if (fillPath == nullptr) {
                devPath->swap(path);
            } else {
                *devPath = path;
            }
        }

        if (fillPath) {
            fillPath->swap(path);
        }
    }

    if (devPath) {
        devPath->updateBoundsCache();
    }
    if (fillPath) {
        fillPath->updateBoundsCache();
    }
}


void SkScalerContextRec::getMatrixFrom2x2(SkMatrix* dst) const {
    dst->setAll(fPost2x2[0][0], fPost2x2[0][1], 0,
                fPost2x2[1][0], fPost2x2[1][1], 0,
                0,              0,              1);
}

void SkScalerContextRec::getLocalMatrix(SkMatrix* m) const {
    SkPaintPriv::MakeTextMatrix(m, fTextSize, fPreScaleX, fPreSkewX);
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

    // If the 'total' matrix is singular, set the 'scale' to something finite and zero the matrices.
    // All underlying ports have issues with zero text size, so use the matricies to zero.

    // Map the vectors [0,1], [1,0], [1,1] and [1,-1] (the EM) through the 'total' matrix.
    // If the length of one of these vectors is less than 1/256 then an EM filling square will
    // never affect any pixels.
    SkVector diag[4] = { { A.getScaleX()               ,                 A.getSkewY() },
                         {                 A.getSkewX(), A.getScaleY()                },
                         { A.getScaleX() + A.getSkewX(), A.getScaleY() + A.getSkewY() },
                         { A.getScaleX() - A.getSkewX(), A.getScaleY() - A.getSkewY() }, };
    if (diag[0].lengthSqd() <= SK_ScalarNearlyZero * SK_ScalarNearlyZero ||
        diag[1].lengthSqd() <= SK_ScalarNearlyZero * SK_ScalarNearlyZero ||
        diag[2].lengthSqd() <= SK_ScalarNearlyZero * SK_ScalarNearlyZero ||
        diag[3].lengthSqd() <= SK_ScalarNearlyZero * SK_ScalarNearlyZero)
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

    // GA is the matrix A with rotation removed.
    SkMatrix GA;
    bool skewedOrFlipped = A.getSkewX() || A.getSkewY() || A.getScaleX() < 0 || A.getScaleY() < 0;
    if (skewedOrFlipped) {
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

SkAxisAlignment SkScalerContext::computeAxisAlignmentForHText() {
    // Why fPost2x2 can be used here.
    // getSingleMatrix multiplies in getLocalMatrix, which consists of
    // * fTextSize (a scale, which has no effect)
    // * fPreScaleX (a scale in x, which has no effect)
    // * fPreSkewX (has no effect, but would on vertical text alignment).
    // In other words, making the text bigger, stretching it along the
    // horizontal axis, or fake italicizing it does not move the baseline.

    if (0 == fRec.fPost2x2[1][0]) {
        // The x axis is mapped onto the x axis.
        return kX_SkAxisAlignment;
    }
    if (0 == fRec.fPost2x2[0][0]) {
        // The x axis is mapped onto the y axis.
        return kY_SkAxisAlignment;
    }
    return kNone_SkAxisAlignment;
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
    void generateAdvance(SkGlyph* glyph) override {
        glyph->zeroMetrics();
    }
    void generateMetrics(SkGlyph* glyph) override {
        glyph->zeroMetrics();
    }
    void generateImage(const SkGlyph& glyph) override {}
    void generatePath(SkGlyphID glyph, SkPath* path) override {}
    void generateFontMetrics(SkPaint::FontMetrics* metrics) override {
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
    return c;
}
