#include "SkCamera.h"

static SkScalar VectorMultiplyDivide(int count, const SkScalar a[], int step_a,
									 const SkScalar b[], int step_b, SkScalar denom)
{
#ifdef SK_SCALAR_IS_FLOAT
	float prod = 0;
	for (int i = 0; i < count; i++)
	{
		prod += a[0] * b[0];
		a += step_a;
		b += step_b;
	}
	if (denom)	// denom == 0 is a formal signal to not divide
		prod /= denom;
	return prod;
#else
	Sk64	prod, tmp;

	prod.set(0);
	for (int i = 0; i < count; i++)
	{
		tmp.setMul(a[0], b[0]);
		prod.add(tmp);
		a += step_a;
		b += step_b;
	}
	if (denom)
	{
		prod.div(denom, Sk64::kRound_DivOption);
		return prod.get32();
	}
	else	// denom == 0 is a formal signal to treat the product as a fixed (i.e. denom = SK_Fixed1)
		return prod.getFixed();
#endif
}

//////////////////////////////////////////////////////////////////////////

SkUnitScalar SkPoint3D::normalize(SkUnit3D* unit) const
{
#ifdef SK_SCALAR_IS_FLOAT
	float mag = sk_float_sqrt(fX*fX + fY*fY + fZ*fZ);
	if (mag)
	{
		float scale = 1.0f / mag;
		unit->fX = fX * scale;
		unit->fY = fY * scale;
		unit->fZ = fZ * scale;
	}
#else
	Sk64	tmp1, tmp2;

	tmp1.setMul(fX, fX);
	tmp2.setMul(fY, fY);
	tmp1.add(tmp2);
	tmp2.setMul(fZ, fZ);
	tmp1.add(tmp2);

	SkFixed mag = tmp1.getSqrt();
	if (mag)
	{
		// what if mag < SK_Fixed1 ??? we will underflow the fixdiv
		SkFixed	scale = SkFixedDiv(SK_Fract1, mag);
		unit->fX = SkFixedMul(fX, scale);
		unit->fY = SkFixedMul(fY, scale);
		unit->fZ = SkFixedMul(fZ, scale);
	}
#endif
	return mag;
}

SkUnitScalar SkUnit3D::Dot(const SkUnit3D& a, const SkUnit3D& b)
{
	return	SkUnitScalarMul(a.fX, b.fX) +
			SkUnitScalarMul(a.fY, b.fY) +
			SkUnitScalarMul(a.fZ, b.fZ);
}

void SkUnit3D::Cross(const SkUnit3D& a, const SkUnit3D& b, SkUnit3D* cross)
{
	SkASSERT(cross);

	// use x,y,z, in case &a == cross or &b == cross


	SkScalar x = SkUnitScalarMul(a.fY, b.fZ) - SkUnitScalarMul(a.fZ, b.fY);
	SkScalar y = SkUnitScalarMul(a.fZ, b.fX) - SkUnitScalarMul(a.fX, b.fY);
	SkScalar z = SkUnitScalarMul(a.fX, b.fY) - SkUnitScalarMul(a.fY, b.fX);

	cross->set(x, y, z);
}

///////////////////////////////////////////////////////////////////////////

SkPatch3D::SkPatch3D()
{
	this->reset();
}

void SkPatch3D::reset()
{
	fOrigin.set(0, 0, 0);
	fU.set(SK_Scalar1, 0, 0);
	fV.set(0, -SK_Scalar1, 0);
}

struct SkMatrix3D {
	SkScalar	fMat[3][3];

	void set(int row, SkScalar a, SkScalar b, SkScalar c)
	{
		fMat[row][0] = a;
		fMat[row][1] = b;
		fMat[row][2] = c;
	}
	void setConcat(const SkMatrix3D& a, const SkMatrix3D& b)
	{
		SkMatrix3D	tmp;
		SkMatrix3D*	c = this;

		if (this == &a || this == &b)
			c = &tmp;

		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 3; j++)
				c->fMat[i][j] = VectorMultiplyDivide(3, &a.fMat[i][0], 1, &b.fMat[0][j], 3, 0);

		if (c == &tmp)
			*this = tmp;
	}

	void map(SkPoint3D* v) const
	{
		SkScalar x = VectorMultiplyDivide(3, &fMat[0][0], 1, &v->fX, 1, 0);
		SkScalar y = VectorMultiplyDivide(3, &fMat[1][0], 1, &v->fX, 1, 0);
		SkScalar z = VectorMultiplyDivide(3, &fMat[2][0], 1, &v->fX, 1, 0);
		v->set(x, y, z);
	}
};

