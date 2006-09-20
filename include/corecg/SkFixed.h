#ifndef SkFixed_DEFINED
#define SkFixed_DEFINED

/**	\file SkFixed.h

	Types and macros for 16.16 fixed point
*/

/**	32 bit signed integer used to represent fractions values with 16 bits to the right of the decimal point
*/
typedef int32_t				SkFixed;
#define SK_Fixed1			(1 << 16)
#define SK_FixedHalf		(1 << 15)
#define SK_FixedMax			(0x7FFFFFFF)
#define SK_FixedMin			(0x1)
#define SK_FixedNaN			((int) 0x80000000)
#define SK_FixedPI			(0x3243F)
#define SK_FixedSqrt2		(92682)
#define SK_FixedTanPIOver8	(0x6A0A)
#define SK_FixedRoot2Over2	(0xB505)

#ifdef SK_CAN_USE_FLOAT
	#define SkFixedToFloat(x)	((x) * 1.5258789e-5f)
	#define SkFloatToFixed(x)	((SkFixed)((x) * SK_Fixed1))
#endif

/**	32 bit signed integer used to represent fractions values with 30 bits to the right of the decimal point
*/
typedef int32_t             SkFract;
#define SK_Fract1			(1 << 30)
#define Sk_FracHalf			(1 << 29)
#define SK_FractPIOver180	(0x11DF46A)

#ifdef SK_CAN_USE_FLOAT
	#define SkFractToFloat(x)	((float)(x) * 0.00000000093132257f)
	#define SkFloatToFract(x)	((SkFract)((x) * SK_Fract1))
#endif

/**	Converts an integer to a SkFixed, asserting that the result does not overflow
	a 32 bit signed integer
*/
#ifdef SK_DEBUG
	inline SkFixed SkIntToFixed(int n)
	{
		SkASSERT(n >= -32768 && n <= 32767);
		return n << 16;
	}
#else
	//	force the cast to SkFixed to ensure that the answer is signed (like the debug version)
	#define SkIntToFixed(n)		(SkFixed)((n) << 16)
#endif

/**	Converts a SkFixed to a SkFract, asserting that the result does not overflow
	a 32 bit signed integer
*/
#ifdef SK_DEBUG
	inline SkFract SkFixedToFract(SkFixed x)
	{
		SkASSERT(x >= (-2 << 16) && x <= (2 << 16) - 1);
		return x << 14;
	}
#else
	#define SkFixedToFract(x)	((x) << 14)
#endif

/**	Returns the signed fraction of a SkFixed
*/
inline SkFixed SkFixedFraction(SkFixed x)
{
	SkFixed mask = x >> 31 << 16;
	return x & 0xFFFF | mask;
}

/**	Converts a SkFract to a SkFixed
*/
#define SkFractToFixed(x)	((x) >> 14)
/**	Round a SkFixed to an integer
*/
#define SkFixedRound(x)		(((x) + SK_FixedHalf) >> 16)
#define SkFixedCeil(x)		(((x) + SK_Fixed1 - 1) >> 16)
#define SkFixedFloor(x)		((x) >> 16)
#define SkFixedAbs(x)		SkAbs32(x)
#define SkFixedAve(a, b)	(((a) + (b)) >> 1)

#if defined(SK_BUILD_FOR_BREW) && !defined(AEE_SIMULATOR)
	inline SkFixed SkFixedSquare(SkFixed a)
	{
		SkFixed	answer;
		asm volatile ( "SMULL r6, r7, %0, %0" : : "r"(a) : "r6", "r7" );
		asm volatile ( "MOV	 r6, r6, LSR #16" );
		asm volatile ( "ORR	 r6, r6, r7, LSL #16" );
		asm volatile ( "STR	 r6, %0" : "=m"(answer) );
		return answer;
	}
	inline SkFixed SkFixedMul(SkFixed a, SkFixed b)
	{
		SkFixed	answer;
		asm volatile ( "SMULL r6, r7, %0, %1" : : "r"(a), "r"(b) : "r6", "r7" );
		asm volatile ( "MOV	 r6, r6, LSR #16" );
		asm volatile ( "ORR	 r6, r6, r7, LSL #16" );
		asm volatile ( "STR	 r6, %0" : "=m"(answer) );
		return answer;
	}
	inline SkFract SkFractMul(SkFract a, SkFract b)
	{
		SkFract	answer;
		asm volatile ( "SMULL r6, r7, %0, %1" : : "r"(a), "r"(b) : "r6", "r7" );
		asm volatile ( "MOV	 r6, r6, LSR #30" );
		asm volatile ( "ORR	 r6, r6, r7, LSL #2" );
		asm volatile ( "STR	 r6, %0" : "=m"(answer) );
		return answer;
	}
#else
	inline SkFixed SkFixedSquare(SkFixed value)
	{
		uint32_t a = SkAbs32(value);
		uint32_t ah = a >> 16;
		uint32_t al = a & 0xFFFF;
		return ah * a + al * ah + (al * al >> 16);
	}
	SkFixed SkFixedMul(SkFixed, SkFixed);
	SkFract	SkFractMul(SkFract, SkFract);
#endif
#define SkFixedDiv(numer, denom)	SkDivBits(numer, denom, 16)
SkFixed SkFixedDivInt(int32_t numer, int32_t denom);
SkFixed SkFixedMod(SkFixed numer, SkFixed denom);
#define SkFixedInvert(n)			SkDivBits(SK_Fixed1, n, 16)
#define SkFixedSqrt(n)				SkSqrtBits(n, 23)
SkFixed SkFixedMean(SkFixed a, SkFixed b);  //*< returns sqrt(x*y)
int SkFixedMulCommon(SkFixed, int , int bias);	// internal used by SkFixedMulFloor, SkFixedMulCeil, SkFixedMulRound

#define SkFractDiv(numer, denom)	SkDivBits(numer, denom, 30)
#define SkFractSqrt(n)				SkSqrtBits(n, 30)

SkFixed SkFixedSinCos(SkFixed radians, SkFixed* cosValueOrNil);
#define SkFixedSin(radians)			SkFixedSinCos(radians, nil)
inline SkFixed SkFixedCos(SkFixed radians)
{
	SkFixed	cosValue;
	(void)SkFixedSinCos(radians, &cosValue);
	return cosValue;
}
SkFixed SkFixedTan(SkFixed radians);
SkFixed SkFixedASin(SkFixed);
SkFixed SkFixedACos(SkFixed);
SkFixed SkFixedATan2(SkFixed y, SkFixed x);
SkFixed SkFixedExp(SkFixed);
SkFixed SkFixedLog(SkFixed);

#define SK_FixedNearlyZero			(SK_Fixed1 >> 12)

inline bool SkFixedNearlyZero(SkFixed x, SkFixed tolerance = SK_FixedNearlyZero)
{
	SkASSERT(tolerance > 0);
	return SkAbs32(x) < tolerance;
}

#endif

