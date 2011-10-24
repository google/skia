
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
#include "SkMaskFilter.h"
#include "SkPathEffect.h"
#include "SkRasterizer.h"
#include "SkRasterClip.h"
#include "SkStroke.h"
#include "SkThread.h"

#define ComputeBWRowBytes(width)        (((unsigned)(width) + 7) >> 3)

static const uint8_t* gBlackGammaTable;
static const uint8_t* gWhiteGammaTable;

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
        SkFlattenableReadBuffer   buffer(data, len);
        obj = buffer.readFlattenable();
        SkASSERT(buffer.offset() == buffer.size());
    }
    return obj;
}

SkScalerContext::SkScalerContext(const SkDescriptor* desc)
    : fPathEffect(NULL), fMaskFilter(NULL)
{
    static bool gHaveGammaTables;
    if (!gHaveGammaTables) {
        const uint8_t* tables[2];
        SkFontHost::GetGammaTables(tables);
        gBlackGammaTable = tables[0];
        gWhiteGammaTable = tables[1];
        gHaveGammaTables = true;
    }

    fBaseGlyphCount = 0;
    fNextContext = NULL;

    const Rec* rec = (const Rec*)desc->findEntry(kRec_SkDescriptorTag, NULL);
    SkASSERT(rec);

    fRec = *rec;

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

    fPathEffect = (SkPathEffect*)load_flattenable(desc, kPathEffect_SkDescriptorTag);
    fMaskFilter = (SkMaskFilter*)load_flattenable(desc, kMaskFilter_SkDescriptorTag);
    fRasterizer = (SkRasterizer*)load_flattenable(desc, kRasterizer_SkDescriptorTag);

    // initialize based on our settings. subclasses can also force this
    fGenerateImageFromPath = fRec.fFrameWidth > 0 || fPathEffect != NULL ||
                             fRasterizer != NULL;
}

SkScalerContext::~SkScalerContext() {
    SkDELETE(fNextContext);

    SkSafeUnref(fPathEffect);
    SkSafeUnref(fMaskFilter);
    SkSafeUnref(fRasterizer);
}

static SkScalerContext* allocNextContext(const SkScalerContext::Rec& rec) {
    // fonthost will determine the next possible font to search, based
    // on the current font in fRec. It will return NULL if ctx is our
    // last font that can be searched (i.e. ultimate fallback font)
    uint32_t newFontID = SkFontHost::NextLogicalFont(rec.fFontID, rec.fOrigFontID);
    if (0 == newFontID) {
        return NULL;
    }

    SkAutoDescriptor    ad(sizeof(rec) + SkDescriptor::ComputeOverhead(1));
    SkDescriptor*       desc = ad.getDesc();

    desc->init();
    SkScalerContext::Rec* newRec =
    (SkScalerContext::Rec*)desc->addEntry(kRec_SkDescriptorTag,
                                          sizeof(rec), &rec);
    newRec->fFontID = newFontID;
    desc->computeChecksum();

    return SkFontHost::CreateScalerContext(desc);
}

/*  Return the next context, creating it if its not already created, but return
    NULL if the fonthost says there are no more fonts to fallback to.
 */
SkScalerContext* SkScalerContext::getNextContext() {
    SkScalerContext* next = fNextContext;
    // if next is null, then either it isn't cached yet, or we're at the
    // end of our possible chain
    if (NULL == next) {
        next = allocNextContext(fRec);
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
            SkDebugf("--- no context for glyph %x\n", glyph.getGlyphID());
            // just return the original context (this)
            return this;
        }
    }
    return ctx;
}

/*  This loops through all available fallback contexts (if needed) until it
    finds some context that can handle the unichar. If all fail, returns 0
 */
