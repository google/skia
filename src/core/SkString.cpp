
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkString.h"
#include "SkFixed.h"
#include "SkThread.h"
#include "SkUtils.h"
#include <stdarg.h>
#include <stdio.h>

// number of bytes (on the stack) to receive the printf result
static const size_t kBufferSize = 512;

#ifdef SK_BUILD_FOR_WIN
    #define VSNPRINTF(buffer, size, format, args) \
        _vsnprintf_s(buffer, size, _TRUNCATE, format, args)
    #define SNPRINTF    _snprintf
#else
    #define VSNPRINTF   vsnprintf
    #define SNPRINTF    snprintf
#endif

#define ARGS_TO_BUFFER(format, buffer, size)        \
    do {                                            \
        va_list args;                               \
        va_start(args, format);                     \
        VSNPRINTF(buffer, size, format, args);      \
        va_end(args);                               \
    } while (0)

///////////////////////////////////////////////////////////////////////////////

bool SkStrEndsWith(const char string[], const char suffixStr[]) {
    SkASSERT(string);
    SkASSERT(suffixStr);
    size_t  strLen = strlen(string);
    size_t  suffixLen = strlen(suffixStr);
    return  strLen >= suffixLen &&
            !strncmp(string + strLen - suffixLen, suffixStr, suffixLen);
}

bool SkStrEndsWith(const char string[], const char suffixChar) {
    SkASSERT(string);
    size_t  strLen = strlen(string);
    if (0 == strLen) {
        return false;
    } else {
        return (suffixChar == string[strLen-1]);
    }
}

int SkStrStartsWithOneOf(const char string[], const char prefixes[]) {
    int index = 0;
    do {
        const char* limit = strchr(prefixes, '\0');
        if (!strncmp(string, prefixes, limit - prefixes)) {
            return index;
        }
        prefixes = limit + 1;
        index++;
    } while (prefixes[0]);
    return -1;
}

char* SkStrAppendS32(char string[], int32_t dec) {
    SkDEBUGCODE(char* start = string;)

    char    buffer[SkStrAppendS32_MaxSize];
    char*   p = buffer + sizeof(buffer);
    bool    neg = false;

    if (dec < 0) {
        neg = true;
        dec = -dec;
    }

    do {
        *--p = SkToU8('0' + dec % 10);
        dec /= 10;
    } while (dec != 0);

    if (neg) {
        *--p = '-';
    }

    SkASSERT(p >= buffer);
    char* stop = buffer + sizeof(buffer);
    while (p < stop) {
        *string++ = *p++;
    }
    SkASSERT(string - start <= SkStrAppendS32_MaxSize);
    return string;
}

char* SkStrAppendS64(char string[], int64_t dec, int minDigits) {
    SkDEBUGCODE(char* start = string;)

    char    buffer[SkStrAppendS64_MaxSize];
    char*   p = buffer + sizeof(buffer);
    bool    neg = false;

    if (dec < 0) {
        neg = true;
        dec = -dec;
    }

    do {
        *--p = SkToU8('0' + (int32_t) (dec % 10));
        dec /= 10;
        minDigits--;
    } while (dec != 0);

    while (minDigits > 0) {
        *--p = '0';
        minDigits--;
    }

    if (neg) {
        *--p = '-';
    }
    SkASSERT(p >= buffer);
    size_t cp_len = buffer + sizeof(buffer) - p;
    memcpy(string, p, cp_len);
    string += cp_len;

    SkASSERT(string - start <= SkStrAppendS64_MaxSize);
    return string;
}

char* SkStrAppendFloat(char string[], float value) {
    // since floats have at most 8 significant digits, we limit our %g to that.
    static const char gFormat[] = "%.8g";
    // make it 1 larger for the terminating 0
    char buffer[SkStrAppendScalar_MaxSize + 1];
    int len = SNPRINTF(buffer, sizeof(buffer), gFormat, value);
    memcpy(string, buffer, len);
    SkASSERT(len <= SkStrAppendScalar_MaxSize);
    return string + len;
}

