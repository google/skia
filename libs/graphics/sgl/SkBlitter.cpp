#include "SkBlitter.h"
#include "SkAntiRun.h"
#include "SkColor.h"
#include "SkColorFilter.h"
#include "SkMask.h"
#include "SkMaskFilter.h"
#include "SkTemplatesPriv.h"
#include "SkUtils.h"
#include "SkXfermode.h"

SkBlitter::~SkBlitter()
{
}

void SkBlitter::blitH(int x, int y, int width)
{
	SkASSERT(!"unimplemented");
}

void SkBlitter::blitAntiH(int x, int y, const SkAlpha antialias[], const int16_t runs[])
{
	SkASSERT(!"unimplemented");
}

void SkBlitter::blitV(int x, int y, int height, SkAlpha alpha)
{
	if (alpha == 255)
		this->blitRect(x, y, 1, height);
	else
	{
		int16_t	runs[2];
		runs[0] = 1;
		runs[1] = 0;

		while (--height >= 0)
			this->blitAntiH(x, y++, &alpha, runs);
	}
}

void SkBlitter::blitRect(int x, int y, int width, int height)
{
	while (--height >= 0)
		this->blitH(x, y++, width);
}

//////////////////////////////////////////////////////////////////////////////

static inline void bits_to_runs(SkBlitter* blitter, int x, int y, const uint8_t bits[], U8CPU left_mask, int rowBytes, U8CPU right_mask)
{
	int	inFill = 0;
	int pos = 0;

	while (--rowBytes >= 0)
	{
		unsigned b = *bits++ & left_mask;
		if (rowBytes == 0)
			b &= right_mask;

		for (unsigned test = 0x80; test != 0; test >>= 1)
		{
			if (b & test)
			{
				if (!inFill)
				{
					pos = x;
					inFill = true;
				}
			}
			else
			{
				if (inFill)
				{
					blitter->blitH(pos, y, x - pos);
					inFill = false;
				}
			}
			x += 1;
		}
		left_mask = 0xFF;
	}

	// final cleanup
	if (inFill)
		blitter->blitH(pos, y, x - pos);
}