uint16_t SkScalerContext::charToGlyphID(SkUnichar uni) {
    SkScalerContext* ctx = this;
    unsigned glyphID;
    for (;;) {
        glyphID = ctx->generateCharToGlyph(uni);
        if (glyphID) {
            break;  // found it
        }
        ctx = ctx->getNextContext();
        if (NULL == ctx) {
            return 0;   // no more contexts, return missing glyph
        }
    }
    // add the ctx's base, making glyphID unique for chain of contexts
    glyphID += ctx->fBaseGlyphCount;
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

static bool isLCD(const SkScalerContext::Rec& rec) {
    return SkMask::kLCD16_Format == rec.fMaskFormat ||
           SkMask::kLCD32_Format == rec.fMaskFormat;
}

static uint16_t a8_to_rgb565(unsigned a8) {
    return SkPackRGB16(a8 >> 3, a8 >> 2, a8 >> 3);
}

static void copyToLCD16(const SkBitmap& src, const SkMask& dst) {
    SkASSERT(SkBitmap::kA8_Config == src.config());
    SkASSERT(SkMask::kLCD16_Format == dst.fFormat);

    const int width = dst.fBounds.width();
    const int height = dst.fBounds.height();
    const uint8_t* srcP = src.getAddr8(0, 0);
    size_t srcRB = src.rowBytes();
    uint16_t* dstP = (uint16_t*)dst.fImage;
    size_t dstRB = dst.fRowBytes;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            dstP[x] = a8_to_rgb565(srcP[x]);
        }
        srcP += srcRB;
        dstP = (uint16_t*)((char*)dstP + dstRB);
    }
}

#define SK_FREETYPE_LCD_LERP    160

static int lerp(int start, int end) {
    SkASSERT((unsigned)SK_FREETYPE_LCD_LERP <= 256);
    return start + ((end - start) * (SK_FREETYPE_LCD_LERP) >> 8);
}

static uint16_t packLCD16(unsigned r, unsigned g, unsigned b) {
    if (SK_FREETYPE_LCD_LERP) {
        // want (a+b+c)/3, but we approx to avoid the divide
        unsigned ave = (5 * (r + g + b) + g) >> 4;
        r = lerp(r, ave);
        g = lerp(g, ave);
        b = lerp(b, ave);
    }
    return SkPackRGB16(r >> 3, g >> 2, b >> 3);
}

static void pack3xHToLCD16(const SkBitmap& src, const SkMask& dst) {
    SkASSERT(SkBitmap::kA8_Config == src.config());
    SkASSERT(SkMask::kLCD16_Format == dst.fFormat);
    
    const int width = dst.fBounds.width();
    const int height = dst.fBounds.height();
    uint16_t* dstP = (uint16_t*)dst.fImage;
    size_t dstRB = dst.fRowBytes;
    for (int y = 0; y < height; ++y) {
        const uint8_t* srcP = src.getAddr8(0, y);
        for (int x = 0; x < width; ++x) {
            unsigned r = *srcP++;
            unsigned g = *srcP++;
            unsigned b = *srcP++;
            dstP[x] = packLCD16(r, g, b);
        }
        dstP = (uint16_t*)((char*)dstP + dstRB);
    }
}

static void pack3xHToLCD32(const SkBitmap& src, const SkMask& dst) {
    SkASSERT(SkBitmap::kA8_Config == src.config());
    SkASSERT(SkMask::kLCD32_Format == dst.fFormat);
    
    const int width = dst.fBounds.width();
    const int height = dst.fBounds.height();
    SkPMColor* dstP = (SkPMColor*)dst.fImage;
    size_t dstRB = dst.fRowBytes;
    for (int y = 0; y < height; ++y) {
        const uint8_t* srcP = src.getAddr8(0, y);
        for (int x = 0; x < width; ++x) {
            unsigned r = *srcP++;
            unsigned g = *srcP++;
            unsigned b = *srcP++;
            unsigned a = SkMax32(SkMax32(r, g), b);
            dstP[x] = SkPackARGB32(a, r, g, b);
        }
        dstP = (SkPMColor*)((char*)dstP + dstRB);
    }
}

