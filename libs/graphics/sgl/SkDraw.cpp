/* libs/graphics/sgl/SkDraw.cpp
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

#include "SkDraw.h"
#include "SkBlitter.h"
#include "SkBounder.h"
#include "SkCanvas.h"
#include "SkMaskFilter.h"
#include "SkPaint.h"
#include "SkPathEffect.h"
#include "SkRasterizer.h"
#include "SkScan.h"
#include "SkShader.h"
#include "SkStroke.h"
#include "SkTemplatesPriv.h"

/** Helper for allocating small blitters on the stack.
*/

#define kBlitterStorageLongCount    40

class SkAutoBlitterChoose {
public:
    SkAutoBlitterChoose(const SkBitmap& device, const SkMatrix& matrix, const SkPaint& paint)
    {
        fBlitter = SkBlitter::Choose(device, matrix, paint, fStorage, sizeof(fStorage));
    }
    ~SkAutoBlitterChoose();

    SkBlitter*  operator->() { return fBlitter; }
    SkBlitter*  get() const { return fBlitter; }

private:
    SkBlitter*  fBlitter;
    uint32_t    fStorage[kBlitterStorageLongCount];
};

SkAutoBlitterChoose::~SkAutoBlitterChoose()
{
    if ((void*)fBlitter == (void*)fStorage)
        fBlitter->~SkBlitter();
    else
        SkDELETE(fBlitter);
}

class SkAutoBitmapShaderInstall {
public:
    SkAutoBitmapShaderInstall(const SkBitmap& src, const SkPaint* paint) : fPaint((SkPaint*)paint)
    {
        fPrevShader = paint->getShader();
        fPrevShader->safeRef();
        fPaint->setShader(SkShader::CreateBitmapShader( src, false, paint->getFilterType(),
                                                        SkShader::kClamp_TileMode, SkShader::kClamp_TileMode,
                                                        fStorage, sizeof(fStorage)));
    }
    ~SkAutoBitmapShaderInstall()
    {
        SkShader* shader = fPaint->getShader();

        fPaint->setShader(fPrevShader);
        fPrevShader->safeUnref();

        if ((void*)shader == (void*)fStorage)
            shader->~SkShader();
        else
            SkDELETE(shader);
    }
private:
    SkPaint*    fPaint;
    SkShader*   fPrevShader;
    uint32_t    fStorage[kBlitterStorageLongCount];
};

class SkAutoPaintStyleRestore {
public:
    SkAutoPaintStyleRestore(const SkPaint& paint, SkPaint::Style style)
        : fPaint((SkPaint&)paint)
    {
        fStyle = paint.getStyle();  // record the old
        fPaint.setStyle(style);     // change it to the specified style
    }
    ~SkAutoPaintStyleRestore()
    {
        fPaint.setStyle(fStyle);    // restore the old
    }
private:
    SkPaint&        fPaint;
    SkPaint::Style  fStyle;

    // illegal
    SkAutoPaintStyleRestore(const SkAutoPaintStyleRestore&);
    SkAutoPaintStyleRestore& operator=(const SkAutoPaintStyleRestore&);
};

///////////////////////////////////////////////////////////////////////////////

SkDraw::SkDraw(const SkCanvas& canvas)
{
    fDevice     = &canvas.getCurrBitmap();
    fMatrix     = &canvas.getTotalMatrix();
    fClip       = &canvas.getTotalClip();
    fBounder    = canvas.getBounder();
    fMapPtProc  = canvas.getCurrMapPtProc();

    SkDEBUGCODE(this->validate();)
}

void SkDraw::drawPaint(const SkPaint& paint)
{
    SkDEBUGCODE(this->validate();)

    if (fClip->isEmpty())
        return;

    SkRect16    devRect;
    devRect.set(0, 0, fDevice->width(), fDevice->height());

    if (fBounder && !fBounder->doIRect(devRect, *fClip))
        return;

    SkAutoBlitterChoose blitter(*fDevice, *fMatrix, paint);
    SkScan::FillDevRect(devRect, fClip, blitter.get());
}

