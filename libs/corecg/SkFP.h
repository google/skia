#ifndef SkFP_DEFINED
#define SkFP_DEFINED

#include "SkMath.h"

#ifdef SK_SCALAR_IS_FLOAT

	typedef float SkFP;

	#define SkIntToFloat(n)			SkIntToScalar(n)
	#define SkFloatRound(x)			SkScalarRound(n)
	#define SkFloatCeil(x)			SkScalarCeil(n)
	#define SkFloatFloor(x)			SkScalarFloor(n)

	#define SkFloatNeg(x)			(-(x))
	#define SkFloatAbs(x)			SkScalarAbs(x)
	#define SkFloatAdd(a, b)		((a) + (b))
	#define SkFloatSub(a, b)		((a) - (b))
	#define SkFloatMul(a, b)		((a) * (b))
	#define SkFloatMulInt(a, n)		((a) * (n))
	#define SkFloatDiv(a, b)		((a) / (b))
	#define SkFloatDivInt(a, n)		((a) / (n))
	#define SkFloatInvert(x)		SkScalarInvert(x)
	#define SkFloatSqrt(x)			SkScalarSqrt(x)
	#define SkFloatCubeRoot(x)		pow(x, 1.0f/3)

	#define SkFloatLT(a, b)			((a) < (b))
	#define SkFloatLE(a, b)			((a) <= (b))
	#define SkFloatGT(a, b)			((a) > (b))
	#define SkFloatGE(a, b)			((a) >= (b))

#else	// scalar is fixed

	#include "SkFloat.h"

	typedef S32	SkFP;

	#define SkIntToFloat(n)			SkFloat::SetShift(n, 0)
	#define SkFloatRound(x)			SkFloat::Round(x);
	#define SkFloatCeil(x)			SkFloat::Ceil();
	#define SkFloatFloor(x)			SkFloat::Floor();

	#define SkScalarToFloat(n)		SkFloat::SetShift(n, -16)
	#define SkFloatToScalar(n)		SkFloat::GetShift(n, -16)
	#define SkFloatNeg(x)			SkFloat::Neg(x)
	#define SkFloatAbs(x)			SkFloat::Abs(x)
	#define SkFloatAdd(a, b)		SkFloat::Add(a, b)
	#define SkFloatSub(a, b)		SkFloat::Add(a, SkFloat::Neg(b))
	#define SkFloatMul(a, b)		SkFloat::Mul(a, b)
	#define SkFloatMulInt(a, n)		SkFloat::MulInt(a, n)
	#define SkFloatDiv(a, b)		SkFloat::Div(a, b)
	#define SkFloatDivInt(a, n)		SkFloat::DivInt(a, n)
	#define SkFloatInvert(x)		SkFloat::Invert(x)
	#define SkFloatSqrt(x)			SkFloat::Sqrt(x)
	#define SkFloatCubeRoot(x)		SkFloat::CubeRoot(x)

	#define SkFloatLT(a, b)			(SkFloat::Cmp(a, b) < 0)
	#define SkFloatLE(a, b)			(SkFloat::Cmp(a, b) <= 0)
	#define SkFloatGT(a, b)			(SkFloat::Cmp(a, b) > 0)
	#define SkFloatGE(a, b)			(SkFloat::Cmp(a, b) >= 0)

#endif

#ifdef SK_DEBUG
	void SkFP_UnitTest();
#endif

#endif
