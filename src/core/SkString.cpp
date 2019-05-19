/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkString.h"
#include "include/private/SkTo.h"
#include "src/core/SkSafeMath.h"
#include "src/core/SkUtils.h"
#include "src/utils/SkUTF.h"

#include <cstdio>
#include <new>
#include <utility>
#include <vector>

// number of bytes (on the stack) to receive the printf result
static const size_t kBufferSize = 1024;

static const char* apply_format_string(const char* format, va_list args, char* stackBuffer,
                                       size_t stackBufferSize, int* length, SkString* heapBuffer) {
    va_list argsCopy;
    va_copy(argsCopy, args);
    *length = std::vsnprintf(stackBuffer, stackBufferSize, format, args);
    if (*length < 0) {
        SkDebugf("SkString: vsnprintf reported error.");
        va_end(argsCopy);
        *length = 0;
        return stackBuffer;
    }
    if (*length < SkToInt(stackBufferSize)) {
        va_end(argsCopy);
        return stackBuffer;
    }
    heapBuffer->resize(*length);
    SkDEBUGCODE(int check =)
            std::vsnprintf(heapBuffer->writable_str(), *length + 1, format, argsCopy);
    SkASSERT(check == *length);
    va_end(argsCopy);
    return heapBuffer->c_str();
}

#define ARGS_TO_BUFFER(format, buffer, size, written, result)                          \
    SkString overflow;                                                                 \
    do {                                                                               \
        va_list args;                                                                  \
        va_start(args, format);                                                        \
        result = apply_format_string(format, args, buffer, size, &written, &overflow); \
        va_end(args);                                                                  \
    } while (0)