static void generateMask(const SkMask& mask, const SkPath& path) {
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
                SkASSERT(!"unexpected mask format");
        }
    }

    SkRasterClip clip;
    clip.setRect(SkIRect::MakeWH(dstW, dstH));

    SkBitmap bm;
    bm.setConfig(config, dstW, dstH, dstRB);

    if (0 == dstRB) {
        bm.allocPixels();
        bm.lockPixels();
    } else {
        bm.setPixels(mask.fImage);
    }
    sk_bzero(bm.getPixels(), bm.getSafeSize());
    
    SkDraw  draw;
    sk_bzero(&draw, sizeof(draw));
    draw.fRC    = &clip;
    draw.fClip  = &clip.bwRgn();
    draw.fMatrix = &matrix;
    draw.fBitmap = &bm;
    draw.drawPath(path, paint);
    
    if (0 == dstRB) {
        switch (mask.fFormat) {
            case SkMask::kLCD16_Format:
                pack3xHToLCD16(bm, mask);
                break;
            case SkMask::kLCD32_Format:
                pack3xHToLCD32(bm, mask);
                break;
            default:
                SkASSERT(!"bad format for copyback");
        }
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
        } else {
            generateMask(mask, devPath);
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
        }
    }

    // check to see if we should filter the alpha channel

    if (NULL == fMaskFilter &&
        fRec.fMaskFormat != SkMask::kBW_Format &&
        fRec.fMaskFormat != SkMask::kLCD16_Format &&
        fRec.fMaskFormat != SkMask::kLCD32_Format &&
        (fRec.fFlags & (kGammaForBlack_Flag | kGammaForWhite_Flag)) != 0)
    {
        const uint8_t* table = (fRec.fFlags & kGammaForBlack_Flag) ? gBlackGammaTable : gWhiteGammaTable;
        if (NULL != table) {
            uint8_t* dst = (uint8_t*)origGlyph.fImage;
            unsigned rowBytes = origGlyph.rowBytes();

            for (int y = origGlyph.fHeight - 1; y >= 0; --y) {
                for (int x = origGlyph.fWidth - 1; x >= 0; --x) {
                    dst[x] = table[dst[x]];
                }
                dst += rowBytes;
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
        matrix.invert(&inverse);
        path.transform(inverse, &localPath);
        // now localPath is only affected by the paint settings, and not the canvas matrix

        SkScalar width = fRec.fFrameWidth;

        if (fPathEffect) {
            SkPath effectPath;

            if (fPathEffect->filterPath(&effectPath, localPath, &width)) {
                localPath.swap(effectPath);
            }
        }

        if (width > 0) {
            SkStroke    stroker;
            SkPath      outline;

            stroker.setWidth(width);
            stroker.setMiterLimit(fRec.fMiterLimit);
            stroker.setJoin((SkPaint::Join)fRec.fStrokeJoin);
            stroker.setDoFill(SkToBool(fRec.fFlags & kFrameAndFill_Flag));
            stroker.strokePath(localPath, &outline);
            localPath.swap(outline);
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


void SkScalerContext::Rec::getMatrixFrom2x2(SkMatrix* dst) const {
    dst->reset();
    dst->setScaleX(fPost2x2[0][0]);
    dst->setSkewX( fPost2x2[0][1]);
    dst->setSkewY( fPost2x2[1][0]);
    dst->setScaleY(fPost2x2[1][1]);
}

void SkScalerContext::Rec::getLocalMatrix(SkMatrix* m) const {
    m->setScale(SkScalarMul(fTextSize, fPreScaleX), fTextSize);
    if (fPreSkewX) {
        m->postSkew(fPreSkewX, 0);
    }
}

void SkScalerContext::Rec::getSingleMatrix(SkMatrix* m) const {
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
    SkScalerContext_Empty(const SkDescriptor* desc) : SkScalerContext(desc) {}

protected:
    virtual unsigned generateGlyphCount() {
        return 0;
    }
    virtual uint16_t generateCharToGlyph(SkUnichar uni) {
        return 0;
    }
    virtual void generateAdvance(SkGlyph* glyph) {
        glyph->zeroMetrics();
    }
    virtual void generateMetrics(SkGlyph* glyph) {
        glyph->zeroMetrics();
    }
    virtual void generateImage(const SkGlyph& glyph) {}
    virtual void generatePath(const SkGlyph& glyph, SkPath* path) {}
    virtual void generateFontMetrics(SkPaint::FontMetrics* mx,
                                     SkPaint::FontMetrics* my) {
        if (mx) {
            sk_bzero(mx, sizeof(*mx));
        }
        if (my) {
            sk_bzero(my, sizeof(*my));
        }
    }
};

extern SkScalerContext* SkCreateColorScalerContext(const SkDescriptor* desc);

SkScalerContext* SkScalerContext::Create(const SkDescriptor* desc) {
	SkScalerContext* c = NULL;  //SkCreateColorScalerContext(desc);
	if (NULL == c) {
		c = SkFontHost::CreateScalerContext(desc);
	}
    if (NULL == c) {
        c = SkNEW_ARGS(SkScalerContext_Empty, (desc));
    }
    return c;
}

