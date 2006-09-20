#ifndef SkFloat_DEFINED
#define SkFloat_DEFINED

#include "SkMath.h"

class SkFloat {
public:
			SkFloat() {}

	void	setZero() { fPacked = 0; }
//	void	setShift(int value, int shift) { fPacked = SetShift(value, shift); }
	void	setInt(int value) { fPacked = SetShift(value, 0); }
	void	setFixed(SkFixed value) { fPacked = SetShift(value, -16); }
	void	setFract(SkFract value) { fPacked = SetShift(value, -30); }

//	int		getShift(int shift) const { return GetShift(fPacked, shift); }
	int		getInt() const { return GetShift(fPacked, 0); }
	SkFixed	getFixed() const { return GetShift(fPacked, -16); }
	SkFract	getFract() const { return GetShift(fPacked, -30); }

	void	abs() { fPacked = Abs(fPacked); }
	void	negate() { fPacked = Neg(fPacked); }

	void	shiftLeft(int bits) { fPacked = Shift(fPacked, bits); }
	void	setShiftLeft(const SkFloat& a, int bits) { fPacked = Shift(a.fPacked, bits); }

	void	shiftRight(int bits) { fPacked = Shift(fPacked, -bits); }
	void	setShiftRight(const SkFloat& a, int bits) { fPacked = Shift(a.fPacked, -bits); }

	void	add(const SkFloat& a) { fPacked = Add(fPacked, a.fPacked); }
	void	setAdd(const SkFloat& a, const SkFloat& b) { fPacked = Add(a.fPacked, b.fPacked); }

	void	sub(const SkFloat& a) { fPacked = Add(fPacked, Neg(a.fPacked)); }
	void	setSub(const SkFloat& a, const SkFloat& b) { fPacked = Add(a.fPacked, Neg(b.fPacked)); }

	void	mul(const SkFloat& a) { fPacked = Mul(fPacked, a.fPacked); }
	void	setMul(const SkFloat& a, const SkFloat& b) { fPacked = Mul(a.fPacked, b.fPacked); }

	void	div(const SkFloat& a) { fPacked = Div(fPacked, a.fPacked); }
	void	setDiv(const SkFloat& a, const SkFloat& b) { fPacked = Div(a.fPacked, b.fPacked); }

	void	sqrt() { fPacked = Sqrt(fPacked); }
	void	setSqrt(const SkFloat& a) { fPacked = Sqrt(a.fPacked); }
	void	cubeRoot() { fPacked = CubeRoot(fPacked); }
	void	setCubeRoot(const SkFloat& a) { fPacked = CubeRoot(a.fPacked); }

	friend bool operator==(const SkFloat& a, const SkFloat& b) { return a.fPacked == b.fPacked; }
	friend bool operator!=(const SkFloat& a, const SkFloat& b) { return a.fPacked != b.fPacked; }
	friend bool operator<(const SkFloat& a, const SkFloat& b) { return Cmp(a.fPacked, b.fPacked) < 0; }
	friend bool operator<=(const SkFloat& a, const SkFloat& b) { return Cmp(a.fPacked, b.fPacked) <= 0; }
	friend bool operator>(const SkFloat& a, const SkFloat& b) { return Cmp(a.fPacked, b.fPacked) > 0; }
	friend bool operator>=(const SkFloat& a, const SkFloat& b) { return Cmp(a.fPacked, b.fPacked) >= 0; }

#ifdef SK_DEBUG
	static void UnitTest();

	void assertEquals(float f, int tolerance = 0)
	{
		S32	s = *(S32*)&f;
		int d = s - fPacked;
		SkASSERT(SkAbs32(d) <= tolerance);
	}
	float getFloat() const
	{
		return *(float*)&fPacked;
	}
#endif

private:
	S32	fPacked;

	SkFloat(S32 packed) : fPacked(fPacked) {}

public:
	static int GetShift(S32 packed, int shift);
	static S32 SetShift(int value, int shift);
	static S32 Neg(S32);
	static S32 Abs(S32 packed) { return (U32)(packed << 1) >> 1; }
	static S32 Shift(S32, int bits);
	static S32 Add(S32, S32);
	static S32 Mul(S32, S32);
	static S32 MulInt(S32, int);
	static S32 Div(S32, S32);
	static S32 DivInt(S32, int);
	static S32 Invert(S32);
	static S32 Sqrt(S32);
	static S32 CubeRoot(S32);
	static int Cmp(S32, S32);
};

#endif
