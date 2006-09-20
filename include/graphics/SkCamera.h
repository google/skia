#ifndef SkCamera_DEFINED
#define SkCamera_DEFINED

#include "SkMatrix.h"
#include "Sk64.h"

#ifdef SK_SCALAR_IS_FIXED
	typedef SkFract	SkUnitScalar;
	#define SK_UnitScalar1			SK_Fract1
	#define SkUnitScalarMul(a, b)	SkFractMul(a, b)
	#define SkUnitScalarDiv(a, b)	SkFractDiv(a, b)
#else
	typedef float	SkUnitScalar;
	#define SK_UnitScalar1			SK_Scalar1
	#define SkUnitScalarMul(a, b)	SkScalarMul(a, b)
	#define SkUnitScalarDiv(a, b)	SkScalarDiv(a, b)
#endif

//	Taken from Rob Johnson's most excellent QuickDraw GX library

struct SkUnit3D {
	SkUnitScalar	fX, fY, fZ;

	void set(SkUnitScalar x, SkUnitScalar y, SkUnitScalar z)
	{
		fX = x; fY = y; fZ = z;
	}
	static SkUnitScalar Dot(const SkUnit3D&, const SkUnit3D&);
	static void Cross(const SkUnit3D&, const SkUnit3D&, SkUnit3D* cross);
};

struct SkPoint3D {
	SkScalar	fX, fY, fZ;

	void set(SkScalar x, SkScalar y, SkScalar z)
	{
		fX = x; fY = y; fZ = z;
	}
	SkScalar	normalize(SkUnit3D*) const;
};

class SkPatch3D {
public:
	SkPatch3D();

	void	reset();
	void	rotate(SkScalar radX, SkScalar radY, SkScalar radZ);
	void	rotateDegrees(SkScalar degX, SkScalar degY, SkScalar degZ)
	{
		this->rotate(SkDegreesToRadians(degX),
					 SkDegreesToRadians(degY),
					 SkDegreesToRadians(degZ));
	}

	// dot a unit vector with the patch's normal
	SkScalar	dotWith(SkScalar dx, SkScalar dy, SkScalar dz) const;

	SkPoint3D	fU, fV, fOrigin;
private:
	friend class SkCamera3D;
};

class SkCamera3D {
public:
	SkCamera3D();

	void update();
	void computeMatrix(const SkPatch3D&, SkMatrix* matrix) const;

	SkPoint3D	fLocation;
	SkPoint3D	fAxis;
	SkPoint3D	fZenith;
	SkPoint3D	fObserver;

private:
	SkMatrix	fOrientation;
};

#endif