void SkDraw::drawLine(const SkPoint& start, const SkPoint& stop, const SkPaint& paint)
{
    SkDEBUGCODE(this->validate();)

    if (fClip->isEmpty() ||
        (paint.getAlpha() == 0 && paint.getXfermode() == NULL)) // nothing to draw
        return;

    if (!paint.getPathEffect() && !paint.getMaskFilter() &&
        !paint.getRasterizer() && paint.getStrokeWidth() == 0)  // hairline
    {
        SkPoint pts[2];
        fMapPtProc(*fMatrix, start.fX, start.fY, &pts[0]);
        fMapPtProc(*fMatrix, stop.fX, stop.fY, &pts[1]);

        if (fBounder && !fBounder->doHairline(pts[0], pts[1], paint, *fClip))
            return;

        SkAutoBlitterChoose blitter(*fDevice, *fMatrix, paint);

        if (paint.isAntiAliasOn())
            SkScan::AntiHairLine(pts[0], pts[1], fClip, blitter.get());
        else
            SkScan::HairLine(pts[0], pts[1], fClip, blitter.get());
    }
    else
    {
        SkPath  path;
        // temporarily mark the paint as framing
        SkAutoPaintStyleRestore restore(paint, SkPaint::kStroke_Style);

        path.moveTo(start.fX, start.fY);
        path.lineTo(stop.fX, stop.fY);
        this->drawPath(path, paint);
    }
}

static inline SkPoint* as_lefttop(SkRect* r)
{
    return (SkPoint*)(void*)r;
}

static inline SkPoint* as_rightbottom(SkRect* r)
{
    return ((SkPoint*)(void*)r) + 1;
}

void SkDraw::drawRect(const SkRect& rect, const SkPaint& paint)
{
    SkDEBUGCODE(this->validate();)

    if (fClip->isEmpty() ||
        (paint.getAlpha() == 0 && paint.getXfermode() == NULL)) // nothing to draw
        return;

    if (paint.getPathEffect() || paint.getMaskFilter() || paint.getRasterizer() ||
        !fMatrix->rectStaysRect() || (paint.getStyle() != SkPaint::kFill_Style && SkScalarHalf(paint.getStrokeWidth()) > 0))
    {
        SkPath  tmp;
        tmp.addRect(rect);
        tmp.setFillType(SkPath::kWinding_FillType);
        this->drawPath(tmp, paint);
        return;
    }

    const SkMatrix&     matrix = *fMatrix;
    SkRect              devRect;

    // transform rect into devRect
    {
        SkMatrix::MapPtProc proc = fMapPtProc;
        proc(matrix, rect.fLeft, rect.fTop, &devRect.asPoints()[0]);
        proc(matrix, rect.fRight, rect.fBottom, &devRect.asPoints()[1]);
        devRect.sort();
    }

    if (fBounder && !fBounder->doRect(devRect, paint, *fClip))
        return;

    SkAutoBlitterChoose blitterStorage(*fDevice, matrix, paint);
    SkBlitter*          blitter = blitterStorage.get();
    const SkRegion*     clip = fClip;

    if (paint.getStyle() == SkPaint::kFill_Style)
        SkScan::FillRect(devRect, clip, blitter);
    else
    {
        if (paint.isAntiAliasOn())
            SkScan::AntiHairRect(devRect, clip, blitter);
        else
            SkScan::HairRect(devRect, clip, blitter);
    }
}

void SkDraw::drawDevMask(const SkMask& srcM, const SkPaint& paint)
{
    if (srcM.fBounds.isEmpty())
        return;

    SkMask          dstM;
    const SkMask*   mask = &srcM;

    dstM.fImage = NULL;
    SkAutoMaskImage ami(&dstM, false);

    if (paint.getMaskFilter() &&
        paint.getMaskFilter()->filterMask(&dstM, srcM, *fMatrix, NULL))
    {
        mask = &dstM;
    }

    if (fBounder && !fBounder->doIRect(mask->fBounds, *fClip))
        return;

    SkAutoBlitterChoose blitter(*fDevice, *fMatrix, paint);

    blitter->blitMaskRegion(*mask, *fClip);
}

