#include "Sk2DPathEffect.h"
#include "SkBlitter.h"
#include "SkPath.h"
#include "SkScan.h"

class Sk2DPathEffectBlitter : public SkBlitter {
public:
	Sk2DPathEffectBlitter(Sk2DPathEffect* pe, SkPath* dst)
		: fPE(pe), fDst(dst)
	{}
	virtual void blitH(int x, int y, int count)
	{
		fPE->nextSpan(x, y, count, fDst);
	}
private:
	Sk2DPathEffect*	fPE;
	SkPath*			fDst;
};

////////////////////////////////////////////////////////////////////////////////////

Sk2DPathEffect::Sk2DPathEffect(const SkMatrix& mat) : fMatrix(mat)
{
	mat.invert(&fInverse);
}

bool Sk2DPathEffect::filterPath(SkPath* dst, const SkPath& src, SkScalar* width)
{
	Sk2DPathEffectBlitter	blitter(this, dst);
	SkPath					tmp;
	SkRect					bounds;
	SkRect16				r;

	src.transform(fInverse, &tmp);
	tmp.computeBounds(&bounds, SkPath::kExact_BoundsType);
	bounds.round(&r);
	if (!r.isEmpty())
	{
		this->begin(r, dst);
		SkScan::FillPath(tmp, nil, &blitter);
		this->end(dst);
	}
	return true;
}

void Sk2DPathEffect::nextSpan(int x, int y, int count, SkPath* path)
{
	const SkMatrix& mat = this->getMatrix();
	SkPoint	src, dst;

	src.set(SkIntToScalar(x) + SK_ScalarHalf, SkIntToScalar(y) + SK_ScalarHalf);
	do {
		mat.mapPoints(&dst, &src, 1);
		this->next(dst, x++, y, path);
		src.fX += SK_Scalar1;
	} while (--count > 0);
}

void Sk2DPathEffect::begin(const SkRect16& uvBounds, SkPath* dst) {}
void Sk2DPathEffect::next(const SkPoint& loc, int u, int v, SkPath* dst) {}
void Sk2DPathEffect::end(SkPath* dst) {}

////////////////////////////////////////////////////////////////////////////////

void Sk2DPathEffect::flatten(SkWBuffer& buffer)
{
    this->INHERITED::flatten(buffer);

    buffer.write(&fMatrix, sizeof(fMatrix));
}

Sk2DPathEffect::Sk2DPathEffect(SkRBuffer& buffer) : SkPathEffect(buffer)
{
    buffer.read(&fMatrix, sizeof(fMatrix));
	fMatrix.invert(&fInverse);
}

SkFlattenable::Factory Sk2DPathEffect::getFactory()
{
    return CreateProc;
}

SkFlattenable* Sk2DPathEffect::CreateProc(SkRBuffer& buffer)
{
    return SkNEW_ARGS(Sk2DPathEffect, (buffer));
}



