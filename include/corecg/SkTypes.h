#ifndef SkTypes_DEFINED
#define SkTypes_DEFINED

#include "SkPreConfig.h"
#include "SkUserConfig.h"
#include "SkPostConfig.h"

#include <stdint.h>
#include <stdio.h>

/**	\file SkTypes.h
*/

/*
    memory wrappers
*/

extern void  sk_out_of_memory(void);    // platform specific, does not return
extern void  sk_throw(void);            // platform specific, does not return
enum {
	SK_MALLOC_TEMP	= 0x01,	//!< hint to sk_malloc that the requested memory will be freed in the scope of the stack frame
	SK_MALLOC_THROW	= 0x02	//!< instructs sk_malloc to call sk_throw if the memory cannot be allocated.
};
/**	Return a block of memory (at least 4-byte aligned) of at least the
	specified size. If the requested memory cannot be returned, either
	return nil (if SK_MALLOC_TEMP bit is clear) or call sk_throw()
	(if SK_MALLOC_TEMP bit is set). To free the memory, call sk_free().
*/
extern void* sk_malloc_flags(size_t size, unsigned flags);
/**	Same as sk_malloc(), but hard coded to pass SK_MALLOC_THROW as the flag
*/
extern void* sk_malloc_throw(size_t size);
/**	Same as standard realloc(), but this one never returns nil on failure. It will throw
	an exception if it fails.
*/
extern void* sk_realloc_throw(void* buffer, size_t size);
/**	Free memory returned by sk_malloc(). It is safe to pass nil.
*/
extern void  sk_free(void*);

///////////////////////////////////////////////////////////////////////

#define SK_INIT_TO_AVOID_WARNING	= 0

#ifdef SK_DEBUG
    #define SkASSERT(cond)              SK_DEBUGBREAK(cond)
	#define SkDEBUGCODE(code)			code
	#define SkDECLAREPARAM(type, var)	, type var
	#define SkPARAM(var)				, var
//	#define SkDEBUGF(args		)		SkDebugf##args
	#define SkDEBUGF(args		)		SkDebugf args
	void SkDebugf(const char format[], ...);

	#define SkAssertResult(cond)		SkASSERT(cond)
#else
	#define SkASSERT(cond)
	#define SkDEBUGCODE(code)
	#define SkDEBUGF(args)
	#define SkDECLAREPARAM(type, var)
	#define SkPARAM(var)

	// unlike SkASSERT, this guy executes its condition in the non-debug build
	#define SkAssertResult(cond)		cond
#endif

///////////////////////////////////////////////////////////////////////

#ifndef nil
	#define nil			0
#endif

// legacy defines. will be removed before shipping
typedef int8_t      S8;
typedef uint8_t     U8;
typedef int16_t     S16;
typedef uint16_t    U16;
typedef int32_t     S32;
typedef uint32_t    U32;

/** Fast type for signed 8 bits. Use for parameter passing and local variables, not for storage
*/
typedef int			S8CPU;
/** Fast type for unsigned 8 bits. Use for parameter passing and local variables, not for storage
*/
typedef int			S16CPU;
/** Fast type for signed 16 bits. Use for parameter passing and local variables, not for storage
*/
typedef unsigned	U8CPU;
/** Fast type for unsigned 16 bits. Use for parameter passing and local variables, not for storage
*/
typedef unsigned	U16CPU;

/** Meant to be faster than bool (doesn't promise to be 0 or 1, just 0 or non-zero
*/
typedef int			SkBool;
/**	Meant to be a small version of bool, for storage purposes. Will be 0 or 1
*/
typedef uint8_t		SkBool8;

#ifdef SK_DEBUG
	int8_t      SkToS8(long);
	uint8_t     SkToU8(size_t);
	int16_t     SkToS16(long);
	uint16_t	SkToU16(size_t);
	int32_t     SkToS32(long);
	uint32_t	SkToU32(size_t);
#else
	#define SkToS8(x)	((int8_t)(x))
	#define SkToU8(x)	((uint8_t)(x))
	#define SkToS16(x)	((int16_t)(x))
	#define SkToU16(x)	((uint16_t)(x))
	#define SkToS32(x)	((int32_t)(x))
	#define SkToU32(x)	((uint32_t)(x))
#endif

/**	Returns 0 or 1 based on the condition
*/
#define SkToBool(cond)	((cond) != 0)

#define SK_MaxS16	32767
#define SK_MinS16	-32767
#define SK_MaxU16	0xFFFF
#define SK_MinU16	0
#define SK_MaxS32	0x7FFFFFFF
#define SK_MinS32	0x80000001
#define SK_MaxU32	0xFFFFFFFF
#define SK_MinU32	0
#define SK_NaN32	0x80000000