void SkDraw::drawPath(const SkPath& origSrcPath, const SkPaint& paint, const SkMatrix* prePathMatrix, bool srcPathIsMutable)
{
    SkDEBUGCODE(this->validate();)

    if (fClip->isEmpty() ||
        (paint.getAlpha() == 0 && paint.getXfermode() == NULL)) // nothing to draw
        return;

    SkPath*         pathPtr = (SkPath*)&origSrcPath;
    bool            doFill = true;
    SkPath          tmpPath;
    SkMatrix        tmpMatrix;
    const SkMatrix* matrix = fMatrix;

    if (prePathMatrix)
    {
        if (paint.getPathEffect() || paint.getStyle() != SkPaint::kFill_Style || paint.getRasterizer())
        {
            SkPath* result = pathPtr;
    
            if (!srcPathIsMutable)
            {
                result = &tmpPath;
                srcPathIsMutable = true;
            }
            pathPtr->transform(*prePathMatrix, result);
            pathPtr = result;
        }
        else
        {
            tmpMatrix.setConcat(*matrix, *prePathMatrix);
            matrix = &tmpMatrix;
        }
    }
    // at this point we're done with prePathMatrix
    SkDEBUGCODE(prePathMatrix = (const SkMatrix*)0x50FF8001;)

    if (paint.getPathEffect() || paint.getStyle() != SkPaint::kFill_Style)
    {
        doFill = paint.getFillPath(*pathPtr, &tmpPath);
        pathPtr = &tmpPath;
    }

    if (paint.getRasterizer())
    {
        SkMask  mask;
        if (paint.getRasterizer()->rasterize(*pathPtr, *matrix, &fClip->getBounds(),
                                             paint.getMaskFilter(), &mask,
                                             SkMask::kComputeBoundsAndRenderImage_CreateMode))
        {
            this->drawDevMask(mask, paint);
            SkMask::FreeImage(mask.fImage);
        }
        return;
    }

    // avoid possibly allocating a new path in transform if we can
    SkPath* devPathPtr = srcPathIsMutable ? pathPtr : &tmpPath;

    // transform the path into device space
    if (!pathPtr->transform(*matrix, devPathPtr))
        return;

    SkAutoBlitterChoose blitter(*fDevice, *fMatrix, paint);

    // how does filterPath() know to fill or hairline the path??? <mrr>
    if (paint.getMaskFilter() &&
        paint.getMaskFilter()->filterPath(*devPathPtr, *fMatrix, *fClip, fBounder, blitter.get()))
    {
        return; // filterPath() called the blitter, so we're done
    }

    if (fBounder && !fBounder->doPath(*devPathPtr, paint, *fClip, doFill))
        return;

    if (doFill)
    {
        if (paint.isAntiAliasOn())
            SkScan::AntiFillPath(*devPathPtr, fClip, blitter.get());
        else
            SkScan::FillPath(*devPathPtr, fClip, blitter.get());
    }
    else    // hairline
    {
        if (paint.isAntiAliasOn())
            SkScan::AntiHairPath(*devPathPtr, fClip, blitter.get());
        else
            SkScan::HairPath(*devPathPtr, fClip, blitter.get());
    }
}

static inline bool just_translate(const SkMatrix& m)
{
    return (m.getType() & ~SkMatrix::kTranslate_Mask) == 0;
}

void SkDraw::drawBitmap(const SkBitmap& bitmap, SkScalar x, SkScalar y, const SkPaint& paint)
{
    SkDEBUGCODE(this->validate();)

    if (fClip->isEmpty() ||
        bitmap.width() == 0 || bitmap.height() == 0 || bitmap.getConfig() == SkBitmap::kNo_Config ||
        (paint.getAlpha() == 0 && paint.getXfermode() == NULL)) // nothing to draw
        return;

    SkAutoPaintStyleRestore restore(paint, SkPaint::kFill_Style);

    SkMatrix matrix = *fMatrix;
    matrix.preTranslate(x, y);

    if (NULL == paint.getColorFilter() && just_translate(matrix))
    {
        int         ix = SkScalarRound(matrix.getTranslateX());
        int         iy = SkScalarRound(matrix.getTranslateY());
        U32         storage[kBlitterStorageLongCount];
        SkBlitter*  blitter = SkBlitter::ChooseSprite(*fDevice, paint, bitmap, ix, iy, storage, sizeof(storage));
        if (blitter)
        {
            SkAutoTPlacementDelete<SkBlitter>   ad(blitter, storage);

            SkRect16    ir;
            ir.set(ix, iy, ix + bitmap.width(), iy + bitmap.height());

            if (fBounder && !fBounder->doIRect(ir, *fClip))
                return;

            SkRegion::Cliperator iter(*fClip, ir);
            const SkRect16&      cr = iter.rect();

            for (; !iter.done(); iter.next())
            {
                SkASSERT(!cr.isEmpty());
            #if 0
                LOGI("blitRect(%d %d %d %d) [%d %d %p]\n", cr.fLeft, cr.fTop, cr.width(), cr.height(),
                        bitmap.width(), bitmap.height(), bitmap.getPixels());
            #endif
                blitter->blitRect(cr.fLeft, cr.fTop, cr.width(), cr.height());
            }
            return;
        }
    }

    SkAutoBitmapShaderInstall   install(bitmap, &paint);

    // save our state
    const SkMatrix*     saveMatrix = fMatrix;
    SkMatrix::MapPtProc saveProc = fMapPtProc;

    // jam in the new temp state
    fMatrix = &matrix;
    fMapPtProc = matrix.getMapPtProc();

    // call ourself with a rect
    {
        SkRect  r;
        r.set(0, 0, SkIntToScalar(bitmap.width()), SkIntToScalar(bitmap.height()));
        // is this ok if paint has a rasterizer? <reed>
        this->drawRect(r, paint);
    }
    
    // restore our state
    fMapPtProc = saveProc;
    fMatrix = saveMatrix;
}