void SkBlitter::blitMask(const SkMask& mask, const SkRect16& clip)
{
	SkASSERT(mask.fBounds.contains(clip));

	if (mask.fFormat == SkMask::kBW_Format)
	{
		int cx = clip.fLeft;
		int cy = clip.fTop;
		int maskLeft = mask.fBounds.fLeft;
		int mask_rowBytes = mask.fRowBytes;
		int	height = clip.height();

		const uint8_t* bits = mask.getAddr1(cx, cy);

		if (cx == maskLeft && clip.fRight == mask.fBounds.fRight)
		{
			while (--height >= 0)
			{
				bits_to_runs(this, cx, cy, bits, 0xFF, mask_rowBytes, 0xFF);
				bits += mask_rowBytes;
				cy += 1;
			}
		}
		else
		{
			int left_edge = cx - maskLeft;
			SkASSERT(left_edge >= 0);
			int rite_edge = clip.fRight - maskLeft;
			SkASSERT(rite_edge > left_edge);

			int left_mask = 0xFF >> (left_edge & 7);
			int rite_mask = 0xFF << (8 - (rite_edge & 7));
			int full_runs = (rite_edge >> 3) - ((left_edge + 7) >> 3);

			// check for empty right mask, so we don't read off the end (or go slower than we need to)
			if (rite_mask == 0)
			{
				SkASSERT(full_runs >= 0);
				full_runs -= 1;
				rite_mask = 0xFF;
			}
			if (left_mask == 0xFF)
				full_runs -= 1;

			// back up manually so we can keep in sync with our byte-aligned src
			// have cx reflect our actual starting x-coord
			cx -= left_edge & 7;

			if (full_runs < 0)
			{
				SkASSERT((left_mask & rite_mask) != 0);
				while (--height >= 0)
				{
					bits_to_runs(this, cx, cy, bits, left_mask, 1, rite_mask);
					bits += mask_rowBytes;
					cy += 1;
				}
			}
			else
			{
				while (--height >= 0)
				{
					bits_to_runs(this, cx, cy, bits, left_mask, full_runs + 2, rite_mask);
					bits += mask_rowBytes;
					cy += 1;
				}
			}
		}
	}
	else
	{
		int                         width =	clip.width();
		SkAutoSTMalloc<64, int16_t>	runStorage(width + 1);
		int16_t*                    runs = runStorage.get();
		const uint8_t*              aa = mask.getAddr(clip.fLeft, clip.fTop);

		sk_memset16((U16*)runs, 1, width);
		runs[width] = 0;

		int height = clip.height();
		int y = clip.fTop;
		while (--height >= 0)
		{
			this->blitAntiH(clip.fLeft, y, aa, runs);
			aa += mask.fRowBytes;
			y += 1;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////

// this guy is not virtual, just a helper
void SkBlitter::blitMaskRegion(const SkMask& mask, const SkRegion& clip)
{
    if (clip.quickReject(mask.fBounds))
        return;

    SkRegion::Cliperator clipper(clip, mask.fBounds);

    if (!clipper.done())
    {
        const SkRect16&	cr = clipper.rect();
        do {
            this->blitMask(mask, cr);
            clipper.next();
        } while (!clipper.done());
    }
}

///////////////////////////////////////////////////////////////////////////////////////

static int compute_anti_width(const int16_t runs[])
{
	int	width = 0;

	for (;;)
	{
		int count = runs[0];

		SkASSERT(count >= 0);
		if (count == 0)
			break;
		width += count;
		runs += count;

		SkASSERT(width < 20000);
	}
	return width;
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

void SkNullBlitter::blitH(int x, int y, int width)
{
}

void SkNullBlitter::blitAntiH(int x, int y, const SkAlpha antialias[], const int16_t runs[])
{
}

void SkNullBlitter::blitV(int x, int y, int height, SkAlpha alpha)
{
}

void SkNullBlitter::blitRect(int x, int y, int width, int height)
{
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

static inline bool y_in_rect(int y, const SkRect16& rect)
{
	return (unsigned)(y - rect.fTop) < (unsigned)rect.height();
}

static inline bool x_in_rect(int x, const SkRect16& rect)
{
	return (unsigned)(x - rect.fLeft) < (unsigned)rect.width();
}

void SkRectClipBlitter::blitH(int left, int y, int width)
{
	SkASSERT(width > 0);

	if (!y_in_rect(y, fClipRect))
		return;

	int	right = left + width;

	if (left < fClipRect.fLeft)
		left = fClipRect.fLeft;
	if (right > fClipRect.fRight)
		right = fClipRect.fRight;

	width = right - left;
	if (width > 0)
		fBlitter->blitH(left, y, width);
}

void SkRectClipBlitter::blitAntiH(int left, int y, const SkAlpha aa[], const int16_t runs[])
{
	if (!y_in_rect(y, fClipRect) || left >= fClipRect.fRight)
		return;

	int x0 = left;
	int	x1 = left + compute_anti_width(runs);

	if (x1 <= fClipRect.fLeft)
		return;

	SkASSERT(x0 < x1);
	if (x0 < fClipRect.fLeft)
	{
		int dx = fClipRect.fLeft - x0;
		SkAlphaRuns::BreakAt((int16_t*)runs, (U8*)aa, dx);
		runs += dx;
		aa += dx;
		x0 = fClipRect.fLeft;
	}

	SkASSERT(x0 < x1 && runs[x1 - x0] == 0);
	if (x1 > fClipRect.fRight)
	{
		x1 = fClipRect.fRight;
		SkAlphaRuns::BreakAt((int16_t*)runs, (U8*)aa, x1 - x0);
		((int16_t*)runs)[x1 - x0] = 0;
	}

	SkASSERT(x0 < x1 && runs[x1 - x0] == 0);
	SkASSERT(compute_anti_width(runs) == x1 - x0);

	fBlitter->blitAntiH(x0, y, aa, runs);
}

void SkRectClipBlitter::blitV(int x, int y, int height, SkAlpha alpha)
{
	SkASSERT(height > 0);

	if (!x_in_rect(x, fClipRect))
		return;

	int	y0 = y;
	int y1 = y + height;

	if (y0 < fClipRect.fTop)
		y0 = fClipRect.fTop;
	if (y1 > fClipRect.fBottom)
		y1 = fClipRect.fBottom;

	if (y0 < y1)
		fBlitter->blitV(x, y0, y1 - y0, alpha);
}

void SkRectClipBlitter::blitRect(int left, int y, int width, int height)
{
	SkRect16	r;

	r.set(left, y, left + width, y + height);
	if (r.intersect(fClipRect))
		fBlitter->blitRect(r.fLeft, r.fTop, r.width(), r.height());
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

void SkRgnClipBlitter::blitH(int x, int y, int width)
{
	SkRegion::Spanerator span(*fRgn, y, x, x + width);
	int	left, right;

	while (span.next(&left, &right))
	{
		SkASSERT(left < right);
		fBlitter->blitH(left, y, right - left);
	}
}

void SkRgnClipBlitter::blitAntiH(int x, int y, const SkAlpha aa[], const int16_t runs[])
{
	int width = compute_anti_width(runs);
	SkRegion::Spanerator span(*fRgn, y, x, x + width);
	int	left, right;
	bool firstTime = true;
	SkDEBUGCODE(const SkRect16& bounds = fRgn->getBounds();)

//SkDebugf("rgnClip: x=%d y=%d: ", x, y);

	while (span.next(&left, &right))
	{
		SkASSERT(left < right);
		SkASSERT(left >= bounds.fLeft && right <= bounds.fRight);
		
		if (firstTime && x < left)
		{
//SkDebugf("zap[%d %d] ", x, left);
			SkAlphaRuns::Break((int16_t*)runs, (U8*)aa, 0, left - x);
			((U8*)aa)[0] = 0;	// skip runs before the first left
			((int16_t*)runs)[0] = SkToS16(left - x);
		}
		firstTime = false;

		SkAlphaRuns::Break((int16_t*)runs, (U8*)aa, left - x, right - left);
		((U8*)aa)[right - x] = 0;	// skip runs after right
		((int16_t*)runs)[right - x] = SkToS16(right - left);
		
//SkDebugf("[%d %d] ", left, right);
	}
	((int16_t*)runs)[right - x] = 0;

//dump_runs(runs, aa);

	fBlitter->blitAntiH(x, y, aa, runs);
}

void SkRgnClipBlitter::blitV(int x, int y, int height, SkAlpha alpha)
{
	SkRect16	bounds;
	bounds.set(x, y, x + 1, y + height);

	SkRegion::Cliperator	iter(*fRgn, bounds);

	while (!iter.done())
	{
		const SkRect16& r = iter.rect();
		SkASSERT(bounds.contains(r));

		fBlitter->blitV(x, r.fTop, r.height(), alpha);
		iter.next();
	}
}

void SkRgnClipBlitter::blitRect(int x, int y, int width, int height)
{
	SkRect16	bounds;
	bounds.set(x, y, x + width, y + height);

	SkRegion::Cliperator	iter(*fRgn, bounds);

	while (!iter.done())
	{
		const SkRect16& r = iter.rect();
		SkASSERT(bounds.contains(r));

		fBlitter->blitRect(r.fLeft, r.fTop, r.width(), r.height());
		iter.next();
	}
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

SkBlitter* SkBlitterClipper::apply(SkBlitter* blitter, const SkRegion* clip, const SkRect16* ir)
{
	if (clip)
	{
		const SkRect16& clipR = clip->getBounds();

		if (clip->isEmpty() || ir && !SkRect16::Intersects(clipR, *ir))
			blitter = &fNullBlitter;
		else if (clip->isRect())
		{
			if (ir == nil || !clipR.contains(*ir))
			{
				fRectBlitter.init(blitter, clipR);
				blitter = &fRectBlitter;
			}
		}
		else
		{
			fRgnBlitter.init(blitter, clip);
			blitter = &fRgnBlitter;
		}
	}
	return blitter;
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

#include "SkShader.h"
#include "SkColorPriv.h"

class SkColorShader : public SkShader {
public:
	virtual U32 getFlags()
	{
        // should I claim hasspan16 if my color isn't opaque?
		return (SkGetPackedA32(fPMColor) == 255 ? kOpaqueAlpha_Flag : kConstAlpha_Flag) | kHasSpan16_Flag;
	}
	virtual bool setContext(const SkBitmap& device, const SkPaint& paint, const SkMatrix& matrix)
	{
		if (!this->INHERITED::setContext(device, paint, matrix))
			return false;

		SkColor	c = paint.getColor();
		unsigned a = SkColorGetA(c);
		unsigned r = SkColorGetR(c);
		unsigned g = SkColorGetG(c);
		unsigned b = SkColorGetB(c);

		if (a != 255)
		{
			a = SkAlpha255To256(a);
			r = SkAlphaMul(r, a);
			g = SkAlphaMul(g, a);
			b = SkAlphaMul(b, a);
		}
		fPMColor = SkPackARGB32(a, r, g, b);
		fColor16 = SkPixel32ToPixel16_ToU16(fPMColor);	// only meaning full if a == 255
		return true;
	}
	virtual void shadeSpan(int x, int y, SkPMColor span[], int count)
	{
		sk_memset32(span, fPMColor, count);
	}
	virtual void shadeSpanOpaque16(int x, int y, U16 span[], int count)
	{
		SkASSERT(SkGetPackedA32(fPMColor) == 255);
		sk_memset16(span, fColor16, count);
	}
private:
	SkPMColor	fPMColor;
	U16			fColor16;

	typedef SkShader INHERITED;
};

class Sk3DShader : public SkShader {
public:
	Sk3DShader(SkShader* proxy) : fProxy(proxy)
	{
		proxy->safeRef();
		fMask = nil;
	}
	virtual ~Sk3DShader()
	{
		fProxy->safeUnref();
	}
	void setMask(const SkMask* mask) { fMask = mask; }

	virtual bool setContext(const SkBitmap& device, const SkPaint& paint, const SkMatrix& matrix)
	{
		if (fProxy)
			return fProxy->setContext(device, paint, matrix);
		else
		{
			fPMColor = SkPreMultiplyColor(paint.getColor());
			return this->INHERITED::setContext(device, paint, matrix);
		}
	}
	virtual void shadeSpan(int x, int y, SkPMColor span[], int count)
	{
		if (fProxy)
			fProxy->shadeSpan(x, y, span, count);

		if (fMask == nil)
		{
			if (fProxy == nil)
				sk_memset32(span, fPMColor, count);
			return;
		}

		SkASSERT(fMask->fBounds.contains(x, y));
		SkASSERT(fMask->fBounds.contains(x + count - 1, y));

		size_t		size = fMask->computeImageSize();
		const U8*	alpha = fMask->getAddr(x, y);
		const U8*	mulp = alpha + size;
		const U8*	addp = mulp + size;

		if (fProxy)
		{
			for (int i = 0; i < count; i++)
			{
				if (alpha[i])
				{
					U32	c = span[i];
					if (c)
					{
						unsigned a = SkGetPackedA32(c);
						unsigned r = SkGetPackedR32(c);
						unsigned g = SkGetPackedG32(c);
						unsigned b = SkGetPackedB32(c);

						unsigned mul = SkAlpha255To256(mulp[i]);
						unsigned add = addp[i];

						r = SkFastMin32(SkAlphaMul(r, mul) + add, a);
						g = SkFastMin32(SkAlphaMul(g, mul) + add, a);
						b = SkFastMin32(SkAlphaMul(b, mul) + add, a);

						span[i] = SkPackARGB32(a, r, g, b);
					}
				}
				else
					span[i] = 0;
			}
		}
		else	// color
		{
			unsigned a = SkGetPackedA32(fPMColor);
			unsigned r = SkGetPackedR32(fPMColor);
			unsigned g = SkGetPackedG32(fPMColor);
			unsigned b = SkGetPackedB32(fPMColor);
			for (int i = 0; i < count; i++)
			{
				if (alpha[i])
				{
					unsigned mul = SkAlpha255To256(mulp[i]);
					unsigned add = addp[i];

					span[i] = SkPackARGB32(	a,
											SkFastMin32(SkAlphaMul(r, mul) + add, a),
											SkFastMin32(SkAlphaMul(g, mul) + add, a),
											SkFastMin32(SkAlphaMul(b, mul) + add, a));
				}
				else
					span[i] = 0;
			}
		}
	}
private:
	SkShader*		fProxy;
	SkPMColor		fPMColor;
	const SkMask*	fMask;

	typedef SkShader INHERITED;
};

class Sk3DBlitter : public SkBlitter {
public:
	Sk3DBlitter(SkBlitter* proxy, Sk3DShader* shader, void (*killProc)(void*))
		: fProxy(proxy), f3DShader(shader), fKillProc(killProc)
	{
		shader->ref();
	}
	virtual ~Sk3DBlitter()
	{
		f3DShader->unref();
		fKillProc(fProxy);
	}

	virtual void blitH(int x, int y, int width)
	{
		fProxy->blitH(x, y, width);
	}
	virtual void blitAntiH(int x, int y, const SkAlpha antialias[], const int16_t runs[])
	{
		fProxy->blitAntiH(x, y, antialias, runs);
	}
	virtual void blitV(int x, int y, int height, SkAlpha alpha)
	{
		fProxy->blitV(x, y, height, alpha);
	}
	virtual void blitRect(int x, int y, int width, int height)
	{
		fProxy->blitRect(x, y, width, height);
	}
	virtual void blitMask(const SkMask& mask, const SkRect16& clip)
	{
		if (mask.fFormat == SkMask::k3D_Format)
		{
			f3DShader->setMask(&mask);

			((SkMask*)&mask)->fFormat = SkMask::kA8_Format;
			fProxy->blitMask(mask, clip);
			((SkMask*)&mask)->fFormat = SkMask::k3D_Format;

			f3DShader->setMask(nil);
		}
		else
			fProxy->blitMask(mask, clip);
	}
private:
	SkBlitter*	fProxy;
	Sk3DShader* f3DShader;
	void		(*fKillProc)(void*);
};

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

#include "SkCoreBlitters.h"

class SkAutoRestoreShader {
public:
	SkAutoRestoreShader(const SkPaint& p) : fPaint((SkPaint*)&p)
	{
		fShader = fPaint->getShader();
		fShader->safeRef();
	}
	~SkAutoRestoreShader()
	{
		fPaint->setShader(fShader);
		fShader->safeUnref();
	}
private:
	SkPaint*	fPaint;
	SkShader*	fShader;
};

class SkAutoCallProc {
public:
	typedef void (*Proc)(void*);
	SkAutoCallProc(void* obj, Proc proc)
		: fObj(obj), fProc(proc)
	{
	}
	~SkAutoCallProc()
	{
		if (fObj && fProc)
			fProc(fObj);
	}
	void* get() const { return fObj; }
	void* detach()
	{
		void* obj = fObj;
		fObj = nil;
		return obj;
	}
private:
	void*	fObj;
	Proc	fProc;
};

static void destroy_blitter(void* blitter)
{
	((SkBlitter*)blitter)->~SkBlitter();
}

static void delete_blitter(void* blitter)
{
	SkDELETE((SkBlitter*)blitter);
}

SkBlitter* SkBlitter::Choose(const SkBitmap& device,
							 const SkMatrix& matrix,
							 const SkPaint& paint,
							 void* storage, size_t storageSize)
{
	SkASSERT(storageSize == 0 || storage != nil);

	SkBlitter*	blitter = nil;
	SkAutoRestoreShader	restore(paint);
	SkShader* shader = paint.getShader();

	Sk3DShader* shader3D = nil;
	if (paint.getMaskFilter() != nil && paint.getMaskFilter()->getFormat() == SkMask::k3D_Format)
	{
		shader3D = SkNEW_ARGS(Sk3DShader, (shader));
		((SkPaint*)&paint)->setShader(shader3D)->unref();
		shader = shader3D;
	}

	SkXfermode* mode = paint.getXfermode();
	if (NULL == shader && (NULL != mode || paint.getColorFilter() != NULL))
	{
		// xfermodes require shaders for our current set of blitters
		shader = SkNEW(SkColorShader);
		((SkPaint*)&paint)->setShader(shader)->unref();
	}
    
    if (paint.getColorFilter() != NULL)
    {
        SkASSERT(shader);
        shader = SkNEW_ARGS(SkFilterShader, (shader, paint.getColorFilter()));
        ((SkPaint*)&paint)->setShader(shader)->unref();
    }

	if (shader)
	{
		if (!shader->setContext(device, paint, matrix))
			return SkNEW(SkNullBlitter);
	}

	switch (device.getConfig()) {
	case SkBitmap::kA1_Config:
		SK_PLACEMENT_NEW_ARGS(blitter, SkA1_Blitter, storage, storageSize, (device, paint));
		break;

	case SkBitmap::kA8_Config:
		if (shader)
			SK_PLACEMENT_NEW_ARGS(blitter, SkA8_Shader_Blitter, storage, storageSize, (device, paint));
		else
			SK_PLACEMENT_NEW_ARGS(blitter, SkA8_Blitter, storage, storageSize, (device, paint));
		break;

	case SkBitmap::kRGB_565_Config:
		if (shader)
		{
			if (mode)
				SK_PLACEMENT_NEW_ARGS(blitter, SkRGB16_Shader_Xfermode_Blitter, storage, storageSize, (device, paint));
			else
				SK_PLACEMENT_NEW_ARGS(blitter, SkRGB16_Shader_Blitter, storage, storageSize, (device, paint));
		}
		else if (paint.getColor() == SK_ColorBLACK)
			SK_PLACEMENT_NEW_ARGS(blitter, SkRGB16_Black_Blitter, storage, storageSize, (device, paint));
		else
			SK_PLACEMENT_NEW_ARGS(blitter, SkRGB16_Blitter, storage, storageSize, (device, paint));
		break;

	case SkBitmap::kARGB_8888_Config:
		if (shader)
			SK_PLACEMENT_NEW_ARGS(blitter, SkARGB32_Shader_Blitter, storage, storageSize, (device, paint));
		else if (paint.getColor() == SK_ColorBLACK)
			SK_PLACEMENT_NEW_ARGS(blitter, SkARGB32_Black_Blitter, storage, storageSize, (device, paint));
        else
			SK_PLACEMENT_NEW_ARGS(blitter, SkARGB32_Blitter, storage, storageSize, (device, paint));
		break;

	default:
		SkASSERT(!"unsupported device config");
		SK_PLACEMENT_NEW(blitter, SkNullBlitter, storage, storageSize);
	}

	if (shader3D)
	{
		void (*proc)(void*) = ((void*)storage == (void*)blitter) ? destroy_blitter : delete_blitter;
		SkAutoCallProc	tmp(blitter, proc);

		blitter = SkNEW_ARGS(Sk3DBlitter, (blitter, shader3D, proc));
		(void)tmp.detach();
	}
	return blitter;
}

