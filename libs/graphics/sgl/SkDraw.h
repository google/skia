#ifndef SkDraw_DEFINED
#define SkDraw_DEFINED

#include "SkBitmap.h"
#include "SkMask.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkRect.h"

class SkBounder;
class SkCanvas;
class SkPath;
class SkRegion;

class SkDraw {
public:
	SkDraw() {}
	SkDraw(const SkCanvas&);

	void	drawPaint(const SkPaint&);
	void	drawLine(const SkPoint& start, const SkPoint& stop, const SkPaint&);
	void	drawRect(const SkRect&, const SkPaint&);
	/*	To save on mallocs, we allow a flag that tells us that srcPath is mutable, so that we don't have to
		make copies of it as we transform it.
	*/
	void	drawPath(const SkPath& srcPath, const SkPaint&, const SkMatrix* prePathMatrix, bool srcPathIsMutable);
	void	drawBitmap(const SkBitmap&, SkScalar x, SkScalar y, const SkPaint&);
    void    drawSprite(const SkBitmap& bitmap, int x, int y, const SkPaint& paint);
	void	drawText(SkUnicodeWalkerProc, const char text[], size_t byteLength, SkScalar x, SkScalar y, const SkPaint& paint);
	void	drawPosText(SkUnicodeWalkerProc, const char text[], size_t byteLength, const SkPoint pos[], const SkPaint& paint);
	void	drawTextOnPath(SkUnicodeWalkerProc, const char text[], size_t byteLength, const SkPath& follow,
							SkScalar offset, const SkPaint& paint);

	void	drawPath(const SkPath& src, const SkPaint& paint)
    {
        this->drawPath(src, paint, NULL, false);
    }

    /** Helper function that creates a mask from a path and an optional maskfilter.
        Note however, that the resulting mask will not have been actually filtered,
        that must be done afterwards (by calling filterMask). The maskfilter is provided
        solely to assist in computing the mask's bounds (if the mode requests that).
    */
    static bool DrawToMask(const SkPath& devPath, const SkRect16* clipBounds,
                           SkMaskFilter* filter, const SkMatrix* filterMatrix,
                           SkMask* mask, SkMask::CreateMode mode);

private:
	void	drawText_asPaths(SkUnicodeWalkerProc, const char text[], size_t byteLength, SkScalar x, SkScalar y, const SkPaint&);
	void	drawDevMask(const SkMask& mask, const SkPaint&);

#ifdef SK_DEBUG
	void	validate() const;
#endif

public:
	const SkBitmap*	fDevice;		// required
	const SkMatrix*	fMatrix;		// required
	const SkRegion*	fClip;			// required
	SkMatrix::MapPtProc	fMapPtProc;	// required
	SkBounder*		fBounder;		// optional
};

class SkTextToPathIter {
public:
	SkTextToPathIter(SkUnicodeWalkerProc, const char text[], size_t length, const SkPaint&,
					 bool applyStrokeAndPathEffects, bool forceLinearTextOn);
	~SkTextToPathIter();

	const SkPaint&	getPaint() const { return fPaint; }
	SkScalar		getPathScale() const { return fScale; }

	const SkPath*	next(SkScalar* xpos);	//!< returns nil when there are no more paths

private:
	SkGlyphCache*	fCache;
	SkPaint			fPaint;
	SkScalar		fScale, fPrevAdvance;
	const char*		fText;
	const char*		fStop;
    SkUnicodeWalkerProc fTextProc;

	const SkPath*	fPath;		// returned in next
	SkScalar		fXPos;		// accumulated xpos, returned in next
};

#endif