void SkDraw::drawSprite(const SkBitmap& bitmap, int x, int y, const SkPaint& paint)
{
    SkDEBUGCODE(this->validate();)

    SkRect16    bounds;
    bounds.set(x, y, x + bitmap.width(), y + bitmap.height());

    if (fClip->quickReject(bounds) ||
        bitmap.getConfig() == SkBitmap::kNo_Config ||
        (paint.getAlpha() == 0 && paint.getXfermode() == NULL))
    {
        return; // nothing to draw
    }

    SkAutoPaintStyleRestore restore(paint, SkPaint::kFill_Style);

    if (NULL == paint.getColorFilter())
    {
        uint32_t    storage[kBlitterStorageLongCount];
        SkBlitter*  blitter = SkBlitter::ChooseSprite(*fDevice, paint, bitmap, x, y, storage, sizeof(storage));

        if (blitter)
        {
            SkAutoTPlacementDelete<SkBlitter>   ad(blitter, storage);

            if (fBounder && !fBounder->doIRect(bounds, *fClip))
                return;

            SkRegion::Cliperator iter(*fClip, bounds);
            const SkRect16&      cr = iter.rect();

            for (; !iter.done(); iter.next())
            {
                SkASSERT(!cr.isEmpty());
                blitter->blitRect(cr.fLeft, cr.fTop, cr.width(), cr.height());
            }
            return;
        }
    }

    SkAutoBitmapShaderInstall   install(bitmap, &paint);

    // save our state
    const SkMatrix*     saveMatrix = fMatrix;
    SkMatrix::MapPtProc saveProc = fMapPtProc;
    SkMatrix            matrix;
    SkRect              r;

    // get a scalar version of our rect
    r.set(bounds);

    // tell the shader our offset
    matrix.setTranslate(r.fLeft, r.fTop);
    paint.getShader()->setLocalMatrix(matrix);

    // jam in the new temp state
    matrix.reset();
    fMatrix = &matrix;
    fMapPtProc = matrix.getMapPtProc();

    // call ourself with a rect
    {
        // is this OK if paint has a rasterizer? <reed>
        this->drawRect(r, paint);
    }
    
    // restore our state
    fMapPtProc = saveProc;
    fMatrix = saveMatrix;
}

///////////////////////////////////////////////////////////////////////////////////

#include "SkScalerContext.h"
#include "SkGlyphCache.h"
#include "SkUtils.h"

static void measure_text(SkGlyphCache* cache, SkGlyphCacheProc glyphCacheProc,
                         const char text[], size_t byteLength, SkVector* stopVector)
{
    SkFixed     x = 0, y = 0;
    const char* stop = text + byteLength;

    while (text < stop)
    {
        const SkGlyph& glyph = glyphCacheProc(cache, &text);
        x += glyph.fAdvanceX;
        y += glyph.fAdvanceY;
    }
    stopVector->set(SkFixedToScalar(x), SkFixedToScalar(y));

    SkASSERT(text == stop);
}

void SkDraw::drawText_asPaths(const char text[], size_t byteLength, SkScalar x, SkScalar y, const SkPaint& paint)
{
    SkDEBUGCODE(this->validate();)

    SkTextToPathIter    iter(text, byteLength, paint, true, true);

    SkMatrix    matrix;
    matrix.setScale(iter.getPathScale(), iter.getPathScale());
    matrix.postTranslate(x, y);

    const SkPath* iterPath;
    SkScalar xpos, prevXPos = 0;

    while ((iterPath = iter.next(&xpos)) != NULL)
    {
        matrix.postTranslate(xpos - prevXPos, 0);
        this->drawPath(*iterPath, iter.getPaint(), &matrix, false);
        prevXPos = xpos;
    }
}

#define kStdStrikeThru_Offset       (-SK_Scalar1 * 6 / 21)
#define kStdUnderline_Offset        (SK_Scalar1 / 9)
#define kStdUnderline_Thickness     (SK_Scalar1 / 18)

static void draw_paint_rect(SkDraw* draw, const SkPaint& paint, const SkRect& r, SkScalar textSize)
{
    if (paint.getStyle() == SkPaint::kFill_Style)
        draw->drawRect(r, paint);
    else
    {
        SkPaint p(paint);
        p.setStrokeWidth(SkScalarMul(textSize, paint.getStrokeWidth()));
        draw->drawRect(r, p);
    }
}

