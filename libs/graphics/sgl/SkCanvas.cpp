#include "SkCanvas.h"
#include "SkDraw.h"
#include "SkBounder.h"
#include "SkUtils.h"

struct MCRecLayer {
    SkBitmap    fBitmap;
    int         fX, fY;
    SkPaint     fPaint;

    MCRecLayer(const SkPaint& paint) : fPaint(paint) {}
};

struct SkCanvas::MCRec {
    MCRec*              fNext;

    SkMatrix            fMatrix;
    SkMatrix::MapPtProc	fMapPtProc;
    SkRegion            fRegion;

    MCRecLayer*     fLayer;         // may be NULL
    const SkBitmap* fCurrBitmap;    // points to layer or prevLayer or pixels

    uint8_t	fSetPaintBits;
    uint8_t fClearPaintBits;
    
    MCRec() : fLayer(NULL)
    {
    }
    MCRec(const MCRec& other)
        : fMatrix(other.fMatrix), fRegion(other.fRegion), fLayer(NULL)
    {
        // don't bother initializing fNext
        fMapPtProc      = other.fMapPtProc;
        fCurrBitmap     = other.fCurrBitmap;
        fSetPaintBits   = other.fSetPaintBits;
        fClearPaintBits = other.fClearPaintBits;
    }
    ~MCRec()
    {
        SkDELETE(fLayer);
    }
};

class AutoPaintSetClear {
public:
	AutoPaintSetClear(const SkPaint& paint, U32 setBits, U32 clearBits) : fPaint(paint)
	{
		fFlags = paint.getFlags();
		((SkPaint*)&paint)->setFlags((fFlags | setBits) & ~clearBits);
	}
	~AutoPaintSetClear()
	{
		((SkPaint*)&fPaint)->setFlags(fFlags);
	}
private:
	const SkPaint&	fPaint;
	U32				fFlags;

	// illegal
	AutoPaintSetClear(const AutoPaintSetClear&);
	AutoPaintSetClear& operator=(const AutoPaintSetClear&);
};

class SkAutoBounderCommit {
public:
	SkAutoBounderCommit(SkBounder* bounder) : fBounder(bounder) {}
	~SkAutoBounderCommit() { if (fBounder) fBounder->commit(); }
private:
	SkBounder*	fBounder;
};

////////////////////////////////////////////////////////////////////////////

SkCanvas::SkCanvas()
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage)), fBounder(NULL)
{
    fMCRec = (MCRec*)fMCStack.push_back();
    new (fMCRec) MCRec;

	fMCRec->fNext = NULL;
	fMCRec->fMatrix.reset();
	fMCRec->fSetPaintBits = 0;
	fMCRec->fClearPaintBits = 0;
	fMCRec->fMapPtProc = NULL;	// mark as dirty/unknown

    fMCRec->fLayer      = NULL;
    fMCRec->fCurrBitmap = &fBitmap;
}

SkCanvas::SkCanvas(const SkBitmap& bitmap)
    : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage)), fBitmap(bitmap), fBounder(NULL)
{
    fMCRec = (MCRec*)fMCStack.push_back();
    new (fMCRec) MCRec;
    
	fMCRec->fNext = NULL;
	fMCRec->fMatrix.reset();
	fMCRec->fSetPaintBits = 0;
	fMCRec->fClearPaintBits = 0;
	fMCRec->fMapPtProc = NULL;	// mark as dirty/unknown
    
    fMCRec->fRegion.setRect(0, 0, bitmap.width(), bitmap.height());

    fMCRec->fLayer      = NULL;
    fMCRec->fCurrBitmap = &fBitmap;
}

SkCanvas::~SkCanvas()
{
}

SkBounder* SkCanvas::setBounder(SkBounder* bounder)
{
	SkRefCnt_SafeAssign(fBounder, bounder);
	return bounder;
}

void SkCanvas::getPixels(SkBitmap* bitmap) const
{
	if (bitmap)
		*bitmap = fBitmap;
}

