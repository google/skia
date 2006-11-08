/* libs/graphics/sgl/SkScalerContext.cpp
**
** Copyright 2006, Google Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include "SkScalerContext.h"
#include "SkDescriptor.h"
#include "SkDraw.h"
#include "SkFontHost.h"
#include "SkMaskFilter.h"
#include "SkPathEffect.h"
#include "SkRasterizer.h"
#include "SkRegion.h"
#include "SkStroke.h"
#include "SkThread.h"

#define ComputeBWRowBytes(width)        (((unsigned)(width) + 7) >> 3)

static uint16_t compute_rowbytes(unsigned format, unsigned width)
{
    if (format == SkMask::kBW_Format)
        width = ComputeBWRowBytes(width);
    return SkToU16(width);
}

static void glyph2mask(const SkGlyph& glyph, SkMask* mask)
{
    SkASSERT(&glyph && mask);

    mask->fImage = (uint8_t*)glyph.fImage;
    mask->fBounds.set(glyph.fLeft, glyph.fTop,
                    glyph.fLeft + glyph.fWidth,
                    glyph.fTop + glyph.fHeight);
    mask->fRowBytes = glyph.fRowBytes;
    mask->fFormat = glyph.fMaskFormat;
}

size_t SkGlyph::computeImageSize() const
{
    size_t size = fRowBytes * fHeight;
    if (fMaskFormat == SkMask::k3D_Format)
        size *= 3;
    return size;
}

#ifdef SK_DEBUG
    #define DUMP_RECx
#endif

static SkFlattenable* load_flattenable(const SkDescriptor* desc, uint32_t tag)
{
    SkFlattenable*  obj = NULL;
    uint32_t        len;
    const void*     data = desc->findEntry(tag, &len);

    if (data)
    {
        SkRBuffer   buffer(data, len);
        SkFlattenable::Factory fact = (SkFlattenable::Factory)buffer.readPtr();
        SkASSERT(fact);
        obj = fact(buffer);
        SkASSERT(buffer.pos() == buffer.size());
    }
    return obj;
}

SkScalerContext::SkScalerContext(const SkDescriptor* desc)
    : fPathEffect(NULL), fMaskFilter(NULL)
{
    fBaseGlyphCount = 0;
    fAuxScalerContext = NULL;

    const Rec* rec = (const Rec*)desc->findEntry(kRec_SkDescriptorTag, NULL);
    SkASSERT(rec);

    fRec = *rec;

#ifdef DUMP_REC
    desc->assertChecksum();
    SkDebugf("SkScalarContext checksum %x count %d length %d\n", desc->getChecksum(), desc->getCount(), desc->getLength());
    SkDebugf(" textsize %g prescale %g preskew %g post [%g %g %g %g]\n",
        rec->fTextSize, rec->fPreScaleX, rec->fPreSkewX, rec->fPost2x2[0][0],
        rec->fPost2x2[0][1], rec->fPost2x2[1][0], rec->fPost2x2[1][1]);
    SkDebugf("  frame %g miter %g hints %d framefill %d format %d join %d\n",
        rec->fFrameWidth, rec->fMiterLimit, rec->fHints, rec->fFrameAndFill,
        rec->fMaskFormat, rec->fStrokeJoin);
    SkDebugf("  pathEffect %x maskFilter %x\n", desc->findEntry(kPathEffect_SkDescriptorTag, NULL),
        desc->findEntry(kMaskFilter_SkDescriptorTag, NULL));
#endif

    fPathEffect = (SkPathEffect*)load_flattenable(desc, kPathEffect_SkDescriptorTag);
    fMaskFilter = (SkMaskFilter*)load_flattenable(desc, kMaskFilter_SkDescriptorTag);
    fRasterizer = (SkRasterizer*)load_flattenable(desc, kRasterizer_SkDescriptorTag);
}

SkScalerContext::~SkScalerContext()
{
    fPathEffect->safeUnref();
    fMaskFilter->safeUnref();
    fRasterizer->safeUnref();

    SkDELETE(fAuxScalerContext);
}

SkScalerContext* SkScalerContext::loadAuxContext() const
{
    if (NULL == fAuxScalerContext)
    {
        fAuxScalerContext = SkFontHost::CreateFallbackScalerContext(fRec);
        if (NULL != fAuxScalerContext)
            fAuxScalerContext->setBaseGlyphCount(this->getGlyphCount());
    }
    return fAuxScalerContext;
}

uint16_t SkScalerContext::charToGlyphID(SkUnichar uni)
{
    unsigned glyphID = this->generateCharToGlyph(uni);

    if (0 == glyphID)   // try auxcontext
    {
        unsigned base = this->getGlyphCount();

        SkScalerContext* ctx = this->loadAuxContext();
        if (NULL == ctx)
            return 0;
        
        glyphID = ctx->generateCharToGlyph(uni);
        if (0 != glyphID)   // only fiddle with it if its not missing
        {
            glyphID += base;
            if (glyphID > 0xFFFF)
                glyphID = 0;
        }
    }
    return SkToU16(glyphID);
}

/*  Internal routine to resolve auxContextID into a real context.
    Only makes sense to call once the glyph has been given a
    valid auxGlyphID.
*/
SkScalerContext* SkScalerContext::getGlyphContext(const SkGlyph& glyph) const
{
    SkScalerContext* ctx = const_cast<SkScalerContext*>(this);
    
    if (glyph.f_GlyphID >= this->getGlyphCount())
    {
        ctx = this->loadAuxContext();
        if (NULL == ctx)    // if no aux, just return us
            ctx = const_cast<SkScalerContext*>(this);
    }
    return ctx;
}

