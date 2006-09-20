#include "SkTypedArray.h"

SkTypedArray::SkTypedArray() : fType(SkType_Unknown) {
}

SkTypedArray::SkTypedArray(SkDisplayTypes type) : fType(type) {
}

bool SkTypedArray::getIndex(int index, SkOperand* operand) {
	if (index >= count()) {
		SkASSERT(0);
		return false;
	}
	*operand = begin()[index];
	return true;
}


#if SK_SMALLER_ARRAY_TEMPLATE_EXPERIMENT == 1
SkDS32Array::SkDS32Array()
{
	fReserve = fCount = 0;
	fArray = nil;
#ifdef SK_DEBUG
	fData = nil;
#endif
}

SkDS32Array::SkDS32Array(const SkDS32Array& src)
{
	fReserve = fCount = 0;
	fArray = nil;
#ifdef SK_DEBUG
	fData = nil;
#endif
	SkDS32Array tmp(src.fArray, src.fCount);
	this->swap(tmp);
}

SkDS32Array::SkDS32Array(const S32 src[], U16CPU count)
{
	SkASSERT(src || count == 0);

	fReserve = fCount = 0;
	fArray = nil;
#ifdef SK_DEBUG
	fData = nil;
#endif
	if (count)
	{
		fArray = (S32*)sk_malloc_throw(count * sizeof(S32));
#ifdef SK_DEBUG
		fData = (S32 (*)[kDebugArraySize]) fArray;
#endif
		memcpy(fArray, src, sizeof(S32) * count);
		fReserve = fCount = SkToU16(count);
	}
}

SkDS32Array& SkDS32Array::operator=(const SkDS32Array& src)
{
	if (this != &src)
	{
		if (src.fCount > fReserve)
		{
			SkDS32Array tmp(src.fArray, src.fCount);
			this->swap(tmp);
		}
		else
		{
			memcpy(fArray, src.fArray, sizeof(S32) * src.fCount);
			fCount = src.fCount;
		}
	}
	return *this;
}

int operator==(const SkDS32Array& a, const SkDS32Array& b)
{
	return a.fCount == b.fCount &&
			(a.fCount == 0 || !memcmp(a.fArray, b.fArray, a.fCount * sizeof(S32)));
}

void SkDS32Array::swap(SkDS32Array& other)
{
	SkTSwap(fArray, other.fArray);
#ifdef SK_DEBUG
	SkTSwap(fData, other.fData);
#endif
	SkTSwap(fReserve, other.fReserve);
	SkTSwap(fCount, other.fCount);
}

S32* SkDS32Array::append(U16CPU count, const S32* src)
{
	unsigned oldCount = fCount;
	if (count)
	{
		SkASSERT(src == nil || fArray == nil ||
				src + count <= fArray || fArray + count <= src);

		this->growBy(count);
		if (src)
			memcpy(fArray + oldCount, src, sizeof(S32) * count);
	}
	return fArray + oldCount;
}

int SkDS32Array::find(const S32& elem) const
{
	const S32* iter = fArray;
	const S32* stop = fArray + fCount;

	for (; iter < stop; iter++)
	{
		if (*iter == elem)
			return (int) (iter - fArray);
	}
	return -1;
}

void SkDS32Array::growBy(U16CPU extra)
{
	SkASSERT(extra);
	SkASSERT(fCount + extra <= 0xFFFF);

	if (fCount + extra > fReserve)
	{
		size_t size = fCount + extra + 4;
		size += size >> 2;
		S32* array = (S32*)sk_malloc_throw(size * sizeof(S32));
		memcpy(array, fArray, fCount * sizeof(S32));

		sk_free(fArray);
		fArray = array;
#ifdef SK_DEBUG
		fData = (S32 (*)[kDebugArraySize]) fArray;
#endif
		fReserve = SkToU16((U16CPU)size);
	}
	fCount = SkToU16(fCount + extra);
}

S32* SkDS32Array::insert(U16CPU index, U16CPU count, const S32* src)
{
	SkASSERT(count);
	int oldCount = fCount;
	this->growBy(count);
	S32* dst = fArray + index;
	memmove(dst + count, dst, sizeof(S32) * (oldCount - index));
	if (src)
		memcpy(dst, src, sizeof(S32) * count);
	return dst;
}


	int SkDS32Array::rfind(const S32& elem) const
	{
		const S32* iter = fArray + fCount;
		const S32* stop = fArray;

		while (iter > stop)
		{
			if (*--iter == elem)
				return (int) (iter - stop);
		}
		return -1;
	}

#endif