void SkCanvas::setPixels(const SkBitmap& bitmap)
{
	unsigned	prevWidth = fBitmap.width();
	unsigned	prevHeight = fBitmap.height();

	fBitmap = bitmap;

	/*	Now we update our initial region to have the bounds of the new bitmap,
		and then intersect all of the clips in our stack with these bounds,
		to ensure that we can't draw outside of the bitmap's bounds (and trash
		memory).

		NOTE: this is only a partial-fix, since if the new bitmap is larger than
		the previous one, we don't know how to "enlarge" the clips in our stack,
		so drawing may be artificially restricted. Without keeping a history of 
		all calls to canvas->clipRect() and canvas->clipPath(), we can't exactly
		reconstruct the correct clips, so this approximation will have to do.
		The caller really needs to restore() back to the base if they want to
		accurately take advantage of the new bitmap bounds.
	*/

	if (prevWidth != bitmap.width() || prevHeight != bitmap.height())
	{
		SkRect16	r;
		r.set(0, 0, bitmap.width(), bitmap.height());

        SkDeque::Iter   iter(fMCStack);
        MCRec*          rec = (MCRec*)iter.next();
        
        SkASSERT(rec);
		rec->fRegion.setRect(r);

        while ((rec = (MCRec*)iter.next()) != NULL)
			(void)rec->fRegion.op(r, SkRegion::kIntersect_Op);
	}
}

bool SkCanvas::isBitmapOpaque() const
{
    SkBitmap::Config c = fBitmap.getConfig();
    
    return c != SkBitmap::kA8_Config && c != SkBitmap::kARGB_8888_Config;
}

///////////////////////////////////////////////////////////////////////////////////////////

U32 SkCanvas::getPaintSetBits() const
{
	return fMCRec->fSetPaintBits;
}

U32 SkCanvas::getPaintClearBits() const
{
	return fMCRec->fClearPaintBits;
}

void SkCanvas::setPaintSetClearBits(U32 setBits, U32 clearBits)
{
	fMCRec->fSetPaintBits = SkToU8(setBits & SkPaint::kAllFlagMasks);
	fMCRec->fClearPaintBits = SkToU8(clearBits & SkPaint::kAllFlagMasks);
}

void SkCanvas::orPaintSetClearBits(U32 setBits, U32 clearBits)
{
	fMCRec->fSetPaintBits |= setBits;
	fMCRec->fClearPaintBits |= clearBits;
}

/////////////////////////////////////////////////////////////////////////////

int SkCanvas::save()
{
    int saveCount = this->getSaveCount(); // record this before the actual save

    MCRec* newTop = (MCRec*)fMCStack.push_back();
    new (newTop) MCRec(*fMCRec);

    newTop->fNext = fMCRec;
    fMCRec = newTop;

    return saveCount;
}

int SkCanvas::saveLayer(const SkRect& bounds, const SkPaint& paint)
{
    // do this before we create the layer
    int count = this->save();

    SkRect      r;
    SkRect16    ir;
    
    fMCRec->fMatrix.mapRect(&r, bounds);
    r.roundOut(&ir);
    
    if (ir.intersect(fMCRec->fRegion.getBounds()))
    {
        MCRecLayer* layer = SkNEW_ARGS(MCRecLayer, (paint));

        layer->fBitmap.setConfig(SkBitmap::kARGB_8888_Config, ir.width(), ir.height());
        layer->fBitmap.allocPixels();
        layer->fBitmap.eraseARGB(0, 0, 0, 0);
        layer->fX = ir.fLeft;
        layer->fY = ir.fTop;
        
        fMCRec->fLayer = layer;
        fMCRec->fCurrBitmap = &layer->fBitmap;

        fMCRec->fMatrix.postTranslate(-SkIntToScalar(ir.fLeft), -SkIntToScalar(ir.fTop));
        fMCRec->fMapPtProc = NULL;

        fMCRec->fRegion.op(ir, SkRegion::kIntersect_Op);
        fMCRec->fRegion.translate(-ir.fLeft, -ir.fTop);
    }
    return count;
}

#include "SkTemplates.h"

void SkCanvas::restore()
{
	SkASSERT(!fMCStack.empty());

    MCRecLayer*                 layer = fMCRec->fLayer;
    SkAutoTDelete<MCRecLayer>   ad(layer);
    // now detach it from fMCRec
    fMCRec->fLayer = NULL;

    // now do the normal restore()
    fMCStack.pop_back();
    fMCRec = (MCRec*)fMCStack.back();

    // now handle the layer if needed
    if (layer)
        this->drawSprite(layer->fBitmap, layer->fX, layer->fY, layer->fPaint);
}

int SkCanvas::getSaveCount() const
{
	return fMCStack.count();
}

