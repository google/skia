
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkPackBits.h"

#define GATHER_STATSx

static inline void small_memcpy(void* SK_RESTRICT dst,
                                const void* SK_RESTRICT src, size_t n) {
    SkASSERT(n > 0 && n <= 15);
    uint8_t* d = (uint8_t*)dst;
    const uint8_t* s = (const uint8_t*)src;
    switch (n) {
        case 15: *d++ = *s++;
        case 14: *d++ = *s++;
        case 13: *d++ = *s++;
        case 12: *d++ = *s++;
        case 11: *d++ = *s++;
        case 10: *d++ = *s++;
        case  9: *d++ = *s++;
        case  8: *d++ = *s++;
        case  7: *d++ = *s++;
        case  6: *d++ = *s++;
        case  5: *d++ = *s++;
        case  4: *d++ = *s++;
        case  3: *d++ = *s++;
        case  2: *d++ = *s++;
        case  1: *d++ = *s++;
        case  0: break;
    }
}

static inline void small_memset(void* dst, uint8_t value, size_t n) {
    SkASSERT(n > 0 && n <= 15);
    uint8_t* d = (uint8_t*)dst;
    switch (n) {
        case 15: *d++ = value;
        case 14: *d++ = value;
        case 13: *d++ = value;
        case 12: *d++ = value;
        case 11: *d++ = value;
        case 10: *d++ = value;
        case  9: *d++ = value;
        case  8: *d++ = value;
        case  7: *d++ = value;
        case  6: *d++ = value;
        case  5: *d++ = value;
        case  4: *d++ = value;
        case  3: *d++ = value;
        case  2: *d++ = value;
        case  1: *d++ = value;
        case  0: break;
    }
}

// can we do better for small counts with our own inlined memcpy/memset?

#define PB_MEMSET(addr, value, count)       \
do {                                        \
if ((count) > 15) {                     \
memset(addr, value, count);         \
} else {                                \
small_memset(addr, value, count);   \
}                                       \
} while (0)

#define PB_MEMCPY(dst, src, count)      \
do {                                    \
    if ((count) > 15) {                 \
        memcpy(dst, src, count);        \
    } else {                            \
        small_memcpy(dst, src, count);  \
    }                                   \
} while (0)

///////////////////////////////////////////////////////////////////////////////

#ifdef GATHER_STATS
    static int gMemSetBuckets[129];
    static int gMemCpyBuckets[129];
    static int gCounter;

static void register_memset_count(int n) {
    SkASSERT((unsigned)n <= 128);
    gMemSetBuckets[n] += 1;
    gCounter += 1;

    if ((gCounter & 0xFF) == 0) {
        SkDebugf("----- packbits memset stats: ");
        for (size_t i = 0; i < SK_ARRAY_COUNT(gMemSetBuckets); i++) {
            if (gMemSetBuckets[i]) {
                SkDebugf(" %d:%d", i, gMemSetBuckets[i]);
            }
        }
    }
}
static void register_memcpy_count(int n) {
    SkASSERT((unsigned)n <= 128);
    gMemCpyBuckets[n] += 1;
    gCounter += 1;

    if ((gCounter & 0x1FF) == 0) {
        SkDebugf("----- packbits memcpy stats: ");
        for (size_t i = 0; i < SK_ARRAY_COUNT(gMemCpyBuckets); i++) {
            if (gMemCpyBuckets[i]) {
                SkDebugf(" %d:%d", i, gMemCpyBuckets[i]);
            }
        }
    }
}
#else
#define register_memset_count(n)
#define register_memcpy_count(n)
#endif


///////////////////////////////////////////////////////////////////////////////

size_t SkPackBits::ComputeMaxSize16(int count) {
    // worst case is the number of 16bit values (times 2) +
    // 1 byte per (up to) 128 entries.
    return ((count + 127) >> 7) + (count << 1);
}

size_t SkPackBits::ComputeMaxSize8(int count) {
    // worst case is the number of 8bit values + 1 byte per (up to) 128 entries.
    return ((count + 127) >> 7) + count;
}