#define V_SKSTRING_PRINTF(output, format)                                                       \
    do {                                                                                        \
        char buffer[kBufferSize];                                                               \
        va_list args;                                                                           \
        va_start(args, format);                                                                 \
        int length;                                                                             \
        auto result = apply_format_string(format, args, buffer, kBufferSize, &length, &output); \
        SkASSERT(result == output.c_str() || result == buffer);                                 \
        if (result == buffer) {                                                                 \
            output.set(buffer, length);                                                         \
        }                                                                                       \
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

char* SkStrAppendU32(char string[], uint32_t dec) {
    SkDEBUGCODE(char* start = string;)

    char    buffer[SkStrAppendU32_MaxSize];
    char*   p = buffer + sizeof(buffer);

    do {
        *--p = SkToU8('0' + dec % 10);
        dec /= 10;
    } while (dec != 0);

    SkASSERT(p >= buffer);
    char* stop = buffer + sizeof(buffer);
    while (p < stop) {
        *string++ = *p++;
    }
    SkASSERT(string - start <= SkStrAppendU32_MaxSize);
    return string;
}

char* SkStrAppendS32(char string[], int32_t dec) {
    uint32_t udec = dec;
    if (dec < 0) {
        *string++ = '-';
        udec = ~udec + 1;  // udec = -udec, but silences some warnings that are trying to be helpful
    }
    return SkStrAppendU32(string, udec);
}

char* SkStrAppendU64(char string[], uint64_t dec, int minDigits) {
    SkDEBUGCODE(char* start = string;)

    char    buffer[SkStrAppendU64_MaxSize];
    char*   p = buffer + sizeof(buffer);

    do {
        *--p = SkToU8('0' + (int32_t) (dec % 10));
        dec /= 10;
        minDigits--;
    } while (dec != 0);

    while (minDigits > 0) {
        *--p = '0';
        minDigits--;
    }

    SkASSERT(p >= buffer);
    size_t cp_len = buffer + sizeof(buffer) - p;
    memcpy(string, p, cp_len);
    string += cp_len;

    SkASSERT(string - start <= SkStrAppendU64_MaxSize);
    return string;
}

char* SkStrAppendS64(char string[], int64_t dec, int minDigits) {
    uint64_t udec = dec;
    if (dec < 0) {
        *string++ = '-';
        udec = ~udec + 1;  // udec = -udec, but silences some warnings that are trying to be helpful
    }
    return SkStrAppendU64(string, udec, minDigits);
}

char* SkStrAppendFloat(char string[], float value) {
    // since floats have at most 8 significant digits, we limit our %g to that.
    static const char gFormat[] = "%.8g";
    // make it 1 larger for the terminating 0
    char buffer[SkStrAppendScalar_MaxSize + 1];
    int len = snprintf(buffer, sizeof(buffer), gFormat, value);
    memcpy(string, buffer, len);
    SkASSERT(len <= SkStrAppendScalar_MaxSize);
    return string + len;
}

///////////////////////////////////////////////////////////////////////////////

const SkString::Rec SkString::gEmptyRec(0, 0);

#define SizeOfRec()     (gEmptyRec.data() - (const char*)&gEmptyRec)

static uint32_t trim_size_t_to_u32(size_t value) {
    if (sizeof(size_t) > sizeof(uint32_t)) {
        if (value > UINT32_MAX) {
            value = UINT32_MAX;
        }
    }
    return (uint32_t)value;
}

static size_t check_add32(size_t base, size_t extra) {
    SkASSERT(base <= UINT32_MAX);
    if (sizeof(size_t) > sizeof(uint32_t)) {
        if (base + extra > UINT32_MAX) {
            extra = UINT32_MAX - base;
        }
    }
    return extra;
}

sk_sp<SkString::Rec> SkString::Rec::Make(const char text[], size_t len) {
    if (0 == len) {
        return sk_sp<SkString::Rec>(const_cast<Rec*>(&gEmptyRec));
    }

    SkSafeMath safe;
    // We store a 32bit version of the length
    uint32_t stringLen = safe.castTo<uint32_t>(len);
    // Add SizeOfRec() for our overhead and 1 for null-termination
    size_t allocationSize = safe.add(len, SizeOfRec() + sizeof(char));
    // Align up to a multiple of 4
    allocationSize = safe.alignUp(allocationSize, 4);

    SkASSERT_RELEASE(safe.ok());

    void* storage = ::operator new (allocationSize);
    sk_sp<Rec> rec(new (storage) Rec(stringLen, 1));
    if (text) {
        memcpy(rec->data(), text, len);
    }
    rec->data()[len] = 0;
    return rec;
}

void SkString::Rec::ref() const {
    if (this == &SkString::gEmptyRec) {
        return;
    }
    SkAssertResult(this->fRefCnt.fetch_add(+1, std::memory_order_relaxed));
}

void SkString::Rec::unref() const {
    if (this == &SkString::gEmptyRec) {
        return;
    }
    int32_t oldRefCnt = this->fRefCnt.fetch_add(-1, std::memory_order_acq_rel);
    SkASSERT(oldRefCnt);
    if (1 == oldRefCnt) {
        delete this;
    }
}

bool SkString::Rec::unique() const {
    return fRefCnt.load(std::memory_order_acquire) == 1;
}

#ifdef SK_DEBUG
const SkString& SkString::validate() const {
    // make sure know one has written over our global
    SkASSERT(0 == gEmptyRec.fLength);
    SkASSERT(0 == gEmptyRec.fRefCnt.load(std::memory_order_relaxed));
    SkASSERT(0 == gEmptyRec.data()[0]);

    if (fRec.get() != &gEmptyRec) {
        SkASSERT(fRec->fLength > 0);
        SkASSERT(fRec->fRefCnt.load(std::memory_order_relaxed) > 0);
        SkASSERT(0 == fRec->data()[fRec->fLength]);
    }
    return *this;
}
#endif

///////////////////////////////////////////////////////////////////////////////

SkString::SkString() : fRec(const_cast<Rec*>(&gEmptyRec)) {
}

SkString::SkString(size_t len) {
    fRec = Rec::Make(nullptr, len);
}

SkString::SkString(const char text[]) {
    size_t  len = text ? strlen(text) : 0;

    fRec = Rec::Make(text, len);
}

SkString::SkString(const char text[], size_t len) {
    fRec = Rec::Make(text, len);
}

SkString::SkString(const SkString& src) : fRec(src.validate().fRec) {}

SkString::SkString(SkString&& src) : fRec(std::move(src.validate().fRec)) {
    src.fRec.reset(const_cast<Rec*>(&gEmptyRec));
}

SkString::~SkString() {
    this->validate();
}

bool SkString::equals(const SkString& src) const {
    return fRec == src.fRec || this->equals(src.c_str(), src.size());
}

bool SkString::equals(const char text[]) const {
    return this->equals(text, text ? strlen(text) : 0);
}

bool SkString::equals(const char text[], size_t len) const {
    SkASSERT(len == 0 || text != nullptr);

    return fRec->fLength == len && !memcmp(fRec->data(), text, len);
}

SkString& SkString::operator=(const SkString& src) {
    this->validate();
    fRec = src.fRec;  // sk_sp<Rec>::operator=(const sk_sp<Ref>&) checks for self-assignment.
    return *this;
}

SkString& SkString::operator=(SkString&& src) {
    this->validate();

    if (fRec != src.fRec) {
        this->swap(src);
    }
    return *this;
}

SkString& SkString::operator=(const char text[]) {
    this->validate();
    return *this = SkString(text);
}

void SkString::reset() {
    this->validate();
    fRec.reset(const_cast<Rec*>(&gEmptyRec));
}

char* SkString::writable_str() {
    this->validate();

    if (fRec->fLength) {
        if (!fRec->unique()) {
            fRec = Rec::Make(fRec->data(), fRec->fLength);
        }
    }
    return fRec->data();
}

void SkString::set(const char text[]) {
    this->set(text, text ? strlen(text) : 0);
}

void SkString::set(const char text[], size_t len) {
    len = trim_size_t_to_u32(len);
    bool unique = fRec->unique();
    if (0 == len) {
        this->reset();
    } else if (unique && len <= fRec->fLength) {
        // should we resize if len <<<< fLength, to save RAM? (e.g. len < (fLength>>1))?
        // just use less of the buffer without allocating a smaller one
        char* p = this->writable_str();
        if (text) {
            memcpy(p, text, len);
        }
        p[len] = 0;
        fRec->fLength = SkToU32(len);
    } else if (unique && (fRec->fLength >> 2) == (len >> 2)) {
        // we have spare room in the current allocation, so don't alloc a larger one
        char* p = this->writable_str();
        if (text) {
            memcpy(p, text, len);
        }
        p[len] = 0;
        fRec->fLength = SkToU32(len);
    } else {
        SkString tmp(text, len);
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

        // Check if length + len exceeds 32bits, we trim len
        len = check_add32(length, len);
        if (0 == len) {
            return;
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
        if (fRec->unique() && (length >> 2) == ((length + len) >> 2)) {
            char* dst = this->writable_str();

            if (offset < length) {
                memmove(dst + offset + len, dst + offset, length - offset);
            }
            memcpy(dst + offset, text, len);

            dst[length + len] = 0;
            fRec->fLength = SkToU32(length + len);
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
    char    buffer[SkUTF::kMaxBytesInUTF8Sequence];
    size_t  len = SkUTF::ToUTF8(uni, buffer);

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

void SkString::insertU32(size_t offset, uint32_t dec) {
    char    buffer[SkStrAppendU32_MaxSize];
    char*   stop = SkStrAppendU32(buffer, dec);
    this->insert(offset, buffer, stop - buffer);
}

void SkString::insertU64(size_t offset, uint64_t dec, int minDigits) {
    char    buffer[SkStrAppendU64_MaxSize];
    char*   stop = SkStrAppendU64(buffer, dec, minDigits);
    this->insert(offset, buffer, stop - buffer);
}

void SkString::insertHex(size_t offset, uint32_t hex, int minDigits) {
    minDigits = SkTPin(minDigits, 0, 8);

    char    buffer[8];
    char*   p = buffer + sizeof(buffer);

    do {
        *--p = SkHexadecimalDigits::gUpper[hex & 0xF];
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
    V_SKSTRING_PRINTF((*this), format);
}

void SkString::appendf(const char format[], ...) {
    char buffer[kBufferSize];
    int length;
    const char* result;
    ARGS_TO_BUFFER(format, buffer, kBufferSize, length, result);

    this->append(result, length);
}

void SkString::appendVAList(const char format[], va_list args) {
    char buffer[kBufferSize];
    int length = vsnprintf(buffer, kBufferSize, format, args);
    SkASSERT(length >= 0 && length < SkToInt(kBufferSize));

    this->append(buffer, length);
}

void SkString::prependf(const char format[], ...) {
    char buffer[kBufferSize];
    int length;
    const char* result;
    ARGS_TO_BUFFER(format, buffer, kBufferSize, length, result);

    this->prepend(result, length);
}

void SkString::prependVAList(const char format[], va_list args) {
    char buffer[kBufferSize];
    int length = vsnprintf(buffer, kBufferSize, format, args);
    SkASSERT(length >= 0 && length < SkToInt(kBufferSize));

    this->prepend(buffer, length);
}


///////////////////////////////////////////////////////////////////////////////

void SkString::remove(size_t offset, size_t length) {
    size_t size = this->size();

    if (offset < size) {
        if (length > size - offset) {
            length = size - offset;
        }
        SkASSERT(length <= size);
        SkASSERT(offset <= size - length);
        if (length > 0) {
            SkString    tmp(size - length);
            char*       dst = tmp.writable_str();
            const char* src = this->c_str();

            if (offset) {
                memcpy(dst, src, offset);
            }
            size_t tail = size - (offset + length);
            if (tail) {
                memcpy(dst + offset, src + (offset + length), tail);
            }
            SkASSERT(dst[tmp.size()] == 0);
            this->swap(tmp);
        }
    }
}

void SkString::swap(SkString& other) {
    this->validate();
    other.validate();

    using std::swap;
    swap(fRec, other.fRec);
}

///////////////////////////////////////////////////////////////////////////////

SkString SkStringPrintf(const char* format, ...) {
    SkString formattedOutput;
    V_SKSTRING_PRINTF(formattedOutput, format);
    return formattedOutput;
}

void SkStrSplit(const char* str, const char* delimiters, SkStrSplitMode splitMode,
                SkTArray<SkString>* out) {
    if (splitMode == kCoalesce_SkStrSplitMode) {
        // Skip any delimiters.
        str += strspn(str, delimiters);
    }
    if (!*str) {
        return;
    }

    while (true) {
        // Find a token.
        const size_t len = strcspn(str, delimiters);
        if (splitMode == kStrict_SkStrSplitMode || len > 0) {
            out->push_back().set(str, len);
            str += len;
        }

        if (!*str) {
            return;
        }
        if (splitMode == kCoalesce_SkStrSplitMode) {
            // Skip any delimiters.
            str += strspn(str, delimiters);
        } else {
            // Skip one delimiter.
            str += 1;
        }
    }
}