static void handle_aftertext(SkDraw* draw, const SkPaint& paint, SkScalar width, const SkPoint& start)
{
    U32 flags = paint.getFlags();

    if (flags & (SkPaint::kUnderlineText_Flag | SkPaint::kStrikeThruText_Flag))
    {
        SkScalar textSize = paint.getTextSize();
        SkScalar height = SkScalarMul(textSize, kStdUnderline_Thickness);
        SkRect   r;

        r.fLeft = start.fX;
        r.fRight = start.fX + width;

        if (flags & SkPaint::kUnderlineText_Flag)
        {
            SkScalar offset = start.fY + SkScalarMul(textSize, kStdUnderline_Offset);
            r.fTop = offset;
            r.fBottom = offset + height;
            draw_paint_rect(draw, paint, r, textSize);
        }
        if (flags & SkPaint::kStrikeThruText_Flag)
        {
            SkScalar offset = start.fY + SkScalarMul(textSize, kStdStrikeThru_Offset);
            r.fTop = offset;
            r.fBottom = offset + height;
            draw_paint_rect(draw, paint, r, textSize);
        }
    }
}

#if defined _WIN32 && _MSC_VER >= 1300  // disable warning : local variable used without having been initialized
#pragma warning ( push )
#pragma warning ( disable : 4701 )
#endif

static inline void draw_one_glyph(const SkGlyph& glyph, int left, int top, SkBounder* bounder,
                                  const SkRegion& clip, SkBlitter* blitter, SkGlyphCache* cache)
{
    SkMask  mask;

    int right   = left + glyph.fWidth;
    int bottom  = top + glyph.fHeight;

    mask.fBounds.set(left, top, right, bottom);

    if (bounder == NULL && clip.quickContains(left, top, right, bottom))
    {
        uint8_t* aa = (uint8_t*)glyph.fImage;               
        if (aa == NULL)
            aa = (uint8_t*)cache->findImage(glyph);

        if (aa)
        {
            mask.fRowBytes = glyph.fRowBytes;
            mask.fFormat = glyph.fMaskFormat;
            mask.fImage = aa;
            blitter->blitMask(mask, mask.fBounds);
        }
    }
    else
    {
        SkRegion::Cliperator clipper(clip, mask.fBounds);
        if (!clipper.done())
        {
            const SkRect16& cr = clipper.rect();
            const uint8_t*  aa = (const uint8_t*)glyph.fImage;
            if (NULL == aa)
                aa = (const uint8_t*)cache->findImage(glyph);

            if (aa && (bounder == NULL || bounder->doIRect(cr, clip)))
            {
                mask.fRowBytes = glyph.fRowBytes;
                mask.fFormat = glyph.fMaskFormat;
                mask.fImage = (uint8_t*)aa;
                do {
                    blitter->blitMask(mask, cr);
                    clipper.next();
                } while (!clipper.done());
            }
        }
    }
}

void SkDraw::drawText(const char text[], size_t byteLength, SkScalar x, SkScalar y, const SkPaint& paint)
{
    SkASSERT(byteLength == 0 || text != NULL);

    SkDEBUGCODE(this->validate();)

    if (text == NULL || byteLength == 0 ||
        fClip->isEmpty() ||
        (paint.getAlpha() == 0 && paint.getXfermode() == NULL)) // nothing to draw
        return;

    SkScalar    underlineWidth = 0;
    SkPoint     underlineStart;

    underlineStart.set(0, 0);    // to avoid warning
    if (paint.getFlags() & (SkPaint::kUnderlineText_Flag | SkPaint::kStrikeThruText_Flag))
    {
        underlineWidth = paint.measureText(text, byteLength, NULL, NULL);

        SkScalar offsetX = 0;
        if (paint.getTextAlign() == SkPaint::kCenter_Align)
            offsetX = SkScalarHalf(underlineWidth);
        else if (paint.getTextAlign() == SkPaint::kRight_Align)
            offsetX = underlineWidth;

        underlineStart.set(x - offsetX, y);
    }

    if (paint.isLinearTextOn() ||
        (fMatrix->getType() & SkMatrix::kPerspective_Mask))
    {
        this->drawText_asPaths(text, byteLength, x, y, paint);
        handle_aftertext(this, paint, underlineWidth, underlineStart);
        return;
    }

    SkGlyphCacheProc glyphCacheProc = paint.getGlyphCacheProc();

    SkAutoGlyphCache    autoCache(paint, fMatrix);
    SkGlyphCache*       cache = autoCache.getCache();
    SkAutoBlitterChoose blitter(*fDevice, *fMatrix, paint);
    
    // transform our starting point
    {
        SkPoint loc;
        fMapPtProc(*fMatrix, x, y, &loc);
        x = loc.fX;
        y = loc.fY;
    }

    if (paint.getTextAlign() != SkPaint::kLeft_Align)   // need to measure first
    {
        SkVector    stop;

        measure_text(cache, glyphCacheProc, text, byteLength, &stop);

        SkScalar    stopX = stop.fX;
        SkScalar    stopY = stop.fY;

        if (paint.getTextAlign() == SkPaint::kCenter_Align)
        {
            stopX = SkScalarHalf(stopX);
            stopY = SkScalarHalf(stopY);
        }
        x -= stopX;
        y -= stopY;
    }
    
    // add a half now so we can trunc rather than round in the loop
    SkFixed fx = SkScalarToFixed(x) + SK_FixedHalf;
    SkFixed fy = SkScalarToFixed(y) + SK_FixedHalf;
    const char* stop = text + byteLength;
    const SkRegion& clip = *fClip;
    SkBounder* bounder = fBounder;
    SkBlitter* blit = blitter.get();

    while (text < stop)
    {
        const SkGlyph& glyph = glyphCacheProc(cache, &text);

        if (glyph.fWidth) {
            draw_one_glyph( glyph,
                            SkFixedFloor(fx) + glyph.fLeft, SkFixedFloor(fy) + glyph.fTop,
                            bounder, clip, blit, cache);
        }
        fx += glyph.fAdvanceX;
        fy += glyph.fAdvanceY;
    }

    if (underlineWidth)
    {
        autoCache.release();    // release this now to free up the RAM
        handle_aftertext(this, paint, underlineWidth, underlineStart);
    }
}