static uint8_t* flush_same16(uint8_t dst[], uint16_t value, int count) {
    while (count > 0) {
        int n = count;
        if (n > 128) {
            n = 128;
        }
        *dst++ = (uint8_t)(n - 1);
        *dst++ = (uint8_t)(value >> 8);
        *dst++ = (uint8_t)value;
        count -= n;
    }
    return dst;
}

static uint8_t* flush_same8(uint8_t dst[], uint8_t value, int count) {
    while (count > 0) {
        int n = count;
        if (n > 128) {
            n = 128;
        }
        *dst++ = (uint8_t)(n - 1);
        *dst++ = (uint8_t)value;
        count -= n;
    }
    return dst;
}

static uint8_t* flush_diff16(uint8_t* SK_RESTRICT dst,
                             const uint16_t* SK_RESTRICT src, int count) {
    while (count > 0) {
        int n = count;
        if (n > 128) {
            n = 128;
        }
        *dst++ = (uint8_t)(n + 127);
        PB_MEMCPY(dst, src, n * sizeof(uint16_t));
        src += n;
        dst += n * sizeof(uint16_t);
        count -= n;
    }
    return dst;
}

static uint8_t* flush_diff8(uint8_t* SK_RESTRICT dst,
                            const uint8_t* SK_RESTRICT src, int count) {
    while (count > 0) {
        int n = count;
        if (n > 128) {
            n = 128;
        }
        *dst++ = (uint8_t)(n + 127);
        PB_MEMCPY(dst, src, n);
        src += n;
        dst += n;
        count -= n;
    }
    return dst;
}

size_t SkPackBits::Pack16(const uint16_t* SK_RESTRICT src, int count,
                          uint8_t* SK_RESTRICT dst) {
    uint8_t* origDst = dst;
    const uint16_t* stop = src + count;

    for (;;) {
        count = SkToInt(stop - src);
        SkASSERT(count >= 0);
        if (count == 0) {
            return dst - origDst;
        }
        if (1 == count) {
            *dst++ = 0;
            *dst++ = (uint8_t)(*src >> 8);
            *dst++ = (uint8_t)*src;
            return dst - origDst;
        }

        unsigned value = *src;
        const uint16_t* s = src + 1;

        if (*s == value) { // accumulate same values...
            do {
                s++;
                if (s == stop) {
                    break;
                }
            } while (*s == value);
            dst = flush_same16(dst, value, SkToInt(s - src));
        } else {    // accumulate diff values...
            do {
                if (++s == stop) {
                    goto FLUSH_DIFF;
                }
            } while (*s != s[-1]);
            s -= 1; // back up so we don't grab one of the "same" values that follow
        FLUSH_DIFF:
            dst = flush_diff16(dst, src, SkToInt(s - src));
        }
        src = s;
    }
}

size_t SkPackBits::Pack8(const uint8_t* SK_RESTRICT src, int count,
                         uint8_t* SK_RESTRICT dst) {
    uint8_t* origDst = dst;
    const uint8_t* stop = src + count;

    for (;;) {
        count = SkToInt(stop - src);
        SkASSERT(count >= 0);
        if (count == 0) {
            return dst - origDst;
        }
        if (1 == count) {
            *dst++ = 0;
            *dst++ = *src;
            return dst - origDst;
        }

        unsigned value = *src;
        const uint8_t* s = src + 1;

        if (*s == value) { // accumulate same values...
            do {
                s++;
                if (s == stop) {
                    break;
                }
            } while (*s == value);
            dst = flush_same8(dst, value, SkToInt(s - src));
        } else {    // accumulate diff values...
            do {
                if (++s == stop) {
                    goto FLUSH_DIFF;
                }
                // only stop if we hit 3 in a row,
                // otherwise we get bigger than compuatemax
            } while (*s != s[-1] || s[-1] != s[-2]);
            s -= 2; // back up so we don't grab the "same" values that follow
        FLUSH_DIFF:
            dst = flush_diff8(dst, src, SkToInt(s - src));
        }
        src = s;
    }
}

