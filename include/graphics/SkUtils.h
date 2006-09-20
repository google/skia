#ifndef SkUtils_DEFINED
#define SkUtils_DEFINED

#include "SkTypes.h"

#ifdef FMS_ARCH_ANDROID_ARM
    #include "utils/memory.h"
    
    #define SK_MEMSET16_REDIRECT(dst, value, count)    android_memset16(dst, value, (count) << 1)
    #define SK_MEMSET32_REDIRECT(dst, value, count)    android_memset32(dst, value, (count) << 2)
#endif

///////////////////////////////////////////////////////////////////////////

#ifdef SK_MEMSET16_REDIRECT
    #define sk_memset16(dst, value, count)  SK_MEMSET16_REDIRECT(dst, value, count)
#else
    /** Similar to memset(), but this function assigns a 16bit value into the buffer.
        @param buffer	The memory to have value copied into it
        @param value	The 16bit value to be copied into buffer
        @param count	The number of times value should be copied into the buffer.
    */
    void sk_memset16(uint16_t dst[], U16CPU value, int count);
#endif

#ifdef SK_MEMSET32_REDIRECT
    #define sk_memset32(dst, value, count)  SK_MEMSET32_REDIRECT(dst, value, count)
#else
    /** Similar to memset(), but this function assigns a 32bit value into the buffer.
        @param buffer	The memory to have value copied into it
        @param value	The 32bit value to be copied into buffer
        @param count	The number of times value should be copied into the buffer.
    */
    void sk_memset32(uint32_t dst[], uint32_t value, int count);
#endif


///////////////////////////////////////////////////////////////////////////

#define kMaxBytesInUTF8Sequence		4

#ifdef SK_DEBUG
	int SkUTF8_LeadByteToCount(unsigned c);
#else
	#define SkUTF8_LeadByteToCount(c)	((((0xE5 << 24) >> ((unsigned)c >> 4 << 1)) & 3) + 1)
#endif

inline int SkUTF8_CountUTF8Bytes(const char utf8[])
{
	SkASSERT(utf8);
	return SkUTF8_LeadByteToCount(*(const uint8_t*)utf8);
}

int			SkUTF8_CountUnichars(const char utf8[]);
int			SkUTF8_CountUnichars(const char utf8[], size_t byteLength);
SkUnichar	SkUTF8_ToUnichar(const char utf8[]);
SkUnichar	SkUTF8_NextUnichar(const char**);

/**	Return the number of bytes need to convert a unichar
	into a utf8 sequence. Will be 1..kMaxBytesInUTF8Sequence,
	or 0 if uni is illegal.
*/
size_t		SkUTF8_FromUnichar(SkUnichar uni, char utf8[] = nil);

/////////////////////////////////////////////////////////////////////////////////

#define SkUTF16_IsHighSurrogate(c)  (((c) & 0xFC00) == 0xD800)
#define SkUTF16_IsLowSurrogate(c)   (((c) & 0xFC00) == 0xDC00)

int         SkUTF16_CountUnichars(const uint16_t utf16[]);
int         SkUTF16_CountUnichars(const uint16_t utf16[], int numberOf16BitValues);
SkUnichar   SkUTF16_NextUnichar(const U16**);
size_t      SkUTF16_FromUnichar(SkUnichar uni, uint16_t utf16[] = nil);

size_t      SkUTF16_ToUTF8(const uint16_t utf16[], int numberOf16BitValues, char utf8[] = nil);

class SkUtils {
public:
#ifdef SK_DEBUG
	static void UnitTest();
#endif
};

#endif