void SkScalerContext::getMetrics(SkGlyph* glyph)
{
    this->getGlyphContext(*glyph)->generateMetrics(glyph);

    // if either dimensin is empty, zap the image bounds of the glyph
    if (0 == glyph->fWidth || 0 == glyph->fHeight)
    {
        glyph->fWidth   = 0;
        glyph->fHeight  = 0;
        glyph->fTop     = 0;
        glyph->fLeft    = 0;
        glyph->fRowBytes = 0;
        glyph->fMaskFormat = 0;
        return;
    }
    
    if (fRec.fFrameWidth > 0 || fPathEffect != NULL || fRasterizer != NULL)
    {
        SkPath      devPath, fillPath;
        SkMatrix    fillToDevMatrix;

        this->internalGetPath(*glyph, &fillPath, &devPath, &fillToDevMatrix);

        if (fRasterizer)
        {
            SkMask  mask;

            if (fRasterizer->rasterize(fillPath, fillToDevMatrix, NULL,
                                       fMaskFilter, &mask,
                                       SkMask::kJustComputeBounds_CreateMode))
            {
                glyph->fLeft    = mask.fBounds.fLeft;
                glyph->fTop     = mask.fBounds.fTop;
                glyph->fWidth   = SkToU16(mask.fBounds.width());
                glyph->fHeight  = SkToU16(mask.fBounds.height());
            }
            else    // draw nothing 'cause we failed
            {
                glyph->fLeft    = 0;
                glyph->fTop     = 0;
                glyph->fWidth   = 0;
                glyph->fHeight  = 0;
                return;
            }
        }
        else    // just use devPath
        {
            SkRect      r;
            SkRect16    ir;

            devPath.computeBounds(&r, SkPath::kExact_BoundsType);
            r.roundOut(&ir);
            
            glyph->fLeft    = ir.fLeft;
            glyph->fTop     = ir.fTop;
            glyph->fWidth   = SkToU16(ir.width());
            glyph->fHeight  = SkToU16(ir.height());
        }
    }

    glyph->fMaskFormat = fRec.fMaskFormat;

    if (fMaskFilter)
    {
        SkMask      src, dst;
        SkMatrix    matrix;

        glyph2mask(*glyph, &src);
        fRec.getMatrixFrom2x2(&matrix);

        src.fImage = NULL;  // only want the bounds from the filter
        if (fMaskFilter->filterMask(&dst, src, matrix, NULL))
        {
            SkASSERT(dst.fImage == NULL);
            glyph->fLeft    = dst.fBounds.fLeft;
            glyph->fTop     = dst.fBounds.fTop;
            glyph->fWidth   = SkToU16(dst.fBounds.width());
            glyph->fHeight  = SkToU16(dst.fBounds.height());
            glyph->fMaskFormat = dst.fFormat;
        }
    }

    glyph->fRowBytes = compute_rowbytes(glyph->fMaskFormat, glyph->fWidth);
}