void SkCanvas::restoreToCount(int count)
{
	SkASSERT(fMCStack.count() >= count);

	while (fMCStack.count() > count)
		this->restore();
}

void SkCanvas::clipDeviceRgn(const SkRegion& rgn)
{
	fMCRec->fRegion.op(rgn, SkRegion::kIntersect_Op);
}

/////////////////////////////////////////////////////////////////////////////

bool SkCanvas::translate(SkScalar dx, SkScalar dy)
{
	fMCRec->fMapPtProc = NULL;	// mark as dirty/unknown
	return fMCRec->fMatrix.preTranslate(dx, dy);
}

bool SkCanvas::scale(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py)
{
	fMCRec->fMapPtProc = NULL;	// mark as dirty/unknown
	return fMCRec->fMatrix.preScale(sx, sy, px, py);
}

bool SkCanvas::rotate(SkScalar degrees, SkScalar px, SkScalar py)
{
	fMCRec->fMapPtProc = NULL;	// mark as dirty/unknown
	return fMCRec->fMatrix.preRotate(degrees, px, py);
}

bool SkCanvas::skew(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py)
{
	fMCRec->fMapPtProc = NULL;	// mark as dirty/unknown
	return fMCRec->fMatrix.preSkew(sx, sy, px, py);
}

bool SkCanvas::concat(const SkMatrix& matrix)
{
	fMCRec->fMapPtProc = NULL;	// mark as dirty/unknown
	return fMCRec->fMatrix.preConcat(matrix);
}

//////////////////////////////////////////////////////////////////////////////

void SkCanvas::clipRect(const SkRect& rect)
{
	if (fMCRec->fMatrix.rectStaysRect())
	{
		SkRect		r;
		SkRect16	ir;

		fMCRec->fMatrix.mapRect(&r, rect);
		r.round(&ir);
		fMCRec->fRegion.op(ir, SkRegion::kIntersect_Op);
	}
	else
	{
		SkPath	path;

		path.addRect(rect);
		this->clipPath(path);
	}
}

void SkCanvas::clipRect(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom)
{
    SkRect  r;
    
    r.set(left, top, right, bottom);
    this->clipRect(r);
}

void SkCanvas::clipPath(const SkPath& path)
{
	SkPath	devPath;

	path.transform(fMCRec->fMatrix, &devPath);
	fMCRec->fRegion.setPath(devPath, &fMCRec->fRegion);
}

bool SkCanvas::quickReject(const SkRect& rect, bool antialiased) const
{
	if (fMCRec->fRegion.isEmpty() || rect.isEmpty())
		return true;

	SkRect		r;
	SkRect16	ir;

	fMCRec->fMatrix.mapRect(&r, rect);
    if (antialiased)
        r.roundOut(&ir);
    else
        r.round(&ir);

	return fMCRec->fRegion.quickReject(ir);
}

bool SkCanvas::quickReject(const SkPath& path, bool antialiased) const
{
	if (fMCRec->fRegion.isEmpty() || path.isEmpty())
		return true;        

    if (fMCRec->fMatrix.rectStaysRect())
    {
        SkRect  r;
        path.computeBounds(&r, SkPath::kExact_BoundsType);
        return this->quickReject(r, antialiased);
    }

    SkPath      dstPath;
	SkRect		r;
	SkRect16	ir;

    path.transform(fMCRec->fMatrix, &dstPath);
    dstPath.computeBounds(&r, SkPath::kExact_BoundsType);
    if (antialiased)
        r.roundOut(&ir);
    else
        r.round(&ir);

	return fMCRec->fRegion.quickReject(ir);
}

//////////////////////////////////////////////////////////////////////////////

void SkCanvas::drawRGB(U8CPU r, U8CPU g, U8CPU b)
{
	SkPaint	paint;

	paint.setARGB(0xFF, r, g, b);
	this->drawPaint(paint);
}

void SkCanvas::drawARGB(U8CPU a, U8CPU r, U8CPU g, U8CPU b)
{
	SkPaint	paint;

	paint.setARGB(a, r, g, b);
	this->drawPaint(paint);
}

void SkCanvas::drawColor(SkColor c, SkPorterDuff::Mode mode)
{
	SkPaint	paint;

	paint.setColor(c);
    paint.setPorterDuffXfermode(mode);
	this->drawPaint(paint);
}

