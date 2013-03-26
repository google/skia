
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkScalerContext.h"
#include "SkColorPriv.h"
#include "SkDescriptor.h"
#include "SkDraw.h"
#include "SkFontHost.h"
#include "SkGlyph.h"
#include "SkMaskFilter.h"
#include "SkMaskGamma.h"
#include "SkOrderedReadBuffer.h"
#include "SkPathEffect.h"
#include "SkRasterizer.h"
#include "SkRasterClip.h"
#include "SkStroke.h"
#include "SkThread.h"

#ifdef SK_BUILD_FOR_ANDROID
    #include "SkTypeface_android.h"
#endif

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

static SkFlattenable* load_flattenable(const SkDescriptor* desc, uint32_t tag) {
    SkFlattenable*  obj = NULL;
    uint32_t        len;
    const void*     data = desc->findEntry(tag, &len);

    if (data) {
        SkOrderedReadBuffer   buffer(data, len);
        obj = buffer.readFlattenable();
        SkASSERT(buffer.offset() == buffer.size());
    }
    return obj;
}

SkScalerContext::SkScalerContext(SkTypeface* typeface, const SkDescriptor* desc)
    : fRec(*static_cast<const Rec*>(desc->findEntry(kRec_SkDescriptorTag, NULL)))

    , fBaseGlyphCount(0)
    , fTypeface(SkRef(typeface))
    , fPathEffect(static_cast<SkPathEffect*>(load_flattenable(desc, kPathEffect_SkDescriptorTag)))
    , fMaskFilter(static_cast<SkMaskFilter*>(load_flattenable(desc, kMaskFilter_SkDescriptorTag)))
    , fRasterizer(static_cast<SkRasterizer*>(load_flattenable(desc, kRasterizer_SkDescriptorTag)))

      // Initialize based on our settings. Subclasses can also force this.
    , fGenerateImageFromPath(fRec.fFrameWidth > 0 || fPathEffect != NULL || fRasterizer != NULL)

    , fNextContext(NULL)

    , fPreBlend(fMaskFilter ? SkMaskGamma::PreBlend() : SkScalerContext::GetMaskPreBlend(fRec))
    , fPreBlendForFilter(fMaskFilter ? SkScalerContext::GetMaskPreBlend(fRec)
                                     : SkMaskGamma::PreBlend())
{
#ifdef DUMP_REC
    desc->assertChecksum();
    SkDebugf("SkScalarContext checksum %x count %d length %d\n",
             desc->getChecksum(), desc->getCount(), desc->getLength());
    SkDebugf(" textsize %g prescale %g preskew %g post [%g %g %g %g]\n",
        rec->fTextSize, rec->fPreScaleX, rec->fPreSkewX, rec->fPost2x2[0][0],
        rec->fPost2x2[0][1], rec->fPost2x2[1][0], rec->fPost2x2[1][1]);
    SkDebugf("  frame %g miter %g hints %d framefill %d format %d join %d\n",
        rec->fFrameWidth, rec->fMiterLimit, rec->fHints, rec->fFrameAndFill,
        rec->fMaskFormat, rec->fStrokeJoin);
    SkDebugf("  pathEffect %x maskFilter %x\n",
             desc->findEntry(kPathEffect_SkDescriptorTag, NULL),
        desc->findEntry(kMaskFilter_SkDescriptorTag, NULL));
#endif
}

SkScalerContext::~SkScalerContext() {
    SkDELETE(fNextContext);

    SkSafeUnref(fPathEffect);
    SkSafeUnref(fMaskFilter);
    SkSafeUnref(fRasterizer);
}

// Return the context associated with the next logical typeface, or NULL if
// there are no more entries in the fallback chain.
SkScalerContext* SkScalerContext::allocNextContext() const {
#ifdef SK_BUILD_FOR_ANDROID
    SkTypeface* newFace = SkAndroidNextLogicalTypeface(fRec.fFontID,
                                                       fRec.fOrigFontID);
    if (0 == newFace) {
        return NULL;
    }

    SkAutoTUnref<SkTypeface> aur(newFace);
    uint32_t newFontID = newFace->uniqueID();

    SkAutoDescriptor    ad(sizeof(fRec) + SkDescriptor::ComputeOverhead(1));
    SkDescriptor*       desc = ad.getDesc();

    desc->init();
    SkScalerContext::Rec* newRec =
    (SkScalerContext::Rec*)desc->addEntry(kRec_SkDescriptorTag,
                                          sizeof(fRec), &fRec);
    newRec->fFontID = newFontID;
    desc->computeChecksum();

    return newFace->createScalerContext(desc);
#else
    return NULL;
#endif
}