char* SkStrAppendFixed(char string[], SkFixed x) {
    SkDEBUGCODE(char* start = string;)
    if (x < 0) {
        *string++ = '-';
        x = -x;
    }

    unsigned frac = x & 0xFFFF;
    x >>= 16;
    if (frac == 0xFFFF) {
        // need to do this to "round up", since 65535/65536 is closer to 1 than to .9999
        x += 1;
        frac = 0;
    }
    string = SkStrAppendS32(string, x);

    // now handle the fractional part (if any)
    if (frac) {
        static const uint16_t   gTens[] = { 1000, 100, 10, 1 };
        const uint16_t*         tens = gTens;

        x = SkFixedRound(frac * 10000);
        SkASSERT(x <= 10000);
        if (x == 10000) {
            x -= 1;
        }
        *string++ = '.';
        do {
            unsigned powerOfTen = *tens++;
            *string++ = SkToU8('0' + x / powerOfTen);
            x %= powerOfTen;
        } while (x != 0);
    }

    SkASSERT(string - start <= SkStrAppendScalar_MaxSize);
    return string;
}

///////////////////////////////////////////////////////////////////////////////

// the 3 values are [length] [refcnt] [terminating zero data]
const SkString::Rec SkString::gEmptyRec = { 0, 0, 0 };

#define SizeOfRec()     (gEmptyRec.data() - (const char*)&gEmptyRec)

SkString::Rec* SkString::AllocRec(const char text[], size_t len) {
    Rec* rec;

    if (0 == len) {
        rec = const_cast<Rec*>(&gEmptyRec);
    } else {
        // add 1 for terminating 0, then align4 so we can have some slop when growing the string
        rec = (Rec*)sk_malloc_throw(SizeOfRec() + SkAlign4(len + 1));
        rec->fLength = len;
        rec->fRefCnt = 1;
        if (text) {
            memcpy(rec->data(), text, len);
        }
        rec->data()[len] = 0;
    }
    return rec;
}

SkString::Rec* SkString::RefRec(Rec* src) {
    if (src != &gEmptyRec) {
        sk_atomic_inc(&src->fRefCnt);
    }
    return src;
}

#ifdef SK_DEBUG
void SkString::validate() const {
    // make sure know one has written over our global
    SkASSERT(0 == gEmptyRec.fLength);
    SkASSERT(0 == gEmptyRec.fRefCnt);
    SkASSERT(0 == gEmptyRec.data()[0]);

    if (fRec != &gEmptyRec) {
        SkASSERT(fRec->fLength > 0);
        SkASSERT(fRec->fRefCnt > 0);
        SkASSERT(0 == fRec->data()[fRec->fLength]);
    }
    SkASSERT(fStr == c_str());
}
#endif

///////////////////////////////////////////////////////////////////////////////

SkString::SkString() : fRec(const_cast<Rec*>(&gEmptyRec)) {
#ifdef SK_DEBUG
    fStr = fRec->data();
#endif
}

SkString::SkString(size_t len) {
    SkASSERT(SkToU16(len) == len);  // can't handle larger than 64K

    fRec = AllocRec(NULL, (U16CPU)len);
#ifdef SK_DEBUG
    fStr = fRec->data();
#endif
}

SkString::SkString(const char text[]) {
    size_t  len = text ? strlen(text) : 0;

    fRec = AllocRec(text, (U16CPU)len);
#ifdef SK_DEBUG
    fStr = fRec->data();
#endif
}

SkString::SkString(const char text[], size_t len) {
    fRec = AllocRec(text, (U16CPU)len);
#ifdef SK_DEBUG
    fStr = fRec->data();
#endif
}

SkString::SkString(const SkString& src) {
    src.validate();

    fRec = RefRec(src.fRec);
#ifdef SK_DEBUG
    fStr = fRec->data();
#endif
}

SkString::~SkString() {
    this->validate();

    if (fRec->fLength) {
        SkASSERT(fRec->fRefCnt > 0);
        if (sk_atomic_dec(&fRec->fRefCnt) == 1) {
            sk_free(fRec);
        }
    }
}

bool SkString::equals(const SkString& src) const {
    return fRec == src.fRec || this->equals(src.c_str(), src.size());
}

bool SkString::equals(const char text[]) const {
    return this->equals(text, text ? strlen(text) : 0);
}

bool SkString::equals(const char text[], size_t len) const {
    SkASSERT(len == 0 || text != NULL);

    return fRec->fLength == len && !memcmp(fRec->data(), text, len);
}

SkString& SkString::operator=(const SkString& src) {
    this->validate();

    if (fRec != src.fRec) {
        SkString    tmp(src);
        this->swap(tmp);
    }
    return *this;
}

SkString& SkString::operator=(const char text[]) {
    this->validate();

    SkString tmp(text);
    this->swap(tmp);

    return *this;
}

