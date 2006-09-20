#ifndef SkMath_DEFINED
#define SkMath_DEFINED

#include "SkTypes.h"

/** \file SkMath.h

	This file defines various math types and functions. It also introduces
	SkScalar, the type used to describe fractional values and coordinates.
	SkScalar is defined at compile time to be either an IEEE float, or a
	16.16 fixed point integer. Various macros and functions in SkMath.h
	allow arithmetic operations to be performed on SkScalars without known
	which representation is being used. e.g. SkScalarMul(a, b) multiplies
	two SkScalar values, and returns a SkScalar, and this works with either
	float or fixed implementations.
*/

//#if defined(SK_BUILD_FOR_BREW) && !defined(AEE_SIMULATOR)
#if 0
	inline int SkCLZ(uint32_t value)
	{
		int	answer;
		asm volatile ( "CLZ r6, %0" : : "r"(value) : "r6" );
		asm volatile ( "STR r6, %0" : "=m"(answer) );
		return answer;
	}
#else
	int	SkCLZ(uint32_t);	//<! Returns the number of leading zero bits (0...32)
#endif

/**	Computes the 64bit product of a * b, and then shifts the answer down by
	shift bits, returning the low 32bits. shift must be [0..63]
	e.g. to perform a fixedmul, call SkMulShift(a, b, 16)
*/
int32_t SkMulShift(int32_t a, int32_t b, unsigned shift);
/**	Computes numer1 * numer2 / denom in full 64 intermediate precision.
	It is an error for denom to be 0. There is no special handling if
	the result overflows 32bits.
*/
int32_t SkMulDiv(int32_t numer1, int32_t numer2, int32_t denom);
/**	Computes (numer1 << shift) / denom in full 64 intermediate precision.
	It is an error for denom to be 0. There is no special handling if
	the result overflows 32bits.
*/
int32_t SkDivBits(int32_t numer, int32_t denom, int shift);
int32_t SkSqrtBits(int32_t value, int bits);
#define SkSqrt32(n)			SkSqrtBits(n, 15)
int32_t SkCubeRootBits(int32_t value, int bits);

/**	Returns -1 if n < 0, else returns 0
*/
#define SkExtractSign(n)	((int32_t)(n) >> 31)

/**	If sign == -1, returns -n, else sign must be 0, and returns n.
	Typically used in conjunction with SkExtractSign().
*/
inline int32_t SkApplySign(int32_t n, int32_t sign)
{
	SkASSERT(sign == 0 || sign == -1);
	return (n ^ sign) - sign;
}

/**	Returns max(value, 0)
*/
inline int SkClampPos(int value)
{
	return value & ~(value >> 31);
}

/**	Given an integer and a positive (max) integer, return the value
	pinned against 0 and max, inclusive.
	Note: only works as long as max - value doesn't wrap around
	@param value	The value we want returned pinned between [0...max]
	@param max		The positive max value
	@return 0 if value < 0, max if value > max, else value
*/
inline int SkClampMax(int value, int max)
{
	// ensure that max is positive
	SkASSERT(max >= 0);
	// ensure that if value is negative, max - value doesn't wrap around
	SkASSERT(value >= 0 || max - value > 0);

#ifdef SK_CPU_HAS_CONDITIONAL_INSTR
	if (value < 0)
		value = 0;
	if (value > max)
		value = max;
	return value;
#else

	int	diff = max - value;
	// clear diff if diff is positive
	diff &= diff >> 31;

	// clear the result if value < 0
	return (value + diff) & ~(value >> 31);
#endif
}

/**	Given a positive value and a positive max, return the value
	pinned against max.
	Note: only works as long as max - value doesn't wrap around
	@return max if value >= max, else value
*/
inline unsigned SkClampUMax(unsigned value, unsigned max)
{
#ifdef SK_CPU_HAS_CONDITIONAL_INSTR
	if (value > max)
		value = max;
	return value;
#else
	int	diff = max - value;
	// clear diff if diff is positive
	diff &= diff >> 31;

	return value + diff;
#endif
}

#include "SkFixed.h"
#include "SkScalar.h"

#ifdef SK_DEBUG
	class SkMath {
	public:
		static void UnitTest();
	};
#endif

#endif

