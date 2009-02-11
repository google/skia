/* libs/graphics/sgl/SkScalerContext.cpp
**
** Copyright 2006, The Android Open Source Project
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

#ifdef SK_DEBUG
//    #define TRACK_MISSING_CHARS
#endif

#define ComputeBWRowBytes(width)        (((unsigned)(width) + 7) >> 3)

static const uint8_t* gBlackGammaTable;
static const uint8_t* gWhiteGammaTable;

void SkGlyph::toMask(SkMask* mask) const {
    SkASSERT(mask);

    mask->fImage = (uint8_t*)fImage;
    mask->fBounds.set(fLeft, fTop, fLeft + fWidth, fTop + fHeight);
    mask->fRowBytes = this->rowBytes();
    mask->fFormat = fMaskFormat;
}

size_t SkGlyph::computeImageSize() const {
    size_t size = this->rowBytes() * fHeight;
    if (fMaskFormat == SkMask::k3D_Format) {
        size *= 3;
    }
    return size;
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

SkScalerContext::~SkScalerContext() {
    fPathEffect->safeUnref();
    fMaskFilter->safeUnref();
    fRasterizer->safeUnref();

    SkDELETE(fAuxScalerContext);
}

SkScalerContext* SkScalerContext::loadAuxContext() const {
    if (NULL == fAuxScalerContext) {
        fAuxScalerContext = SkFontHost::CreateFallbackScalerContext(fRec);
        if (NULL != fAuxScalerContext) {
            fAuxScalerContext->setBaseGlyphCount(this->getGlyphCount());
        }
    }
    return fAuxScalerContext;
}

#ifdef TRACK_MISSING_CHARS
    static uint8_t gMissingChars[1 << 13];
#endif

uint16_t SkScalerContext::charToGlyphID(SkUnichar uni) {
    unsigned glyphID = this->generateCharToGlyph(uni);

    if (0 == glyphID) {   // try auxcontext
        SkScalerContext* ctx = this->loadAuxContext();
        if (NULL != ctx) {
            glyphID = ctx->generateCharToGlyph(uni);
            if (0 != glyphID) {   // only fiddle with it if its not missing
                glyphID += this->getGlyphCount();
                if (glyphID > 0xFFFF) {
                    glyphID = 0;
                }
            }
        }
    }
#ifdef TRACK_MISSING_CHARS
    if (0 == glyphID) {
        bool announce = false;
        if (uni > 0xFFFF) {   // we don't record these
            announce = true;
        } else {
            unsigned index = uni >> 3;
            unsigned mask = 1 << (uni & 7);
            SkASSERT(index < SK_ARRAY_COUNT(gMissingChars));
            if ((gMissingChars[index] & mask) == 0) {
                gMissingChars[index] |= mask;
                announce = true;
            }
        }
        if (announce) {
            printf(">>> MISSING CHAR <<< 0x%04X\n", uni);
        }
    }
#endif
    return SkToU16(glyphID);
}

/*  Internal routine to resolve auxContextID into a real context.
    Only makes sense to call once the glyph has been given a
    valid auxGlyphID.
*/
SkScalerContext* SkScalerContext::getGlyphContext(const SkGlyph& glyph) const {
    SkScalerContext* ctx = const_cast<SkScalerContext*>(this);
    
    if (glyph.getGlyphID() >= this->getGlyphCount()) {
        ctx = this->loadAuxContext();
        if (NULL == ctx) {    // if no aux, just return us
            ctx = const_cast<SkScalerContext*>(this);
        }
    }
    return ctx;
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
    
    if (fRec.fFrameWidth > 0 || fPathEffect != NULL || fRasterizer != NULL) {
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
                // draw nothing 'cause we failed
                glyph->fLeft    = 0;
                glyph->fTop     = 0;
                glyph->fWidth   = 0;
                glyph->fHeight  = 0;
                return;
            }
        } else {
            // just use devPath
            SkRect  r;
            SkIRect ir;

            devPath.computeBounds(&r, SkPath::kExact_BoundsType);
            r.roundOut(&ir);
            
            glyph->fLeft    = ir.fLeft;
            glyph->fTop     = ir.fTop;
            glyph->fWidth   = SkToU16(ir.width());
            glyph->fHeight  = SkToU16(ir.height());
        }
    }

    glyph->fMaskFormat = fRec.fMaskFormat;

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
}