void SkString::reset() {
    this->validate();

    if (fRec->fLength) {
        SkASSERT(fRec->fRefCnt > 0);
        if (sk_atomic_dec(&fRec->fRefCnt) == 1) {
            sk_free(fRec);
        }
    }

    fRec = const_cast<Rec*>(&gEmptyRec);
#ifdef SK_DEBUG
    fStr = fRec->data();
#endif
}

char* SkString::writable_str() {
    this->validate();

    if (fRec->fLength) {
        if (fRec->fRefCnt > 1) {
            Rec* rec = AllocRec(fRec->data(), fRec->fLength);
            if (sk_atomic_dec(&fRec->fRefCnt) == 1) {
                // In this case after our check of fRecCnt > 1, we suddenly
                // did become the only owner, so now we have two copies of the
                // data (fRec and rec), so we need to delete one of them.
                sk_free(fRec);
            }
            fRec = rec;
        #ifdef SK_DEBUG
            fStr = fRec->data();
        #endif
        }
    }
    return fRec->data();
}

void SkString::set(const char text[]) {
    this->set(text, text ? strlen(text) : 0);
}

void SkString::set(const char text[], size_t len) {
    if (0 == len) {
        this->reset();
    } else if (1 == fRec->fRefCnt && len <= fRec->fLength) {
        // should we resize if len <<<< fLength, to save RAM? (e.g. len < (fLength>>1))?
        // just use less of the buffer without allocating a smaller one
        char* p = this->writable_str();
        if (text) {
            memcpy(p, text, len);
        }
        p[len] = 0;
        fRec->fLength = len;
    } else if (1 == fRec->fRefCnt && (fRec->fLength >> 2) == (len >> 2)) {
        // we have spare room in the current allocation, so don't alloc a larger one
        char* p = this->writable_str();
        if (text) {
            memcpy(p, text, len);
        }
        p[len] = 0;
        fRec->fLength = len;
    } else {
        SkString tmp(text, len);
        this->swap(tmp);
    }
}

void SkString::setUTF16(const uint16_t src[]) {
    int count = 0;

    while (src[count]) {
        count += 1;
    }
    setUTF16(src, count);
}

void SkString::setUTF16(const uint16_t src[], size_t count) {
    if (0 == count) {
        this->reset();
    } else if (count <= fRec->fLength) {
        // should we resize if len <<<< fLength, to save RAM? (e.g. len < (fLength>>1))
        if (count < fRec->fLength) {
            this->resize(count);
        }
        char* p = this->writable_str();
        for (size_t i = 0; i < count; i++) {
            p[i] = SkToU8(src[i]);
        }
        p[count] = 0;
    } else {
        SkString tmp(count); // puts a null terminator at the end of the string
        char*    p = tmp.writable_str();

        for (size_t i = 0; i < count; i++) {
            p[i] = SkToU8(src[i]);
        }
        this->swap(tmp);
    }
}

void SkString::insert(size_t offset, const char text[]) {
    this->insert(offset, text, text ? strlen(text) : 0);
}

void SkString::insert(size_t offset, const char text[], size_t len) {
    if (len) {
        size_t length = fRec->fLength;
        if (offset > length) {
            offset = length;
        }

        /*  If we're the only owner, and we have room in our allocation for the insert,
            do it in place, rather than allocating a new buffer.

            To know we have room, compare the allocated sizes
            beforeAlloc = SkAlign4(length + 1)
            afterAlloc  = SkAligh4(length + 1 + len)
            but SkAlign4(x) is (x + 3) >> 2 << 2
            which is equivalent for testing to (length + 1 + 3) >> 2 == (length + 1 + 3 + len) >> 2
            and we can then eliminate the +1+3 since that doesn't affec the answer
        */
        if (1 == fRec->fRefCnt && (length >> 2) == ((length + len) >> 2)) {
            char* dst = this->writable_str();

            if (offset < length) {
                memmove(dst + offset + len, dst + offset, length - offset);
            }
            memcpy(dst + offset, text, len);

            dst[length + len] = 0;
            fRec->fLength = length + len;
        } else {
            /*  Seems we should use realloc here, since that is safe if it fails
                (we have the original data), and might be faster than alloc/copy/free.
            */
            SkString    tmp(fRec->fLength + len);
            char*       dst = tmp.writable_str();

            if (offset > 0) {
                memcpy(dst, fRec->data(), offset);
            }
            memcpy(dst + offset, text, len);
            if (offset < fRec->fLength) {
                memcpy(dst + offset + len, fRec->data() + offset,
                       fRec->fLength - offset);
            }

            this->swap(tmp);
        }
    }
}