/*  Return the next context, creating it if its not already created, but return
    NULL if the fonthost says there are no more fonts to fallback to.
 */
SkScalerContext* SkScalerContext::getNextContext() {
    SkScalerContext* next = fNextContext;
    // if next is null, then either it isn't cached yet, or we're at the
    // end of our possible chain
    if (NULL == next) {
        next = this->allocNextContext();
        if (NULL == next) {
            return NULL;
        }
        // next's base is our base + our local count
        next->setBaseGlyphCount(fBaseGlyphCount + this->getGlyphCount());
        // cache the answer
        fNextContext = next;
    }
    return next;
}

SkScalerContext* SkScalerContext::getGlyphContext(const SkGlyph& glyph) {
    unsigned glyphID = glyph.getGlyphID();
    SkScalerContext* ctx = this;
    for (;;) {
        unsigned count = ctx->getGlyphCount();
        if (glyphID < count) {
            break;
        }
        glyphID -= count;
        ctx = ctx->getNextContext();
        if (NULL == ctx) {
//            SkDebugf("--- no context for glyph %x\n", glyph.getGlyphID());
            // just return the original context (this)
            return this;
        }
    }
    return ctx;
}

SkScalerContext* SkScalerContext::getContextFromChar(SkUnichar uni,
                                                     uint16_t* glyphID) {
    SkScalerContext* ctx = this;
    for (;;) {
        const uint16_t glyph = ctx->generateCharToGlyph(uni);
        if (glyph) {
            if (NULL != glyphID) {
                *glyphID = glyph;
            }
            break;  // found it
        }
        ctx = ctx->getNextContext();
        if (NULL == ctx) {
            return NULL;
        }
    }
    return ctx;
}

#ifdef SK_BUILD_FOR_ANDROID
SkFontID SkScalerContext::findTypefaceIdForChar(SkUnichar uni) {
    SkScalerContext* ctx = this->getContextFromChar(uni, NULL);
    if (NULL != ctx) {
        return ctx->fRec.fFontID;
    } else {
        return 0;
    }
}

/*  This loops through all available fallback contexts (if needed) until it
    finds some context that can handle the unichar and return it.

    As this is somewhat expensive operation, it should only be done on the first
    char of a run.
 */
unsigned SkScalerContext::getBaseGlyphCount(SkUnichar uni) {
    SkScalerContext* ctx = this->getContextFromChar(uni, NULL);
    if (NULL != ctx) {
        return ctx->fBaseGlyphCount;
    } else {
        SkDEBUGF(("--- no context for char %x\n", uni));
        return this->fBaseGlyphCount;
    }
}
#endif

/*  This loops through all available fallback contexts (if needed) until it
    finds some context that can handle the unichar. If all fail, returns 0
 */
uint16_t SkScalerContext::charToGlyphID(SkUnichar uni) {

    uint16_t tempID;
    SkScalerContext* ctx = this->getContextFromChar(uni, &tempID);
    if (NULL == ctx) {
        return 0; // no more contexts, return missing glyph
    }
    // add the ctx's base, making glyphID unique for chain of contexts
    unsigned glyphID = tempID + ctx->fBaseGlyphCount;
    // check for overflow of 16bits, since our glyphID cannot exceed that
    if (glyphID > 0xFFFF) {
        glyphID = 0;
    }
    return SkToU16(glyphID);
}

SkUnichar SkScalerContext::glyphIDToChar(uint16_t glyphID) {
    SkScalerContext* ctx = this;
    unsigned rangeEnd = 0;
    do {
        unsigned rangeStart = rangeEnd;

        rangeEnd += ctx->getGlyphCount();
        if (rangeStart <= glyphID && glyphID < rangeEnd) {
            return ctx->generateGlyphToChar(glyphID - rangeStart);
        }
        ctx = ctx->getNextContext();
    } while (NULL != ctx);
    return 0;
}

void SkScalerContext::getAdvance(SkGlyph* glyph) {
    // mark us as just having a valid advance
    glyph->fMaskFormat = MASK_FORMAT_JUST_ADVANCE;
    // we mark the format before making the call, in case the impl
    // internally ends up calling its generateMetrics, which is OK
    // albeit slower than strictly necessary
    this->getGlyphContext(*glyph)->generateAdvance(glyph);
}

