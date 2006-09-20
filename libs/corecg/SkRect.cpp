#include "SkRect.h"

bool SkRect16::intersect(S16CPU left, S16CPU top, S16CPU right, S16CPU bottom)
{
	if (fLeft < right && left < fRight && fTop < bottom && top < fBottom)
	{
		if (fLeft < left) fLeft = SkToS16(left);
		if (fTop < top) fTop = SkToS16(top);
		if (fRight > right) fRight = SkToS16(right);
		if (fBottom > bottom) fBottom = SkToS16(bottom);
		return true;
	}
	return false;
}

bool SkRect16::intersect(const SkRect16& r)
{
	SkASSERT(&r);
	return this->intersect(r.fLeft, r.fTop, r.fRight, r.fBottom);
}

bool SkRect16::intersect(const SkRect16& a, const SkRect16& b)
{
	SkASSERT(&a && &b);

	*this = a;
	return this->intersect(b.fLeft, b.fTop, b.fRight, b.fBottom);
}

void SkRect16::sort()
{
	if (fLeft > fRight)
		SkTSwap<S16>(fLeft, fRight);
	if (fTop > fBottom)
		SkTSwap<S16>(fTop, fBottom);
}

/////////////////////////////////////////////////////////////////////////////

void SkRect::sort()
{
	if (fLeft > fRight)
		SkTSwap<SkScalar>(fLeft, fRight);
	if (fTop > fBottom)
		SkTSwap<SkScalar>(fTop, fBottom);
}

void SkRect::toQuad(SkPoint quad[4]) const
{
	SkASSERT(quad);

	quad[0].set(fLeft, fTop);
	quad[1].set(fRight, fTop);
	quad[2].set(fRight, fBottom);
	quad[3].set(fLeft, fBottom);
}

void SkRect::set(const SkPoint pts[], int count)
{
	SkASSERT(pts && count > 0 || count == 0);

	if (count <= 0)
		memset(this, 0, sizeof(SkRect));
	else
	{
		SkScalar	l, t, r, b;

		l = r = pts[0].fX;
		t = b = pts[0].fY;

		for (int i = 1; i < count; i++)
		{
			SkScalar x = pts[i].fX;
			SkScalar y = pts[i].fY;

			if (x < l) l = x; else if (x > r) r = x;
			if (y < t) t = y; else if (y > b) b = y;
		}
		this->set(l, t, r, b);
	}
}

bool SkRect::intersect(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom)
{
	if (fLeft < right && left < fRight && fTop < bottom && top < fBottom)
	{
		if (fLeft < left) fLeft = left;
		if (fTop < top) fTop = top;
		if (fRight > right) fRight = right;
		if (fBottom > bottom) fBottom = bottom;
		return true;
	}
	return false;
}

bool SkRect::intersect(const SkRect& r)
{
	SkASSERT(&r);
	return this->intersect(r.fLeft, r.fTop, r.fRight, r.fBottom);
}