void SkString::insertUnichar(size_t offset, SkUnichar uni) {
    char    buffer[kMaxBytesInUTF8Sequence];
    size_t  len = SkUTF8_FromUnichar(uni, buffer);

    if (len) {
        this->insert(offset, buffer, len);
    }
}

void SkString::insertS32(size_t offset, int32_t dec) {
    char    buffer[SkStrAppendS32_MaxSize];
    char*   stop = SkStrAppendS32(buffer, dec);
    this->insert(offset, buffer, stop - buffer);
}

void SkString::insertS64(size_t offset, int64_t dec, int minDigits) {
    char    buffer[SkStrAppendS64_MaxSize];
    char*   stop = SkStrAppendS64(buffer, dec, minDigits);
    this->insert(offset, buffer, stop - buffer);
}

void SkString::insertHex(size_t offset, uint32_t hex, int minDigits) {
    minDigits = SkPin32(minDigits, 0, 8);

    static const char gHex[] = "0123456789ABCDEF";

    char    buffer[8];
    char*   p = buffer + sizeof(buffer);

    do {
        *--p = gHex[hex & 0xF];
        hex >>= 4;
        minDigits -= 1;
    } while (hex != 0);

    while (--minDigits >= 0) {
        *--p = '0';
    }

    SkASSERT(p >= buffer);
    this->insert(offset, p, buffer + sizeof(buffer) - p);
}

void SkString::insertScalar(size_t offset, SkScalar value) {
    char    buffer[SkStrAppendScalar_MaxSize];
    char*   stop = SkStrAppendScalar(buffer, value);
    this->insert(offset, buffer, stop - buffer);
}

void SkString::printf(const char format[], ...) {
    char    buffer[kBufferSize];
    ARGS_TO_BUFFER(format, buffer, kBufferSize);

    this->set(buffer, strlen(buffer));
}

void SkString::appendf(const char format[], ...) {
    char    buffer[kBufferSize];
    ARGS_TO_BUFFER(format, buffer, kBufferSize);

    this->append(buffer, strlen(buffer));
}

void SkString::appendf(const char format[], va_list args) {
    char    buffer[kBufferSize];
    VSNPRINTF(buffer, kBufferSize, format, args);

    this->append(buffer, strlen(buffer));
}

void SkString::prependf(const char format[], ...) {
    char    buffer[kBufferSize];
    ARGS_TO_BUFFER(format, buffer, kBufferSize);

    this->prepend(buffer, strlen(buffer));
}

///////////////////////////////////////////////////////////////////////////////

void SkString::remove(size_t offset, size_t length) {
    size_t size = this->size();

    if (offset < size) {
        if (offset + length > size) {
            length = size - offset;
        }
        if (length > 0) {
            SkASSERT(size > length);
            SkString    tmp(size - length);
            char*       dst = tmp.writable_str();
            const char* src = this->c_str();

            if (offset) {
                SkASSERT(offset <= tmp.size());
                memcpy(dst, src, offset);
            }
            size_t tail = size - offset - length;
            SkASSERT((int32_t)tail >= 0);
            if (tail) {
        //      SkASSERT(offset + length <= tmp.size());
                memcpy(dst + offset, src + offset + length, tail);
            }
            SkASSERT(dst[tmp.size()] == 0);
            this->swap(tmp);
        }
    }
}

void SkString::swap(SkString& other) {
    this->validate();
    other.validate();

    SkTSwap<Rec*>(fRec, other.fRec);
#ifdef SK_DEBUG
    SkTSwap<const char*>(fStr, other.fStr);
#endif
}

///////////////////////////////////////////////////////////////////////////////

SkAutoUCS2::SkAutoUCS2(const char utf8[]) {
    size_t len = strlen(utf8);
    fUCS2 = (uint16_t*)sk_malloc_throw((len + 1) * sizeof(uint16_t));

    uint16_t* dst = fUCS2;
    for (;;) {
        SkUnichar uni = SkUTF8_NextUnichar(&utf8);
        *dst++ = SkToU16(uni);
        if (uni == 0) {
            break;
        }
    }
    fCount = (int)(dst - fUCS2);
}

SkAutoUCS2::~SkAutoUCS2() {
    sk_free(fUCS2);
}

///////////////////////////////////////////////////////////////////////////////

SkString SkStringPrintf(const char* format, ...) {
    SkString formattedOutput;
    char buffer[kBufferSize];
    ARGS_TO_BUFFER(format, buffer, kBufferSize);
    formattedOutput.set(buffer);
    return formattedOutput;
}

#undef VSNPRINTF
#undef SNPRINTF