void SkScalerContext::getImage(const SkGlyph& origGlyph) {
    const SkGlyph*  glyph = &origGlyph;
    SkGlyph         tmpGlyph;

    if (fMaskFilter) {   // restore the prefilter bounds
        tmpGlyph.fID = origGlyph.fID;

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

    if (fRec.fFrameWidth > 0 || fPathEffect != NULL || fRasterizer != NULL) {
        SkPath      devPath, fillPath;
        SkMatrix    fillToDevMatrix;

        this->internalGetPath(*glyph, &fillPath, &devPath, &fillToDevMatrix);

        if (fRasterizer) {
            SkMask  mask;
            
            glyph->toMask(&mask);
            mask.fFormat = SkMask::kA8_Format;
            bzero(glyph->fImage, mask.computeImageSize());
            
            if (!fRasterizer->rasterize(fillPath, fillToDevMatrix, NULL,
                                        fMaskFilter, &mask,
                                        SkMask::kJustRenderImage_CreateMode)) {
                return;
            }
        } else {
            SkBitmap    bm;
            SkBitmap::Config config;
            SkMatrix    matrix;
            SkRegion    clip;
            SkPaint     paint;
            SkDraw      draw;

            if (SkMask::kA8_Format == fRec.fMaskFormat) {
                config = SkBitmap::kA8_Config;
                paint.setAntiAlias(true);
            } else {
                SkASSERT(SkMask::kBW_Format == fRec.fMaskFormat);
                config = SkBitmap::kA1_Config;
                paint.setAntiAlias(false);
            }

            clip.setRect(0, 0, glyph->fWidth, glyph->fHeight);
            matrix.setTranslate(-SkIntToScalar(glyph->fLeft),
                                -SkIntToScalar(glyph->fTop));
            bm.setConfig(config, glyph->fWidth, glyph->fHeight,
                         glyph->rowBytes());
            bm.setPixels(glyph->fImage);
            bzero(glyph->fImage, bm.height() * bm.rowBytes());

            draw.fClip  = &clip;
            draw.fMatrix = &matrix;
            draw.fBitmap = &bm;
            draw.fBounder = NULL;
            draw.drawPath(devPath, paint);
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
            //bzero(dst, height * dstRB);

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
        (fRec.fFlags & (kGammaForBlack_Flag | kGammaForWhite_Flag)) != 0)
    {
        const uint8_t* table = (fRec.fFlags & kGammaForBlack_Flag) ? gBlackGammaTable : gWhiteGammaTable;
        if (NULL != table)
        {
            uint8_t* dst = (uint8_t*)origGlyph.fImage;
            unsigned rowBytes = origGlyph.rowBytes();
            
            for (int y = origGlyph.fHeight - 1; y >= 0; --y)
            {
                for (int x = origGlyph.fWidth - 1; x >= 0; --x)
                    dst[x] = table[dst[x]];
                dst += rowBytes;
            }
        }
    }
}

void SkScalerContext::getPath(const SkGlyph& glyph, SkPath* path)
{
    this->internalGetPath(glyph, NULL, path, NULL);
}

void SkScalerContext::getFontMetrics(SkPaint::FontMetrics* mx, SkPaint::FontMetrics* my)
{
    this->generateFontMetrics(mx, my);
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
            stroker.setDoFill(SkToBool(fRec.fFlags & kFrameAndFill_Flag));
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
    
    if (devPath)
        devPath->updateBoundsCache();
    if (fillPath)
        fillPath->updateBoundsCache();
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

///////////////////////////////////////////////////////////////////////////////

#include "SkFontHost.h"

class SkScalerContext_Empty : public SkScalerContext {
public:
    SkScalerContext_Empty(const SkDescriptor* desc) : SkScalerContext(desc) {}

protected:
    virtual unsigned generateGlyphCount() const {
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
            bzero(mx, sizeof(*mx));
        }
        if (my) {
            bzero(my, sizeof(*my));
        }
    }
};

SkScalerContext* SkScalerContext::Create(const SkDescriptor* desc)
{
    SkScalerContext* c = SkFontHost::CreateScalerContext(desc);
    if (NULL == c) {
        c = SkNEW_ARGS(SkScalerContext_Empty, (desc));
    }
    return c;
}