typedef void (*AlignProc)(const SkPoint&, const SkGlyph&, SkPoint16*);

static void leftAlignProc(const SkPoint& loc, const SkGlyph& glyph, SkPoint16* dst)
{
    dst->set(SkScalarRound(loc.fX) + glyph.fLeft,
             SkScalarRound(loc.fY) + glyph.fTop);
}

static void centerAlignProc(const SkPoint& loc, const SkGlyph& glyph, SkPoint16* dst)
{
    dst->set(SkFixedRound(SkScalarToFixed(loc.fX) - (glyph.fAdvanceX >> 1)) + glyph.fLeft,
             SkFixedRound(SkScalarToFixed(loc.fY) - (glyph.fAdvanceY >> 1)) + glyph.fTop);
}

static void rightAlignProc(const SkPoint& loc, const SkGlyph& glyph, SkPoint16* dst)
{
    dst->set(SkFixedRound(SkScalarToFixed(loc.fX) - glyph.fAdvanceX) + glyph.fLeft,
             SkFixedRound(SkScalarToFixed(loc.fY) - glyph.fAdvanceY) + glyph.fTop);
}

static AlignProc pick_align_proc(SkPaint::Align align)
{
    static const AlignProc gProcs[] = { leftAlignProc, centerAlignProc, rightAlignProc };
    
    SkASSERT((unsigned)align < SK_ARRAY_COUNT(gProcs));

    return gProcs[align];
}

void SkDraw::drawPosText(const char text[], size_t byteLength,
                         const SkPoint pos[], const SkPaint& paint)
{
    SkASSERT(byteLength == 0 || text != NULL);

    SkDEBUGCODE(this->validate();)

    if (text == NULL || byteLength == 0 ||
        fClip->isEmpty() ||
        (paint.getAlpha() == 0 && paint.getXfermode() == NULL)) // nothing to draw
        return;

    if (paint.isLinearTextOn() ||
        (fMatrix->getType() & SkMatrix::kPerspective_Mask))
    {
        // TODO !!!!
//      this->drawText_asPaths(text, byteLength, x, y, paint);
        return;
    }

    SkGlyphCacheProc    glyphCacheProc = paint.getGlyphCacheProc();
    SkAutoGlyphCache    autoCache(paint, fMatrix);
    SkGlyphCache*       cache = autoCache.getCache();
    SkAutoBlitterChoose blitter(*fDevice, *fMatrix, paint);
    
    const char*         stop = text + byteLength;
    const SkRegion&     clip = *fClip;
    SkBounder*          bounder = fBounder;
    SkBlitter*          blit = blitter.get();
    SkMatrix::MapPtProc mapPtProc = fMapPtProc;
    const SkMatrix&     matrix = *fMatrix;
    AlignProc           alignProc = pick_align_proc(paint.getTextAlign());

    while (text < stop)
    {
        const SkGlyph& glyph = glyphCacheProc(cache, &text);

        if (glyph.fWidth)
        {
            SkPoint loc;
            mapPtProc(matrix, pos->fX, pos->fY, &loc);

            SkPoint16   devLoc;
            alignProc(loc, glyph, &devLoc);

            draw_one_glyph( glyph, devLoc.fX, devLoc.fY,
                            bounder, clip, blit, cache);
        }
        pos += 1;
    }
}

#if defined _WIN32 && _MSC_VER >= 1300
#pragma warning ( pop )
#endif

////////////////////////////////////////////////////////////////////////////////////////////

#include "SkPathMeasure.h"