//#define PLAY_WITH_GAMMA

#ifdef PLAY_WITH_GAMMA
static SkFixed interp(SkFixed a, SkFixed b, int scale)  // scale is [0..255]
{
    return a + ((b - a) * scale >> 8);
}

static void filter_image(uint8_t image[], size_t size)
{
    static uint8_t gGammaTable[256];
    static bool gInit;

    if (!gInit)
    {
        for (int i = 0; i < 256; i++)
        {
            SkFixed n = i * 257;
            n += n >> 15;
            SkASSERT(n >= 0 && n <= SK_Fixed1);
        
        //    n = SkFixedSqrt(n);
            n = interp(SkFixedMul(n, n), n, 0xDD);

            n = n * 255 >> 16;
        //  SkDebugf("morph %d -> %d\n", i, n);
            gGammaTable[i] = SkToU8(n);
        }
        gInit = true;
    }
    
    const uint8_t*   table = gGammaTable;
    uint8_t*         stop = image + size;
    while (image < stop)
    {
        *image = table[*image];
        image += 1;
    }
}
#endif

void SkScalerContext::getImage(const SkGlyph& origGlyph)
{
    const SkGlyph*  glyph = &origGlyph;

    SkGlyph tmpGlyph;
    if (fMaskFilter)    // restore the prefilter bounds
    {
        tmpGlyph.f_GlyphID = origGlyph.f_GlyphID;

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

    if (fRec.fFrameWidth > 0 || fPathEffect != NULL || fRasterizer != NULL)
    {
        SkPath      devPath, fillPath;
        SkMatrix    fillToDevMatrix;

        this->internalGetPath(*glyph, &fillPath, &devPath, &fillToDevMatrix);

        if (fRasterizer)
        {
            SkMask  mask;
            
            mask.fFormat = SkMask::kA8_Format;
            mask.fRowBytes = glyph->fRowBytes;
            mask.fBounds.set(glyph->fLeft,
                             glyph->fTop,
                             glyph->fLeft + glyph->fWidth,
                             glyph->fTop + glyph->fHeight);
            mask.fImage = (uint8_t*)glyph->fImage;
            memset(glyph->fImage, 0, glyph->fRowBytes * glyph->fHeight);
            
            if (!fRasterizer->rasterize(fillPath, fillToDevMatrix, NULL,
                                        fMaskFilter, &mask, SkMask::kJustRenderImage_CreateMode))
            {
                return;
            }
        }
        else
        {
            SkBitmap    bm;
            SkBitmap::Config config;
            SkMatrix    matrix;
            SkRegion    clip;
            SkPaint     paint;
            SkDraw      draw;

            if (SkMask::kA8_Format == fRec.fMaskFormat)
            {
                config = SkBitmap::kA8_Config;
                paint.setAntiAliasOn(true);
            }
            else
            {
                SkASSERT(SkMask::kBW_Format == fRec.fMaskFormat);
                config = SkBitmap::kA1_Config;
                paint.setAntiAliasOn(false);
            }

            clip.setRect(0, 0, glyph->fWidth, glyph->fHeight);
            matrix.setTranslate(-SkIntToScalar(glyph->fLeft), -SkIntToScalar(glyph->fTop));
            bm.setConfig(config, glyph->fWidth, glyph->fHeight);
            bm.setPixels(glyph->fImage);
            memset(glyph->fImage, 0, bm.height() * bm.rowBytes());

            draw.fClip  = &clip;
            draw.fMatrix = &matrix;
            draw.fDevice = &bm;
            draw.fBounder = NULL;
            draw.drawPath(devPath, paint);
        }
    }
    else
    {
        this->getGlyphContext(*glyph)->generateImage(*glyph);
    }

    if (fMaskFilter)
    {
        SkMask      srcM, dstM;
        SkMatrix    matrix;

        SkASSERT(SkMask::k3D_Format != glyph->fMaskFormat); // the src glyph image shouldn't be 3D
        glyph2mask(*glyph, &srcM);
        fRec.getMatrixFrom2x2(&matrix);

        if (fMaskFilter->filterMask(&dstM, srcM, matrix, NULL))
        {
            if (true)   // hack until I can figure out why the assert below sometimes fails
            {
                int width = SkFastMin32(origGlyph.fWidth, dstM.fBounds.width());
                int height = SkFastMin32(origGlyph.fHeight, dstM.fBounds.height());
                int srcRB = origGlyph.fRowBytes;
                int dstRB = dstM.fRowBytes;
                
                const uint8_t* src = (const uint8_t*)dstM.fImage;
                uint8_t* dst = (uint8_t*)origGlyph.fImage;
                
                if (SkMask::k3D_Format == dstM.fFormat)   // we have to copy 3 times as much
                    height *= 3;

                while (--height >= 0)
                {
                    memcpy(dst, src, width);
                    src += srcRB;
                    dst += dstRB;
                }
            }
            else
            {
                SkASSERT(origGlyph.fWidth == dstM.fBounds.width());
                SkASSERT(origGlyph.fTop == dstM.fBounds.fTop);
                SkASSERT(origGlyph.fLeft == dstM.fBounds.fLeft);
                SkASSERT(origGlyph.fHeight == dstM.fBounds.height());
                SkASSERT(origGlyph.computeImageSize() == dstM.computeTotalImageSize());

                memcpy(glyph->fImage, dstM.fImage, dstM.computeTotalImageSize());
            }
            SkMask::FreeImage(dstM.fImage);
        }
    }
}

void SkScalerContext::getPath(const SkGlyph& glyph, SkPath* path)
{
    this->internalGetPath(glyph, NULL, path, NULL);
}

void SkScalerContext::getLineHeight(SkPoint* above, SkPoint* below)
{
    this->generateLineHeight(above, below);

    // apply any mods due to effects (e.g. stroking, etc.)...
}

///////////////////////////////////////////////////////////////////////

void SkScalerContext::internalGetPath(const SkGlyph& glyph, SkPath* fillPath, SkPath* devPath, SkMatrix* fillToDevMatrix)
{
    SkPath  path;

    this->getGlyphContext(glyph)->generatePath(glyph, &path);

    if (fRec.fFrameWidth > 0 || fPathEffect != NULL)
    {
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

        if (fPathEffect)
        {
            SkPath effectPath;

            if (fPathEffect->filterPath(&effectPath, localPath, &width))
                localPath.swap(effectPath);
        }

        if (width > 0)
        {
            SkStroke    stroker;
            SkPath      outline;

            stroker.setWidth(width);
            stroker.setMiterLimit(fRec.fMiterLimit);
            stroker.setJoin((SkPaint::Join)fRec.fStrokeJoin);
            stroker.setDoFill(fRec.fFrameAndFill != 0);
            stroker.strokePath(localPath, &outline);
            localPath.swap(outline);
        }
        
        // now return stuff to the caller
        if (fillToDevMatrix)
            *fillToDevMatrix = matrix;
        
        if (devPath)
            localPath.transform(matrix, devPath);
        
        if (fillPath)
            fillPath->swap(localPath);
    }
    else    // nothing tricky to do
    {
        if (fillToDevMatrix)
            fillToDevMatrix->reset();
        
        if (devPath)
        {
            if (fillPath == NULL)
                devPath->swap(path);
            else
                *devPath = path;
        }
        
        if (fillPath)
            fillPath->swap(path);
    }
}


void SkScalerContext::Rec::getMatrixFrom2x2(SkMatrix* dst) const
{
    dst->reset();
    dst->setScaleX(fPost2x2[0][0]);
    dst->setSkewX( fPost2x2[0][1]);
    dst->setSkewY( fPost2x2[1][0]);
    dst->setScaleY(fPost2x2[1][1]);
}

void SkScalerContext::Rec::getLocalMatrix(SkMatrix* m) const
{
    m->setScale(SkScalarMul(fTextSize, fPreScaleX), fTextSize);
    if (fPreSkewX)
        m->postSkew(fPreSkewX, 0);
}

void SkScalerContext::Rec::getSingleMatrix(SkMatrix* m) const
{
    this->getLocalMatrix(m);

    //  now concat the device matrix
    {
        SkMatrix    deviceMatrix;
        this->getMatrixFrom2x2(&deviceMatrix);
        m->postConcat(deviceMatrix);
    }
}

#include "SkFontHost.h"

SkScalerContext* SkScalerContext::Create(const SkDescriptor* desc)
{
    return SkFontHost::CreateScalerContext(desc);
}

