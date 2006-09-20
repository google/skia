#include "SkPoint.h"

void SkPoint16::rotateCW(SkPoint16* dst) const
{
	SkASSERT(dst);

	// use a tmp in case this == dst
	S16 tmp = fX;
	dst->fX = -fY;
	dst->fY = tmp;
}

void SkPoint16::rotateCCW(SkPoint16* dst) const
{
	SkASSERT(dst);

	// use a tmp in case this == dst
	S16 tmp = fX;
	dst->fX = fY;
	dst->fY = -tmp;
}

/////////////////////////////////////////////////////////////////////

void SkPoint::rotateCW(SkPoint* dst) const
{
	SkASSERT(dst);

	// use a tmp in case this == dst
	SkScalar tmp = fX;
	dst->fX = -fY;
	dst->fY = tmp;
}

void SkPoint::rotateCCW(SkPoint* dst) const
{
	SkASSERT(dst);

	// use a tmp in case this == dst
	SkScalar tmp = fX;
	dst->fX = fY;
	dst->fY = -tmp;
}

void SkPoint::scale(SkScalar scale, SkPoint* dst) const
{
	SkASSERT(dst);
	dst->set(SkScalarMul(fX, scale), SkScalarMul(fY, scale));
}

#define kNearlyZero		(SK_Scalar1 / 8092)

bool SkPoint::normalize()
{
	return this->setLength(fX, fY, SK_Scalar1);
}

bool SkPoint::setUnit(SkScalar x, SkScalar y)
{
	return this->setLength(x, y, SK_Scalar1);
}

bool SkPoint::setLength(SkScalar length)
{
	return this->setLength(fX, fY, length);
}

#ifdef SK_SCALAR_IS_FLOAT

SkScalar SkPoint::Length(SkScalar dx, SkScalar dy)
{
	return sk_float_sqrt(dx * dx + dy * dy);
}

bool SkPoint::setLength(float x, float y, float length)
{
	float mag = sk_float_sqrt(x * x + y * y);
	if (mag > kNearlyZero)
	{
		length /= mag;
		fX = x * length;
		fY = y * length;
		return true;
	}
	return false;
}

#else

#include "Sk64.h"

SkScalar SkPoint::Length(SkScalar dx, SkScalar dy)
{
	Sk64	tmp1, tmp2;

	tmp1.setMul(dx, dx);
	tmp2.setMul(dy, dy);
	tmp1.add(tmp2);

	return tmp1.getSqrt();
}

#ifdef SK_DEBUGx
static SkFixed fixlen(SkFixed x, SkFixed y)
{
	float fx = (float)x;
	float fy = (float)y;

	return (int)floorf(sqrtf(fx*fx + fy*fy) + 0.5f);
}
#endif

static inline U32 squarefixed(unsigned x)
{
	x >>= 16;
	return x*x;
}

/*
	Normalize x,y, and then scale them by length.

	The obvious way to do this would be the following:
		S64	tmp1, tmp2;
		tmp1.setMul(x,x);
		tmp2.setMul(y,y);
		tmp1.add(tmp2);
		len = tmp1.getSqrt();
		x' = SkFixedDiv(x, len);
		y' = SkFixedDiv(y, len);
	This is fine, but slower than what we do below.

	The present technique does not compute the starting length, but
	rather fiddles with x,y iteratively, all the while checking its
	magnitude^2 (avoiding a sqrt).

	We normalize by first shifting x,y so that at least one of them
	has bit 31 set (after taking the abs of them).
	Then we loop, refining x,y by squaring them and comparing
	against a very large 1.0 (1 << 28), and then adding or subtracting
	a delta (which itself is reduced by half each time through the loop).
	For speed we want the squaring to be with a simple integer mul. To keep
	that from overflowing we shift our coordinates down until we are dealing
	with at most 15 bits (2^15-1)^2 * 2 says withing 32 bits)
	When our square is close to 1.0, we shift x,y down into fixed range.
*/
bool SkPoint::setLength(SkFixed ox, SkFixed oy, SkFixed length)
{
	if (ox == 0)
	{
		if (oy == 0)
			return false;
		this->set(0, SkApplySign(length, SkExtractSign(oy)));
		return true;
	}
	if (oy == 0)
	{
		this->set(SkApplySign(length, SkExtractSign(ox)), 0);
		return true;
	}

	SkFixed x = SkAbs32(ox);
	SkFixed y = SkAbs32(oy);

	// shift x,y so that the greater of them is 15bits (1.14 fixed point)
	{
		int shift = SkCLZ(x | y);
		// make them .30
		x <<= shift - 1;
		y <<= shift - 1;
	}

	SkFixed dx = x;
	SkFixed dy = y;

	for (int i = 0; i < 17; i++)
	{
		dx >>= 1;
		dy >>= 1;

		U32 len2 = squarefixed(x) + squarefixed(y);
		if (len2 >> 28)
		{
			x -= dx;
			y -= dy;
		}
		else
		{
			x += dx;
			y += dy;
		}
	}
	x >>= 14;
	y >>= 14;

#ifdef SK_DEBUGx	// measure how far we are from unit-length
	{
		static int gMaxError;
		static int gMaxDiff;

		SkFixed len = fixlen(x, y);
		int err = len - SK_Fixed1;
		err = SkAbs32(err);

		if (err > gMaxError)
		{
			gMaxError = err;
			SkDebugf("gMaxError %d\n", err);
		}

		float fx = SkAbs32(ox)/65536.0f;
		float fy = SkAbs32(oy)/65536.0f;
		float mag = sqrtf(fx*fx + fy*fy);
		fx /= mag;
		fy /= mag;
		SkFixed xx = (int)floorf(fx * 65536 + 0.5f);
		SkFixed yy = (int)floorf(fy * 65536 + 0.5f);
		err = SkMax32(SkAbs32(xx-x), SkAbs32(yy-y));
		if (err > gMaxDiff)
		{
			gMaxDiff = err;
			SkDebugf("gMaxDiff %d\n", err);
		}
	}
#endif

	x = SkApplySign(x, SkExtractSign(ox));
	y = SkApplySign(y, SkExtractSign(oy));
	if (length != SK_Fixed1)
	{
		x = SkFixedMul(x, length);
		y = SkFixedMul(y, length);
	}
	this->set(x, y);
	return true;
}

#endif

