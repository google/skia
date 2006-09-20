#ifndef SkTSearch_DEFINED
#define SkTSearch_DEFINED

#include "SkTypes.h"

template <typename T>
int SkTSearch(const T* base, int count, T target, size_t elemSize)
{
	SkASSERT(base != nil);
	SkASSERT(count >= 0);

	if (count <= 0)
		return ~0;

	int	lo = 0;
	int	hi = count - 1;

	while (lo < hi)
	{
		int mid = (hi + lo) >> 1;
		const T* elem = (const T*)((const char*)base + mid * elemSize);

		if (*elem < target)
			lo = mid + 1;
		else
			hi = mid;
	}

	const T* elem = (const T*)((const char*)base + hi * elemSize);
	if (*elem != target)
	{
		if (*elem < target)
			hi += 1;
		hi = ~hi;
	}
	return hi;
}

int SkStrSearch(const char*const* base, int count, const char target[], size_t target_len, size_t elemSize);
int SkStrSearch(const char*const* base, int count, const char target[], size_t elemSize);

/**	Like SkStrSearch, but treats target as if it were all lower-case. Assumes that
	base points to a table of lower-case strings.
*/
int SkStrLCSearch(const char*const* base, int count, const char target[], size_t target_len, size_t elemSize);
int SkStrLCSearch(const char*const* base, int count, const char target[], size_t elemSize);

extern "C" {
	typedef int (*SkQSortCompareProc)(const void*, const void*);
	void SkQSort(void* base, size_t count, size_t elemSize, SkQSortCompareProc);
}

SkDEBUGCODE(void SkQSort_UnitTest();)

#endif

