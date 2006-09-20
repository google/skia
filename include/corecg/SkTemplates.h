#ifndef SkTemplates_DEFINED
#define SkTemplates_DEFINED

#include "SkTypes.h"

/**	\file SkTemplates.h

	This file contains light-weight template classes for type-safe and exception-safe
	resource management.
*/

/** \class SkAutoTCallProc

	Similar to SkAutoTDelete, this class is used to auto delete an object
	when leaving the scope of the object. This is mostly useful when
	errors occur and objects need to be cleaned up. The template uses two
	parameters, the object, and a function that is to be called in the destructor.
	If detach() is called then the function is not called when SkAutoTCallProc goes out
	of scope. This also happens is the passed in object is nil.

*/
template <typename T, void (*P)(T*)> class SkAutoTCallProc {
public:
	SkAutoTCallProc(T* obj): fObj(obj) {}
	~SkAutoTCallProc()
	{
		if (fObj)
			P(fObj);
	}
	T* detach() { T* obj = fObj; fObj = nil; return obj; }
private:
	T* fObj;
};

template <typename T> class SkAutoTDelete {
public:
	SkAutoTDelete(T* obj) : fObj(obj) {}
	~SkAutoTDelete() { delete fObj; }

	void	free() { delete fObj; fObj = nil; }
	T*		detach() { T* obj = fObj; fObj = nil; return obj; }

private:
	T*	fObj;
};

template <typename T> class SkAutoTDeleteArray {
public:
	SkAutoTDeleteArray(T array[]) : fArray(array) {}
	~SkAutoTDeleteArray() { delete[] fArray; }

	void	free() { delete[] fArray; fArray = nil; }
	T*		detach() { T* array = fArray; fArray = nil; return array; }

private:
	T*	fArray;
};

template <typename T> class SkAutoTArray {
public:
	SkAutoTArray(size_t count)
	{
		fArray = nil;	// init first in case we throw
		if (count)
			fArray = new T[count];
#ifdef SK_DEBUG
		fCount = count;
#endif
	}
	~SkAutoTArray()
	{
		delete[] fArray;
	}

	T* get() const { return fArray; }
	T&	operator[](int index) const { SkASSERT((unsigned)index < fCount); return fArray[index]; }

	void reset()
	{
		if (fArray)
		{
			delete[] fArray;
			fArray = nil;
		}
	}

	void replace(T* array)
	{
		if (fArray != array)
		{
			delete[] fArray;
			fArray = array;
		}
	}

	/**	Call swap to exchange your pointer to an array of T with the SkAutoTArray object.
		After this call, the SkAutoTArray object will be responsible for deleting your
		array, and you will be responsible for deleting its.
	*/
	void swap(T*& other)
	{
		T*	tmp = fArray;
		fArray = other;
		other = tmp;
	}

private:
#ifdef SK_DEBUG
	size_t fCount;
#endif
	T*	fArray;
};

/** Allocate a temp array on the stack/heap.
    Does NOT call any constructors/destructors on T (i.e. T must be POD)
*/
template <typename T> class SkAutoTMalloc {
public:
    SkAutoTMalloc(size_t count)
    {
        fPtr = (T*)sk_malloc_flags(count * sizeof(T), SK_MALLOC_THROW | SK_MALLOC_TEMP);
	}
	~SkAutoTMalloc()
	{
        sk_free(fPtr);
	}
	T* get() const { return fPtr; }

private:
	T*  fPtr;
	// illegal
	SkAutoTMalloc(const SkAutoTMalloc&);
	SkAutoTMalloc& operator=(const SkAutoTMalloc&);        
};

template <size_t N, typename T> class SkAutoSTMalloc {
public:
    SkAutoSTMalloc(size_t count)
    {
		if (count <= N)
			fPtr = (T*)fStorage;
		else
			fPtr = (T*)sk_malloc_flags(count * sizeof(T), SK_MALLOC_THROW | SK_MALLOC_TEMP);
	}
	~SkAutoSTMalloc()
	{
		if (fPtr != (T*)fStorage)
			sk_free(fPtr);
	}
	T* get() const { return fPtr; }

private:
	T*          fPtr;
	uint32_t	fStorage[(N*sizeof(T) + 3) >> 2];
	// illegal
	SkAutoSTMalloc(const SkAutoSTMalloc&);
	SkAutoSTMalloc& operator=(const SkAutoSTMalloc&);        
};

#endif