static void morphpoints(SkPoint dst[], const SkPoint src[], int count,
                        SkPathMeasure& meas, SkScalar offset, SkScalar scale)
{
    for (int i = 0; i < count; i++)
    {
        SkPoint pos;
        SkVector tangent;

        SkScalar sx = SkScalarMul(src[i].fX, scale) + offset;
        SkScalar sy = SkScalarMul(src[i].fY, scale);

        meas.getPosTan(sx, &pos, &tangent);

        SkMatrix    matrix;
        SkPoint     pt;

        pt.set(sx, sy);
        matrix.setSinCos(tangent.fY, tangent.fX);
        matrix.preTranslate(-sx, 0);
        matrix.postTranslate(pos.fX, pos.fY);
        matrix.mapPoints(&dst[i], &pt, 1);
    }
}

/*  TODO

    Need differentially more subdivisions when the follow-path is curvy. Not sure how to
    determine that, but we need it. I guess a cheap answer is let the caller tell us,
    but that seems like a cop-out. Another answer is to get Rob Johnson to figure it out.
*/
static void morphpath(SkPath* dst, const SkPath& src, SkPathMeasure& meas,
                      SkScalar offset, SkScalar scale)
{
    SkPath::Iter    iter(src, false);
    SkPoint         srcP[4], dstP[3];
    SkPath::Verb    verb;

    while ((verb = iter.next(srcP)) != SkPath::kDone_Verb)
    {
        switch (verb) {
        case SkPath::kMove_Verb:
            morphpoints(dstP, srcP, 1, meas, offset, scale);
            dst->moveTo(dstP[0]);
            break;
        case SkPath::kLine_Verb:
            srcP[2] = srcP[1];
            srcP[1].set(SkScalarAve(srcP[0].fX, srcP[2].fX),
                        SkScalarAve(srcP[0].fY, srcP[2].fY));
            // fall through to quad
        case SkPath::kQuad_Verb:
            morphpoints(dstP, &srcP[1], 2, meas, offset, scale);
            dst->quadTo(dstP[0], dstP[1]);
            break;
        case SkPath::kCubic_Verb:
            morphpoints(dstP, &srcP[1], 3, meas, offset, scale);
            dst->cubicTo(dstP[0], dstP[1], dstP[2]);
            break;
        case SkPath::kClose_Verb:
            dst->close();
            break;
        default:
            SkASSERT(!"unknown verb");
            break;
        }
    }
}

void SkDraw::drawTextOnPath(const char text[], size_t byteLength,
                            const SkPath& follow, SkScalar offset, const SkPaint& paint)
{
    SkASSERT(byteLength == 0 || text != NULL);

    if (text == NULL || byteLength == 0 ||
        fClip->getBounds().isEmpty() ||
        (paint.getAlpha() == 0 && paint.getXfermode() == NULL)) // nothing to draw
        return;

    SkTextToPathIter    iter(text, byteLength, paint, true, true);
    SkPathMeasure       meas(follow, false);

    if (paint.getTextAlign() != SkPaint::kLeft_Align)   // need to measure first
    {
        SkScalar pathLen = meas.getLength();
        if (paint.getTextAlign() == SkPaint::kCenter_Align)
            pathLen = SkScalarHalf(pathLen);
        offset += pathLen;
    }

    const SkPath*   iterPath;
    SkScalar        xpos;
    while ((iterPath = iter.next(&xpos)) != NULL)
    {
        SkPath  tmp;
        morphpath(&tmp, *iterPath, meas, offset + xpos, iter.getPathScale());
        this->drawPath(tmp, iter.getPaint());
    }
}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG

void SkDraw::validate() const
{
    SkASSERT(fDevice != NULL);
    SkASSERT(fMatrix != NULL);
    SkASSERT(fClip != NULL);

    const SkRect16& cr = fClip->getBounds();
    SkRect16        br;

    br.set(0, 0, fDevice->width(), fDevice->height());
    SkASSERT(br.contains(cr));
}

#endif

//////////////////////////////////////////////////////////////////////////////////////////

bool SkBounder::doIRect(const SkRect16& r, const SkRegion& clip)
{
    SkRect16    rr;
    return rr.intersect(clip.getBounds(), r) && this->onIRect(rr);
}

bool SkBounder::doHairline(const SkPoint& pt0, const SkPoint& pt1, const SkPaint& paint, const SkRegion& clip)
{
    SkRect16    r;
    SkScalar    v0, v1;

    v0 = pt0.fX;
    v1 = pt1.fX;
    if (v0 > v1)
        SkTSwap<SkScalar>(v0, v1);
    r.fLeft     = SkToS16(SkScalarFloor(v0));
    r.fRight    = SkToS16(SkScalarCeil(v1));

    v0 = pt0.fY;
    v1 = pt1.fY;
    if (v0 > v1)
        SkTSwap<SkScalar>(v0, v1);
    r.fTop      = SkToS16(SkScalarFloor(v0));
    r.fBottom   = SkToS16(SkScalarCeil(v1));

    if (paint.isAntiAliasOn())
        r.inset(-1, -1);

    return this->doIRect(r, clip);
}