#ifndef SK_OFFSETOF
	#define SK_OFFSETOF(type, field)	((char*)&(((type*)1)->field) - (char*)1)
#endif

/**	Returns the number of entries in an array (not a pointer)
*/
#define SK_ARRAY_COUNT(array)		(sizeof(array) / sizeof(array[0]))

/**	Returns x rounded up to a multiple of 2
*/
#define SkAlign2(x)		(((x) + 1) >> 1 << 1)
/** Returns x rounded up to a multiple of 4
*/
#define SkAlign4(x)		(((x) + 3) >> 2 << 2)

typedef uint32_t SkFourByteTag;
#define SkSetFourByteTag(a, b, c, d)	(((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

/**	32 bit integer to hold a unicode value
*/
typedef int32_t	SkUnichar;
/**	32 bit value to hold a millisecond count
*/
typedef uint32_t SkMSec;
/**	1 second measured in milliseconds
*/
#define SK_MSec1 1000
/**	maximum representable milliseconds
*/
#define SK_MSecMax 0x7FFFFFFF
/**	Returns a < b for milliseconds, correctly handling wrap-around from 0xFFFFFFFF to 0
*/
#define SkMSec_LT(a, b)		((int32_t)(a) - (int32_t)(b) < 0)
/**	Returns a <= b for milliseconds, correctly handling wrap-around from 0xFFFFFFFF to 0
*/
#define SkMSec_LE(a, b)		((int32_t)(a) - (int32_t)(b) <= 0)


/****************************************************************************
	The rest of these only build with C++
*/
#ifdef __cplusplus

/**	Faster than SkToBool for integral conditions. Returns 0 or 1
*/
inline int Sk32ToBool(uint32_t n)
{
	return (n | (0-n)) >> 31;
}

template <typename T> inline void SkTSwap(T& a, T& b)
{
	T c(a);
	a = b;
	b = c;
}

inline int32_t SkAbs32(int32_t value)
{
#ifdef SK_CPU_HAS_CONDITIONAL_INSTR
	if (value < 0)
		value = -value;
	return value;
#else
	int32_t mask = value >> 31;
	return (value ^ mask) - mask;
#endif
}

inline int32_t SkMax32(int32_t a, int32_t b)
{
	if (a < b)
		a = b;
	return a;
}

inline int32_t SkMin32(int32_t a, int32_t b)
{
	if (a > b)
		a = b;
	return a;
}

inline int32_t SkSign32(int32_t a)
{
	return (a >> 31) | ((unsigned) -a >> 31);
}

inline int32_t SkFastMin32(int32_t value, int32_t max)
{
#ifdef SK_CPU_HAS_CONDITIONAL_INSTR
	if (value > max)
		value = max;
	return value;
#else
	int diff = max - value;
	// clear diff if it is negative (clear if value > max)
	diff &= (diff >> 31);
	return value + diff;
#endif
}

/**	Returns signed 32 bit value pinned between min and max, inclusively
*/
inline int32_t SkPin32(int32_t value, int32_t min, int32_t max)
{
#ifdef SK_CPU_HAS_CONDITIONAL_INSTR
	if (value < min)
		value = min;
	if (value > max)
		value = max;
#else
	if (value < min)
		value = min;
	else if (value > max)
		value = max;
#endif
	return value;
}

inline uint32_t SkSetClear32(uint32_t flags, bool cond, unsigned shift)
{
	return flags & ~(1 << shift) | ((int)cond << shift);
}

class SkAutoMalloc {
public:
	SkAutoMalloc(size_t size)
	{
		fPtr = sk_malloc_flags(size, SK_MALLOC_THROW | SK_MALLOC_TEMP);
	}
	~SkAutoMalloc()
	{
		sk_free(fPtr);
	}
	void* get() const { return fPtr; }
private:
	void* fPtr;
	// illegal
	SkAutoMalloc(const SkAutoMalloc&);
	SkAutoMalloc& operator=(const SkAutoMalloc&);
};

template <size_t kSize> class SkAutoSMalloc {
public:
	SkAutoSMalloc(size_t size)
	{
		if (size <= kSize)
			fPtr = fStorage;
		else
			fPtr = sk_malloc_flags(size, SK_MALLOC_THROW | SK_MALLOC_TEMP);
	}
	~SkAutoSMalloc()
	{
		if (fPtr != (void*)fStorage)
			sk_free(fPtr);
	}
	void* get() const { return fPtr; }
private:
	void*       fPtr;
	uint32_t	fStorage[(kSize + 3) >> 2];
	// illegal
	SkAutoSMalloc(const SkAutoSMalloc&);
	SkAutoSMalloc& operator=(const SkAutoSMalloc&);
};

#endif /* C++ */

#endif