void SkPatch3D::rotate(SkScalar radX, SkScalar radY, SkScalar radZ)
{
	SkScalar	s, c;
	SkMatrix3D	mx, my, mz, m;

	s = SkScalarSinCos(radX, &c);
	mx.set(0, SK_Scalar1, 0, 0);
	mx.set(1, 0, c, -s);
	mx.set(2, 0, s, c);

	s = SkScalarSinCos(radY, &c);
	my.set(0, c, 0, -s);
	my.set(1, 0, SK_Scalar1, 0);
	my.set(2, s, 0, c);

	s = SkScalarSinCos(radZ, &c);
	mz.set(0, c, -s, 0);
	mz.set(1, s, c, 0);
	mz.set(2, 0, 0, SK_Scalar1);

	m.setConcat(mx, my);
	m.setConcat(m, mz);

	m.map(&fU);
	m.map(&fV);
}

SkScalar SkPatch3D::dotWith(SkScalar dx, SkScalar dy, SkScalar dz) const
{
	SkUnit3D	normal;

	SkUnit3D::Cross(*(SkUnit3D*)&fU, *(SkUnit3D*)&fV, &normal);
	return	SkUnitScalarMul(normal.fX, dx) +
			SkUnitScalarMul(normal.fY, dy) +
			SkUnitScalarMul(normal.fZ, dz);
}

///////////////////////////////////////////////////////////////////////////

SkCamera3D::SkCamera3D()
{
    fLocation.set(0, 0, -SkIntToScalar(1440));	// 20 inches backward
	fAxis.set(0, 0, SK_Scalar1);				// forward
	fZenith.set(0, -SK_Scalar1, 0);				// up
	fObserver.set(0, 0, -SkIntToScalar(1440));	// 20 inches backward

	this->update();
}

void SkCamera3D::update()
{
	SkUnit3D	axis, zenith, cross;

	fAxis.normalize(&axis);

	{
		SkScalar dot = SkUnit3D::Dot(*(SkUnit3D*)&fZenith, axis);

		zenith.fX = fZenith.fX - SkUnitScalarMul(dot, axis.fX);
		zenith.fY = fZenith.fY - SkUnitScalarMul(dot, axis.fY);
		zenith.fZ = fZenith.fZ - SkUnitScalarMul(dot, axis.fZ);

		(void)((SkPoint3D*)&zenith)->normalize(&zenith);
	}

	SkUnit3D::Cross(axis, zenith, &cross);

	{
		SkScalar* destPtr = (SkScalar*)&fOrientation;
		SkScalar x = fObserver.fX;
		SkScalar y = fObserver.fY;
		SkScalar z = fObserver.fZ;

		destPtr[0] = SkUnitScalarMul(x, axis.fX) - SkUnitScalarMul(z, cross.fX);
		destPtr[1] = SkUnitScalarMul(x, axis.fY) - SkUnitScalarMul(z, cross.fY);
		destPtr[2] = SkUnitScalarMul(x, axis.fZ) - SkUnitScalarMul(z, cross.fZ);
		destPtr[3] = SkUnitScalarMul(y, axis.fX) - SkUnitScalarMul(z, zenith.fX);
		destPtr[4] = SkUnitScalarMul(y, axis.fY) - SkUnitScalarMul(z, zenith.fY);
		destPtr[5] = SkUnitScalarMul(y, axis.fZ) - SkUnitScalarMul(z, zenith.fZ);
		memcpy(&destPtr[6], &axis, sizeof(axis));
    }
}

void SkCamera3D::computeMatrix(const SkPatch3D& quilt, SkMatrix* matrix) const
{
	SkScalar*		destPtr = (SkScalar*)matrix;
	const SkScalar*	mapPtr = (const SkScalar*)&fOrientation;
	const SkScalar*	patchPtr;
	SkPoint3D		diff;
	SkScalar		dot;

	diff.fX = quilt.fOrigin.fX - fLocation.fX;
	diff.fY = quilt.fOrigin.fY - fLocation.fY;
	diff.fZ = quilt.fOrigin.fZ - fLocation.fZ;

	dot = SkUnit3D::Dot(*(const SkUnit3D*)&diff, *(const SkUnit3D*)((const SkScalar*)&fOrientation + 6));

	patchPtr = (const SkScalar*)&quilt;
	destPtr[0] = VectorMultiplyDivide(3, patchPtr, 1, mapPtr, 1, dot);
	destPtr[3] = VectorMultiplyDivide(3, patchPtr, 1, mapPtr+3, 1, dot);
	destPtr[6] = VectorMultiplyDivide(3, patchPtr, 1, mapPtr+6, 1, dot);
	patchPtr += 3;
	destPtr[1] = VectorMultiplyDivide(3, patchPtr, 1, mapPtr, 1, dot);
	destPtr[4] = VectorMultiplyDivide(3, patchPtr, 1, mapPtr+3, 1, dot);
	destPtr[7] = VectorMultiplyDivide(3, patchPtr, 1, mapPtr+6, 1, dot);
	patchPtr = (const SkScalar*)&diff;
	destPtr[2] = VectorMultiplyDivide(3, patchPtr, 1, mapPtr, 1, dot);
	destPtr[5] = VectorMultiplyDivide(3, patchPtr, 1, mapPtr+3, 1, dot);
	destPtr[8] = SK_UnitScalar1;
}