bool SkBounder::doRect(const SkRect& rect, const SkPaint& paint, const SkRegion& clip)
{
    SkRect16    r;

    if (paint.getStyle() == SkPaint::kFill_Style)
        rect.round(&r);
    else
    {
        int rad = -1;
        rect.roundOut(&r);
        if (paint.isAntiAliasOn())
            rad = -2;
        r.inset(rad, rad);
    }
    return this->doIRect(r, clip);
}

bool SkBounder::doPath(const SkPath& path, const SkPaint& paint, const SkRegion& clip, bool doFill)
{
    SkRect      bounds;
    SkRect16    r;

    path.computeBounds(&bounds, SkPath::kFast_BoundsType);

    if (doFill)
        bounds.round(&r);
    else    // hairline
        bounds.roundOut(&r);

    if (paint.isAntiAliasOn())
        r.inset(-1, -1);

    return this->doIRect(r, clip);
}

void SkBounder::commit()
{
    // override in subclass
}

////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkPath.h"
#include "SkDraw.h"
#include "SkRegion.h"
#include "SkBlitter.h"

static bool compute_bounds(const SkPath& devPath, const SkRect16* clipBounds,
                           SkMaskFilter* filter, const SkMatrix* filterMatrix,
                           SkRect16* bounds)
{
    if (devPath.isEmpty())
        return false;

    SkPoint16   margin;
    margin.set(0, 0);

    //  init our bounds from the path
    {
        SkRect      pathBounds;
        devPath.computeBounds(&pathBounds, SkPath::kExact_BoundsType);
        pathBounds.inset(-SK_ScalarHalf, -SK_ScalarHalf);
        pathBounds.roundOut(bounds);
    }
    
    if (filter)
    {
        SkASSERT(filterMatrix);
        
        SkMask  srcM, dstM;
        
        srcM.fBounds = *bounds;
        srcM.fFormat = SkMask::kA8_Format;
        srcM.fImage = NULL;
        if (!filter->filterMask(&dstM, srcM, *filterMatrix, &margin))
            return false;
        
        *bounds = dstM.fBounds;
    }

    if (clipBounds && !SkRect16::Intersects(*clipBounds, *bounds))
        return false;
    
    // (possibly) trim the srcM bounds to reflect the clip
    // (plus whatever slop the filter needs)
    if (clipBounds && !clipBounds->contains(*bounds))
    {
        SkRect16 tmp = *bounds;
        (void)tmp.intersect(*clipBounds);
        tmp.inset(-margin.fX, -margin.fY);
        (void)bounds->intersect(tmp);
    }

    return true;
}

static void draw_into_mask(const SkMask& mask, const SkPath& devPath)
{
    SkBitmap    bm;
    SkDraw      draw;
    SkRegion    clipRgn;
    SkMatrix    matrix;
    SkPaint     paint;

    bm.setConfig(SkBitmap::kA8_Config, mask.fBounds.width(), mask.fBounds.height(), mask.fRowBytes);
    bm.setPixels(mask.fImage);

    clipRgn.setRect(0, 0, mask.fBounds.width(), mask.fBounds.height());
    matrix.setTranslate(-SkIntToScalar(mask.fBounds.fLeft),
                        -SkIntToScalar(mask.fBounds.fTop));

    draw.fDevice    = &bm;
    draw.fClip      = &clipRgn;
    draw.fMatrix    = &matrix;
    draw.fMapPtProc = matrix.getMapPtProc();
    draw.fBounder   = NULL;
    paint.setAntiAliasOn(true);
    draw.drawPath(devPath, paint);
}

bool SkDraw::DrawToMask(const SkPath& devPath, const SkRect16* clipBounds,
                        SkMaskFilter* filter, const SkMatrix* filterMatrix,
                        SkMask* mask, SkMask::CreateMode mode)
{
    if (SkMask::kJustRenderImage_CreateMode != mode)
    {
        if (!compute_bounds(devPath, clipBounds, filter, filterMatrix, &mask->fBounds))
            return false;
    }
    
    if (SkMask::kComputeBoundsAndRenderImage_CreateMode == mode)
    {
        mask->fFormat = SkMask::kA8_Format;
        mask->fRowBytes = mask->fBounds.width();
        mask->fImage = SkMask::AllocImage(mask->computeImageSize());
        memset(mask->fImage, 0, mask->computeImageSize());
    }

    if (SkMask::kJustComputeBounds_CreateMode != mode)
        draw_into_mask(*mask, devPath);

    return true;
}