void SkCanvas::drawPaint(const SkPaint& paint)
{
	SkAutoBounderCommit	ac(fBounder);
	SkDraw				draw(*this);

	draw.drawPaint(paint);
}

void SkCanvas::drawLine(const SkPoint& start, const SkPoint& stop, const SkPaint& paint)
{
	AutoPaintSetClear	force(paint, fMCRec->fSetPaintBits, fMCRec->fClearPaintBits);
	SkAutoBounderCommit	ac(fBounder);
	SkDraw				draw(*this);

	draw.drawLine(start, stop, paint);
}

void SkCanvas::drawLine(SkScalar x0, SkScalar y0, SkScalar x1, SkScalar y1, const SkPaint& paint)
{
	AutoPaintSetClear	force(paint, fMCRec->fSetPaintBits, fMCRec->fClearPaintBits);
	SkAutoBounderCommit	ac(fBounder);
	SkDraw				draw(*this);

	SkPoint	pts[2];
	pts[0].set(x0, y0);
	pts[1].set(x1, y1);
	draw.drawLine(pts[0], pts[1], paint);
}

//#include <stdio.h>

void SkCanvas::drawRect(const SkRect& r, const SkPaint& paint)
{
	AutoPaintSetClear	force(paint, fMCRec->fSetPaintBits, fMCRec->fClearPaintBits);
	SkAutoBounderCommit	ac(fBounder);
	SkDraw				draw(*this);

#if 0
    const SkScalar* m = (const SkScalar*)draw.fMatrix;
printf("drawRect(%g %g %g %g) matrix(%g %g %g %g %g %g)\n",
        SkScalarToFloat(r.fLeft), SkScalarToFloat(r.fTop), SkScalarToFloat(r.fRight), SkScalarToFloat(r.fBottom),
        SkScalarToFloat(m[0]), SkScalarToFloat(m[1]), SkScalarToFloat(m[2]), 
        SkScalarToFloat(m[3]), SkScalarToFloat(m[4]), SkScalarToFloat(m[5]),
        SkScalarToFloat(m[6]), SkScalarToFloat(m[7]), SkScalarToFloat(m[8]));
#endif

	draw.drawRect(r, paint);
}

void SkCanvas::drawRect(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom, const SkPaint& paint)
{
    SkRect  r;
    
    r.set(left, top, right, bottom);
    this->drawRect(r, paint);
}

void SkCanvas::drawOval(const SkRect& oval, const SkPaint& paint)
{
	AutoPaintSetClear	force(paint, fMCRec->fSetPaintBits, fMCRec->fClearPaintBits);
	SkAutoBounderCommit	ac(fBounder);
	SkDraw				draw(*this);

	SkPath	path;

	path.addOval(oval);
	draw.drawPath(path, paint, NULL, true);
}

void SkCanvas::drawCircle(SkScalar cx, SkScalar cy, SkScalar radius, const SkPaint& paint)
{
	AutoPaintSetClear	force(paint, fMCRec->fSetPaintBits, fMCRec->fClearPaintBits);
	SkAutoBounderCommit	ac(fBounder);
	SkDraw				draw(*this);

	SkPath	path;

	path.addCircle(cx, cy, radius);
	draw.drawPath(path, paint, NULL, true);
}

void SkCanvas::drawRoundRect(const SkRect& r, SkScalar rx, SkScalar ry, const SkPaint& paint)
{
	AutoPaintSetClear	force(paint, fMCRec->fSetPaintBits, fMCRec->fClearPaintBits);
	SkAutoBounderCommit	ac(fBounder);
	SkDraw				draw(*this);

	SkPath	path;

	path.addRoundRect(r, rx, ry, SkPath::kCW_Direction);
	draw.drawPath(path, paint, NULL, true);
}

void SkCanvas::drawPath(const SkPath& path, const SkPaint& paint)
{
	AutoPaintSetClear	force(paint, fMCRec->fSetPaintBits, fMCRec->fClearPaintBits);
	SkAutoBounderCommit	ac(fBounder);
	SkDraw				draw(*this);

	draw.drawPath(path, paint, NULL, false);
}

void SkCanvas::drawBitmap(const SkBitmap& bitmap, SkScalar x, SkScalar y, const SkPaint& paint)
{
	AutoPaintSetClear	force(paint, fMCRec->fSetPaintBits, fMCRec->fClearPaintBits);
	SkAutoBounderCommit	ac(fBounder);
	SkDraw				draw(*this);

	draw.drawBitmap(bitmap, x, y, paint);
}

