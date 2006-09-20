#ifndef SkTDArray_DEFINED
#define SkTDArray_DEFINED

#include "SkTypes.h"

template <typename T> class SkTDArray {
public:
	SkTDArray()
	{
		fReserve = fCount = 0;
		fArray = NULL;
#ifdef SK_DEBUG
		fData = NULL;
#endif
	}
	SkTDArray(const T src[], U16CPU count)
	{
		SkASSERT(src || count == 0);

		fReserve = fCount = 0;
		fArray = NULL;
#ifdef SK_DEBUG
		fData = NULL;
#endif
		if (count)
		{
			fArray = (T*)sk_malloc_throw(count * sizeof(T));
#ifdef SK_DEBUG
		//	fData = (T (*)[kDebugArraySize]) fArray;
			(T*&)fData = fArray;
#endif
			memcpy(fArray, src, sizeof(T) * count);
			fReserve = fCount = SkToU16(count);
		}
	}
	SkTDArray(const SkTDArray<T>& src)
	{
		fReserve = fCount = 0;
		fArray = NULL;
#ifdef SK_DEBUG
		fData = NULL;
#endif
		SkTDArray<T> tmp(src.fArray, src.fCount);
		this->swap(tmp);
	}
	~SkTDArray()
	{
		sk_free(fArray);
	}

	SkTDArray<T>& operator=(const SkTDArray<T>& src)
	{
		if (this != &src)
		{
			if (src.fCount > fReserve)
			{
				SkTDArray<T> tmp(src.fArray, src.fCount);
				this->swap(tmp);
			}
			else
			{
				memcpy(fArray, src.fArray, sizeof(T) * src.fCount);
				fCount = src.fCount;
			}
		}
		return *this;
	}

	friend int operator==(const SkTDArray<T>& a, const SkTDArray<T>& b)
	{
		return	a.fCount == b.fCount &&
				(a.fCount == 0 || !memcmp(a.fArray, b.fArray, a.fCount * sizeof(T)));
	}

	void swap(SkTDArray<T>& other)
	{
		SkTSwap(fArray, other.fArray);
#ifdef SK_DEBUG
		SkTSwap(fData, other.fData);
#endif
		SkTSwap(fReserve, other.fReserve);
		SkTSwap(fCount, other.fCount);
	}

	bool isEmpty() const { return fCount == 0; }
	int	count() const { return fCount; }
	T*	begin() const { return fArray; }
	T*	end() const { return fArray ? fArray + fCount : NULL; }
	T&	operator[](int index) const { SkASSERT((unsigned)index < fCount); return fArray[index]; }

	void reset()
	{
		if (fArray)
		{
			sk_free(fArray);
			fArray = NULL;
#ifdef SK_DEBUG
			fData = NULL;
#endif
			fReserve = fCount = 0;
		}
		else
		{
			SkASSERT(fReserve == 0 && fCount == 0);
		}
	}

	void setCount(U16CPU count)
	{
		if (count > fReserve)
			this->growBy(count - fCount);
		else
			fCount = SkToU16(count);
	}

	void setReserve(U16CPU reserve)
	{
		if (reserve > fReserve)
		{
			SkASSERT(reserve > fCount);
			U16	count = fCount;
			this->growBy(reserve - fCount);
			fCount = count;
		}
	}

	T* prepend()
	{
		this->growBy(1);
		memmove(fArray + 1, fArray, (fCount - 1) * sizeof(T));
		return fArray;
	}

	T* append() { return this->append(1, NULL); }
	T* append(U16CPU count, const T* src = NULL)
	{
		unsigned oldCount = fCount;
		if (count)
		{
			SkASSERT(src == NULL || fArray == NULL ||
					src + count <= fArray || fArray + oldCount <= src);

			this->growBy(count);
			if (src)
				memcpy(fArray + oldCount, src, sizeof(T) * count);
		}
		return fArray + oldCount;
	}
	
	T* appendClear()
	{
		T* result = this->append(); 
		*result = 0;
		return result;
	}

	T* insert(U16CPU index) { return this->insert(index, 1, NULL); }
	T* insert(U16CPU index, U16CPU count, const T* src = NULL)
	{
		SkASSERT(count);
		int oldCount = fCount;
		this->growBy(count);
		T* dst = fArray + index;
		memmove(dst + count, dst, sizeof(T) * (oldCount - index));
		if (src)
			memcpy(dst, src, sizeof(T) * count);
		return dst;
	}

	void remove(U16CPU index, U16CPU count = 1)
	{
		SkASSERT(index + count <= fCount);
		fCount = SkToU16(fCount - count);
		memmove(fArray + index, fArray + index + count, sizeof(T) * (fCount - index));
	}

	void removeShuffle(U16CPU index)
	{
		SkASSERT(index < fCount);
		unsigned newCount = fCount - 1;
		fCount = SkToU16(newCount);
		if (index != newCount)
			memcpy(fArray + index, fArray + newCount, sizeof(T));
	}

	int find(const T& elem) const
	{
		const T* iter = fArray;
		const T* stop = fArray + fCount;

		for (; iter < stop; iter++)
		{
			if (*iter == elem)
				return (int) (iter - fArray);
		}
		return -1;
	}

	int rfind(const T& elem) const
	{
		const T* iter = fArray + fCount;
		const T* stop = fArray;

		while (iter > stop)
		{
			if (*--iter == elem)
				return iter - stop;
		}
		return -1;
	}

	// routines to treat the array like a stack
	T*			push() { return this->append(); }
	void		push(T& elem) { *this->append() = elem; }
	const T&	top() const { return (*this)[fCount - 1]; }
	T&			top() { return (*this)[fCount - 1]; }
	void		pop(T* elem) { if (elem) *elem = (*this)[fCount - 1]; --fCount; }
	void		pop() { --fCount; }

	void deleteAll()
	{
		T*	iter = fArray;
		T*	stop = fArray + fCount;
		while (iter < stop)
		{
			delete (*iter);
			iter += 1;
		}
		this->reset();
	}

	void freeAll()
	{
		T*	iter = fArray;
		T*	stop = fArray + fCount;
		while (iter < stop)
		{
			sk_free(*iter);
			iter += 1;
		}
		this->reset();
	}

	void unrefAll()
	{
		T*	iter = fArray;
		T*	stop = fArray + fCount;
		while (iter < stop)
		{
			(*iter)->unref();
			iter += 1;
		}
		this->reset();
	}

private:
#ifdef SK_DEBUG
    enum {
        kDebugArraySize = 16
    };
	T(* fData)[kDebugArraySize];
#endif
	T*	fArray;
	U16	fReserve, fCount;

	void growBy(U16CPU extra)
	{
		SkASSERT(extra);
		SkASSERT(fCount + extra <= 0xFFFF);

		if (fCount + extra > fReserve)
		{
			size_t size = fCount + extra + 4;
			size += size >> 2;

			fArray = (T*)sk_realloc_throw(fArray, size * sizeof(T));
#ifdef SK_DEBUG
		//	fData = (T (*)[kDebugArraySize]) fArray;
			(T*&)fData = fArray;
#endif
			fReserve = SkToU16((U16CPU)size);
		}
		fCount = SkToU16(fCount + extra);
	}
};

#endif