void SkScalerContext::getMetrics(SkGlyph* glyph) {
    this->getGlyphContext(*glyph)->generateMetrics(glyph);

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

        this->internalGetPath(*glyph, &fillPath, &devPath, &fillToDevMatrix);

        if (fRasterizer) {
            SkMask  mask;

            if (fRasterizer->rasterize(fillPath, fillToDevMatrix, NULL,
                                       fMaskFilter, &mask,
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
            SkIRect ir;
            devPath.getBounds().roundOut(&ir);

            if (ir.isEmpty() || !ir.is16Bit()) {
                goto SK_ERROR;
            }
            glyph->fLeft    = ir.fLeft;
            glyph->fTop     = ir.fTop;
            glyph->fWidth   = SkToU16(ir.width());
            glyph->fHeight  = SkToU16(ir.height());
        }
    }

    if (SkMask::kARGB32_Format != glyph->fMaskFormat) {
        glyph->fMaskFormat = fRec.fMaskFormat;
    }

    if (fMaskFilter) {
        SkMask      src, dst;
        SkMatrix    matrix;

        glyph->toMask(&src);
        fRec.getMatrixFrom2x2(&matrix);

        src.fImage = NULL;  // only want the bounds from the filter
        if (fMaskFilter->filterMask(&dst, src, matrix, NULL)) {
            if (dst.fBounds.isEmpty() || !dst.fBounds.is16Bit()) {
                goto SK_ERROR;
            }
            SkASSERT(dst.fImage == NULL);
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
static void pack3xHToLCD16(const SkBitmap& src, const SkMask& dst,
                           const SkMaskGamma::PreBlend& maskPreBlend) {
    SkASSERT(SkBitmap::kA8_Config == src.config());
    SkASSERT(SkMask::kLCD16_Format == dst.fFormat);

    const int width = dst.fBounds.width();
    const int height = dst.fBounds.height();
    uint16_t* dstP = (uint16_t*)dst.fImage;
    size_t dstRB = dst.fRowBytes;

    for (int y = 0; y < height; ++y) {
        const uint8_t* srcP = src.getAddr8(0, y);
        for (int x = 0; x < width; ++x) {
            U8CPU r = sk_apply_lut_if<APPLY_PREBLEND>(*srcP++, maskPreBlend.fR);
            U8CPU g = sk_apply_lut_if<APPLY_PREBLEND>(*srcP++, maskPreBlend.fG);
            U8CPU b = sk_apply_lut_if<APPLY_PREBLEND>(*srcP++, maskPreBlend.fB);
            dstP[x] = SkPack888ToRGB16(r, g, b);
        }
        dstP = (uint16_t*)((char*)dstP + dstRB);
    }
}

template<bool APPLY_PREBLEND>
static void pack3xHToLCD32(const SkBitmap& src, const SkMask& dst,
                           const SkMaskGamma::PreBlend& maskPreBlend) {
    SkASSERT(SkBitmap::kA8_Config == src.config());
    SkASSERT(SkMask::kLCD32_Format == dst.fFormat);

    const int width = dst.fBounds.width();
    const int height = dst.fBounds.height();
    SkPMColor* dstP = (SkPMColor*)dst.fImage;
    size_t dstRB = dst.fRowBytes;

    for (int y = 0; y < height; ++y) {
        const uint8_t* srcP = src.getAddr8(0, y);
        for (int x = 0; x < width; ++x) {
            U8CPU r = sk_apply_lut_if<APPLY_PREBLEND>(*srcP++, maskPreBlend.fR);
            U8CPU g = sk_apply_lut_if<APPLY_PREBLEND>(*srcP++, maskPreBlend.fG);
            U8CPU b = sk_apply_lut_if<APPLY_PREBLEND>(*srcP++, maskPreBlend.fB);
            dstP[x] = SkPackARGB32(0xFF, r, g, b);
        }
        dstP = (SkPMColor*)((char*)dstP + dstRB);
    }
}

static void generateMask(const SkMask& mask, const SkPath& path,
                         const SkMaskGamma::PreBlend& maskPreBlend) {
    SkBitmap::Config config;
    SkPaint     paint;

    int srcW = mask.fBounds.width();
    int srcH = mask.fBounds.height();
    int dstW = srcW;
    int dstH = srcH;
    int dstRB = mask.fRowBytes;

    SkMatrix matrix;
    matrix.setTranslate(-SkIntToScalar(mask.fBounds.fLeft),
                        -SkIntToScalar(mask.fBounds.fTop));

    if (SkMask::kBW_Format == mask.fFormat) {
        config = SkBitmap::kA1_Config;
        paint.setAntiAlias(false);
    } else {
        config = SkBitmap::kA8_Config;
        paint.setAntiAlias(true);
        switch (mask.fFormat) {
            case SkMask::kA8_Format:
                break;
            case SkMask::kLCD16_Format:
            case SkMask::kLCD32_Format:
                // TODO: trigger off LCD orientation
                dstW *= 3;
                matrix.postScale(SkIntToScalar(3), SK_Scalar1);
                dstRB = 0;  // signals we need a copy
                break;
            default:
                SkDEBUGFAIL("unexpected mask format");
        }
    }

    SkRasterClip clip;
    clip.setRect(SkIRect::MakeWH(dstW, dstH));

    SkBitmap bm;
    bm.setConfig(config, dstW, dstH, dstRB);

    if (0 == dstRB) {
        if (!bm.allocPixels()) {
            // can't allocate offscreen, so empty the mask and return
            sk_bzero(mask.fImage, mask.computeImageSize());
            return;
        }
        bm.lockPixels();
    } else {
        bm.setPixels(mask.fImage);
    }
    sk_bzero(bm.getPixels(), bm.getSafeSize());

    SkDraw  draw;
    draw.fRC    = &clip;
    draw.fClip  = &clip.bwRgn();
    draw.fMatrix = &matrix;
    draw.fBitmap = &bm;
    draw.drawPath(path, paint);

    switch (mask.fFormat) {
        case SkMask::kA8_Format:
            if (maskPreBlend.isApplicable()) {
                applyLUTToA8Mask(mask, maskPreBlend.fG);
            }
            break;
        case SkMask::kLCD16_Format:
            if (maskPreBlend.isApplicable()) {
                pack3xHToLCD16<true>(bm, mask, maskPreBlend);
            } else {
                pack3xHToLCD16<false>(bm, mask, maskPreBlend);
            }
            break;
        case SkMask::kLCD32_Format:
            if (maskPreBlend.isApplicable()) {
                pack3xHToLCD32<true>(bm, mask, maskPreBlend);
            } else {
                pack3xHToLCD32<false>(bm, mask, maskPreBlend);
            }
            break;
        default:
            break;
    }
}

void SkScalerContext::getImage(const SkGlyph& origGlyph) {
    const SkGlyph*  glyph = &origGlyph;
    SkGlyph         tmpGlyph;

    if (fMaskFilter) {   // restore the prefilter bounds
        tmpGlyph.init(origGlyph.fID);

        // need the original bounds, sans our maskfilter
        SkMaskFilter* mf = fMaskFilter;
        fMaskFilter = NULL;             // temp disable
        this->getMetrics(&tmpGlyph);
        fMaskFilter = mf;               // restore

        tmpGlyph.fImage = origGlyph.fImage;

        // we need the prefilter bounds to be <= filter bounds
        SkASSERT(tmpGlyph.fWidth <= origGlyph.fWidth);
        SkASSERT(tmpGlyph.fHeight <= origGlyph.fHeight);
        glyph = &tmpGlyph;
    }

    if (fGenerateImageFromPath) {
        SkPath      devPath, fillPath;
        SkMatrix    fillToDevMatrix;
        SkMask      mask;

        this->internalGetPath(*glyph, &fillPath, &devPath, &fillToDevMatrix);
        glyph->toMask(&mask);

        if (fRasterizer) {
            mask.fFormat = SkMask::kA8_Format;
            sk_bzero(glyph->fImage, mask.computeImageSize());

            if (!fRasterizer->rasterize(fillPath, fillToDevMatrix, NULL,
                                        fMaskFilter, &mask,
                                        SkMask::kJustRenderImage_CreateMode)) {
                return;
            }
            if (fPreBlend.isApplicable()) {
                applyLUTToA8Mask(mask, fPreBlend.fG);
            }
        } else {
            generateMask(mask, devPath, fPreBlend);
        }
    } else {
        this->getGlyphContext(*glyph)->generateImage(*glyph);
    }

    if (fMaskFilter) {
        SkMask      srcM, dstM;
        SkMatrix    matrix;

        // the src glyph image shouldn't be 3D
        SkASSERT(SkMask::k3D_Format != glyph->fMaskFormat);
        glyph->toMask(&srcM);
        fRec.getMatrixFrom2x2(&matrix);

        if (fMaskFilter->filterMask(&dstM, srcM, matrix, NULL)) {
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

void SkScalerContext::getPath(const SkGlyph& glyph, SkPath* path) {
    this->internalGetPath(glyph, NULL, path, NULL);
}

void SkScalerContext::getFontMetrics(SkPaint::FontMetrics* mx,
                                     SkPaint::FontMetrics* my) {
    this->generateFontMetrics(mx, my);
}

SkUnichar SkScalerContext::generateGlyphToChar(uint16_t glyph) {
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

void SkScalerContext::internalGetPath(const SkGlyph& glyph, SkPath* fillPath,
                                  SkPath* devPath, SkMatrix* fillToDevMatrix) {
    SkPath  path;

    this->getGlyphContext(glyph)->generatePath(glyph, &path);

    if (fRec.fFlags & SkScalerContext::kSubpixelPositioning_Flag) {
        SkFixed dx = glyph.getSubXFixed();
        SkFixed dy = glyph.getSubYFixed();
        if (dx | dy) {
            path.offset(SkFixedToScalar(dx), SkFixedToScalar(dy));
        }
    }

    if (fRec.fFrameWidth > 0 || fPathEffect != NULL) {
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
            rec.setStrokeParams(SkPaint::kButt_Cap,
                                (SkPaint::Join)fRec.fStrokeJoin,
                                fRec.fMiterLimit);
        }

        if (fPathEffect) {
            SkPath effectPath;
            if (fPathEffect->filterPath(&effectPath, localPath, &rec, NULL)) {
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
            if (fillPath == NULL) {
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
                0,              0,              SkScalarToPersp(SK_Scalar1));
}

void SkScalerContextRec::getLocalMatrix(SkMatrix* m) const {
    m->setScale(SkScalarMul(fTextSize, fPreScaleX), fTextSize);
    if (fPreSkewX) {
        m->postSkew(fPreSkewX, 0);
    }
}

void SkScalerContextRec::getSingleMatrix(SkMatrix* m) const {
    this->getLocalMatrix(m);

    //  now concat the device matrix
    SkMatrix    deviceMatrix;
    this->getMatrixFrom2x2(&deviceMatrix);
    m->postConcat(deviceMatrix);
}

SkAxisAlignment SkComputeAxisAlignmentForHText(const SkMatrix& matrix) {
    SkASSERT(!matrix.hasPerspective());

    if (0 == matrix[SkMatrix::kMSkewY]) {
        return kX_SkAxisAlignment;
    }
    if (0 == matrix[SkMatrix::kMScaleX]) {
        return kY_SkAxisAlignment;
    }
    return kNone_SkAxisAlignment;
}

///////////////////////////////////////////////////////////////////////////////

#include "SkFontHost.h"

class SkScalerContext_Empty : public SkScalerContext {
public:
    SkScalerContext_Empty(SkTypeface* face, const SkDescriptor* desc)
        : SkScalerContext(face, desc) {}

protected:
    virtual unsigned generateGlyphCount() SK_OVERRIDE {
        return 0;
    }
    virtual uint16_t generateCharToGlyph(SkUnichar uni) SK_OVERRIDE {
        return 0;
    }
    virtual void generateAdvance(SkGlyph* glyph) SK_OVERRIDE {
        glyph->zeroMetrics();
    }
    virtual void generateMetrics(SkGlyph* glyph) SK_OVERRIDE {
        glyph->zeroMetrics();
    }
    virtual void generateImage(const SkGlyph& glyph) SK_OVERRIDE {}
    virtual void generatePath(const SkGlyph& glyph, SkPath* path) SK_OVERRIDE {}
    virtual void generateFontMetrics(SkPaint::FontMetrics* mx,
                                     SkPaint::FontMetrics* my) SK_OVERRIDE {
        if (mx) {
            sk_bzero(mx, sizeof(*mx));
        }
        if (my) {
            sk_bzero(my, sizeof(*my));
        }
    }
};

extern SkScalerContext* SkCreateColorScalerContext(const SkDescriptor* desc);

SkScalerContext* SkTypeface::createScalerContext(const SkDescriptor* desc) const {
    SkScalerContext* c = NULL;  //SkCreateColorScalerContext(desc);
    if (NULL == c) {
        c = this->onCreateScalerContext(desc);
    }
    if (NULL == c) {
        c = SkNEW_ARGS(SkScalerContext_Empty,
                       (const_cast<SkTypeface*>(this), desc));
    }
    return c;
}