#include "SkUtils.h"

int SkPackBits::Unpack16(const uint8_t* SK_RESTRICT src, size_t srcSize,
                         uint16_t* SK_RESTRICT dst) {
    uint16_t* origDst = dst;
    const uint8_t* stop = src + srcSize;

    while (src < stop) {
        unsigned n = *src++;
        if (n <= 127) {   // repeat count (n + 1)
            n += 1;
            sk_memset16(dst, (src[0] << 8) | src[1], n);
            src += 2;
        } else {    // same count (n - 127)
            n -= 127;
            PB_MEMCPY(dst, src, n * sizeof(uint16_t));
            src += n * sizeof(uint16_t);
        }
        dst += n;
    }
    SkASSERT(src == stop);
    return SkToInt(dst - origDst);
}

int SkPackBits::Unpack8(const uint8_t* SK_RESTRICT src, size_t srcSize,
                        uint8_t* SK_RESTRICT dst) {
    uint8_t* origDst = dst;
    const uint8_t* stop = src + srcSize;

    while (src < stop) {
        unsigned n = *src++;
        if (n <= 127) {   // repeat count (n + 1)
            n += 1;
            PB_MEMSET(dst, *src++, n);
        } else {    // same count (n - 127)
            n -= 127;
            PB_MEMCPY(dst, src, n);
            src += n;
        }
        dst += n;
    }
    SkASSERT(src == stop);
    return SkToInt(dst - origDst);
}

enum UnpackState {
    CLEAN_STATE,
    REPEAT_BYTE_STATE,
    COPY_SRC_STATE
};

void SkPackBits::Unpack8(uint8_t* SK_RESTRICT dst, size_t dstSkip,
                         size_t dstWrite, const uint8_t* SK_RESTRICT src) {
    if (dstWrite == 0) {
        return;
    }

    UnpackState state = CLEAN_STATE;
    size_t      stateCount = 0;

    // state 1: do the skip-loop
    while (dstSkip > 0) {
        size_t n = *src++;
        if (n <= 127) {   // repeat count (n + 1)
            n += 1;
            if (n > dstSkip) {
                state = REPEAT_BYTE_STATE;
                stateCount = n - dstSkip;
                n = dstSkip;
                // we don't increment src here, since its needed in stage 2
            } else {
                src++;  // skip the src byte
            }
        } else {    // same count (n - 127)
            n -= 127;
            if (n > dstSkip) {
                state = COPY_SRC_STATE;
                stateCount = n - dstSkip;
                n = dstSkip;
            }
            src += n;
        }
        dstSkip -= n;
    }

    // stage 2: perform any catchup from the skip-stage
    if (stateCount > dstWrite) {
        stateCount = dstWrite;
    }
    switch (state) {
        case REPEAT_BYTE_STATE:
            SkASSERT(stateCount > 0);
            register_memset_count(stateCount);
            PB_MEMSET(dst, *src++, stateCount);
            break;
        case COPY_SRC_STATE:
            SkASSERT(stateCount > 0);
            register_memcpy_count(stateCount);
            PB_MEMCPY(dst, src, stateCount);
            src += stateCount;
            break;
        default:
            SkASSERT(stateCount == 0);
            break;
    }
    dst += stateCount;
    dstWrite -= stateCount;

    // copy at most dstWrite bytes into dst[]
    while (dstWrite > 0) {
        size_t n = *src++;
        if (n <= 127) {   // repeat count (n + 1)
            n += 1;
            if (n > dstWrite) {
                n = dstWrite;
            }
            register_memset_count(n);
            PB_MEMSET(dst, *src++, n);
        } else {    // same count (n - 127)
            n -= 127;
            if (n > dstWrite) {
                n = dstWrite;
            }
            register_memcpy_count(n);
            PB_MEMCPY(dst, src, n);
            src += n;
        }
        dst += n;
        dstWrite -= n;
    }
    SkASSERT(0 == dstWrite);
}