void SkCanvas::drawBitmap(const SkBitmap& bitmap, SkScalar x, SkScalar y)
{
    SkPaint paint;
    this->drawBitmap(bitmap, x, y, paint);
}

void SkCanvas::drawSprite(const SkBitmap& bitmap, int x, int y, const SkPaint& paint)
{
	AutoPaintSetClear	force(paint, fMCRec->fSetPaintBits, fMCRec->fClearPaintBits);
	SkAutoBounderCommit	ac(fBounder);
	SkDraw				draw(*this);

	draw.drawSprite(bitmap, x, y, paint);
}

void SkCanvas::drawText(const char text[], size_t byteLength, SkScalar x, SkScalar y, const SkPaint& paint)
{
	AutoPaintSetClear	force(paint, fMCRec->fSetPaintBits, fMCRec->fClearPaintBits);
	SkAutoBounderCommit	ac(fBounder);
	SkDraw				draw(*this);

	draw.drawText((SkUnicodeWalkerProc)SkUTF8_NextUnichar, text, byteLength, x, y, paint);
}

void SkCanvas::drawText16(const U16 text[], size_t numberOf16BitValues, SkScalar x, SkScalar y, const SkPaint& paint)
{
	AutoPaintSetClear	force(paint, fMCRec->fSetPaintBits, fMCRec->fClearPaintBits);
	SkAutoBounderCommit	ac(fBounder);
	SkDraw				draw(*this);

	draw.drawText((SkUnicodeWalkerProc)SkUTF16_NextUnichar, (const char*)text, numberOf16BitValues << 1, x, y, paint);
}

void SkCanvas::drawPosText(const char text[], size_t byteLength, const SkPoint pos[], const SkPaint& paint)
{
	AutoPaintSetClear	force(paint, fMCRec->fSetPaintBits, fMCRec->fClearPaintBits);
	SkAutoBounderCommit	ac(fBounder);
	SkDraw				draw(*this);

	draw.drawPosText((SkUnicodeWalkerProc)SkUTF8_NextUnichar, text, byteLength, pos, paint);
}

void SkCanvas::drawPosText16(const U16 text[], size_t numberOf16BitValues, const SkPoint pos[], const SkPaint& paint)
{
	AutoPaintSetClear	force(paint, fMCRec->fSetPaintBits, fMCRec->fClearPaintBits);
	SkAutoBounderCommit	ac(fBounder);
	SkDraw				draw(*this);

	draw.drawPosText((SkUnicodeWalkerProc)SkUTF16_NextUnichar, (const char*)text, numberOf16BitValues << 1, pos, paint);
}

void SkCanvas::drawTextOnPath(const char text[], size_t byteLength, const SkPath& path, SkScalar offset, const SkPaint& paint)
{
	AutoPaintSetClear	force(paint, fMCRec->fSetPaintBits, fMCRec->fClearPaintBits);
	SkAutoBounderCommit	ac(fBounder);
	SkDraw				draw(*this);

	draw.drawTextOnPath((SkUnicodeWalkerProc)SkUTF8_NextUnichar, text, byteLength, path, offset, paint);
}

void SkCanvas::drawText16OnPath(const U16 text[], size_t numberOf16BitValues, const SkPath& path, SkScalar offset, const SkPaint& paint)
{
	AutoPaintSetClear	force(paint, fMCRec->fSetPaintBits, fMCRec->fClearPaintBits);
	SkAutoBounderCommit	ac(fBounder);
	SkDraw				draw(*this);

	draw.drawTextOnPath((SkUnicodeWalkerProc)SkUTF16_NextUnichar, (const char*)text, numberOf16BitValues << 1, path, offset, paint);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

const SkBitmap& SkCanvas::getCurrBitmap() const
{
    return *fMCRec->fCurrBitmap;
}

SkMatrix::MapPtProc SkCanvas::getCurrMapPtProc() const
{
    if (fMCRec->fMapPtProc == NULL)
        fMCRec->fMapPtProc = fMCRec->fMatrix.getMapPtProc();

    return fMCRec->fMapPtProc;
}

const SkMatrix& SkCanvas::getTotalMatrix() const
{
    return fMCRec->fMatrix;
}

const SkRegion& SkCanvas::getTotalClip() const
{
    return fMCRec->fRegion;
}

