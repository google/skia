
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkString_DEFINED
#define SkString_DEFINED

#include "SkScalar.h"
#include "SkTArray.h"

#include <stdarg.h>

/*  Some helper functions for C strings
*/

static bool SkStrStartsWith(const char string[], const char prefixStr[]) {
    SkASSERT(string);
    SkASSERT(prefixStr);
    return !strncmp(string, prefixStr, strlen(prefixStr));
}
static bool SkStrStartsWith(const char string[], const char prefixChar) {
    SkASSERT(string);
    return (prefixChar == *string);
}

bool SkStrEndsWith(const char string[], const char suffixStr[]);
bool SkStrEndsWith(const char string[], const char suffixChar);

int SkStrStartsWithOneOf(const char string[], const char prefixes[]);

static int SkStrFind(const char string[], const char substring[]) {
    const char *first = strstr(string, substring);
    if (NULL == first) return -1;
    return SkToS32(first - &string[0]);
}

static bool SkStrContains(const char string[], const char substring[]) {
    SkASSERT(string);
    SkASSERT(substring);
    return (-1 != SkStrFind(string, substring));
}
static bool SkStrContains(const char string[], const char subchar) {
    SkASSERT(string);
    char tmp[2];
    tmp[0] = subchar;
    tmp[1] = '\0';
    return (-1 != SkStrFind(string, tmp));
}

static inline char *SkStrDup(const char string[]) {
    char *ret = (char *) sk_malloc_throw(strlen(string)+1);
    memcpy(ret,string,strlen(string)+1);
    return ret;
}



#define SkStrAppendU32_MaxSize  10
char*   SkStrAppendU32(char buffer[], uint32_t);
#define SkStrAppendU64_MaxSize  20
char*   SkStrAppendU64(char buffer[], uint64_t, int minDigits);

#define SkStrAppendS32_MaxSize  (SkStrAppendU32_MaxSize + 1)
char*   SkStrAppendS32(char buffer[], int32_t);
#define SkStrAppendS64_MaxSize  (SkStrAppendU64_MaxSize + 1)
char*   SkStrAppendS64(char buffer[], int64_t, int minDigits);

/**
 *  Floats have at most 8 significant digits, so we limit our %g to that.
 *  However, the total string could be 15 characters: -1.2345678e-005
 *
 *  In theory we should only expect up to 2 digits for the exponent, but on
 *  some platforms we have seen 3 (as in the example above).
 */
#define SkStrAppendScalar_MaxSize  15

/**
 *  Write the scaler in decimal format into buffer, and return a pointer to
 *  the next char after the last one written. Note: a terminating 0 is not
 *  written into buffer, which must be at least SkStrAppendScalar_MaxSize.
 *  Thus if the caller wants to add a 0 at the end, buffer must be at least
 *  SkStrAppendScalar_MaxSize + 1 bytes large.
 */
#ifdef SK_SCALAR_IS_FLOAT
    #define SkStrAppendScalar SkStrAppendFloat
#else
    #define SkStrAppendScalar SkStrAppendFixed
#endif

char* SkStrAppendFloat(char buffer[], float);
char* SkStrAppendFixed(char buffer[], SkFixed);

/** \class SkString

    Light weight class for managing strings. Uses reference
    counting to make string assignments and copies very fast
    with no extra RAM cost. Assumes UTF8 encoding.
*/
class SK_API SkString {
public:
                SkString();
    explicit    SkString(size_t len);
    explicit    SkString(const char text[]);
                SkString(const char text[], size_t len);
                SkString(const SkString&);
                ~SkString();

    bool        isEmpty() const { return 0 == fRec->fLength; }
    size_t      size() const { return (size_t) fRec->fLength; }
    const char* c_str() const { return fRec->data(); }
    char operator[](size_t n) const { return this->c_str()[n]; }

    bool equals(const SkString&) const;
    bool equals(const char text[]) const;
    bool equals(const char text[], size_t len) const;

    bool startsWith(const char prefixStr[]) const {
        return SkStrStartsWith(fRec->data(), prefixStr);
    }
    bool startsWith(const char prefixChar) const {
        return SkStrStartsWith(fRec->data(), prefixChar);
    }
    bool endsWith(const char suffixStr[]) const {
        return SkStrEndsWith(fRec->data(), suffixStr);
    }
    bool endsWith(const char suffixChar) const {
        return SkStrEndsWith(fRec->data(), suffixChar);
    }
    bool contains(const char substring[]) const {
        return SkStrContains(fRec->data(), substring);
    }
    bool contains(const char subchar) const {
        return SkStrContains(fRec->data(), subchar);
    }
    int find(const char substring[]) const {
        return SkStrFind(fRec->data(), substring);
    }

    friend bool operator==(const SkString& a, const SkString& b) {
        return a.equals(b);
    }
    friend bool operator!=(const SkString& a, const SkString& b) {
        return !a.equals(b);
    }

    // these methods edit the string

    SkString& operator=(const SkString&);
    SkString& operator=(const char text[]);

    char* writable_str();
    char& operator[](size_t n) { return this->writable_str()[n]; }

    void reset();
    void resize(size_t len) { this->set(NULL, len); }
    void set(const SkString& src) { *this = src; }
    void set(const char text[]);
    void set(const char text[], size_t len);
    void setUTF16(const uint16_t[]);
    void setUTF16(const uint16_t[], size_t len);

    void insert(size_t offset, const SkString& src) { this->insert(offset, src.c_str(), src.size()); }
    void insert(size_t offset, const char text[]);
    void insert(size_t offset, const char text[], size_t len);
    void insertUnichar(size_t offset, SkUnichar);
    void insertS32(size_t offset, int32_t value);
    void insertS64(size_t offset, int64_t value, int minDigits = 0);
    void insertU32(size_t offset, uint32_t value);
    void insertU64(size_t offset, uint64_t value, int minDigits = 0);
    void insertHex(size_t offset, uint32_t value, int minDigits = 0);
    void insertScalar(size_t offset, SkScalar);

    void append(const SkString& str) { this->insert((size_t)-1, str); }
    void append(const char text[]) { this->insert((size_t)-1, text); }
    void append(const char text[], size_t len) { this->insert((size_t)-1, text, len); }
    void appendUnichar(SkUnichar uni) { this->insertUnichar((size_t)-1, uni); }
    void appendS32(int32_t value) { this->insertS32((size_t)-1, value); }
    void appendS64(int64_t value, int minDigits = 0) { this->insertS64((size_t)-1, value, minDigits); }
    void appendU32(uint32_t value) { this->insertU32((size_t)-1, value); }
    void appendU64(uint64_t value, int minDigits = 0) { this->insertU64((size_t)-1, value, minDigits); }
    void appendHex(uint32_t value, int minDigits = 0) { this->insertHex((size_t)-1, value, minDigits); }
    void appendScalar(SkScalar value) { this->insertScalar((size_t)-1, value); }

    void prepend(const SkString& str) { this->insert(0, str); }
    void prepend(const char text[]) { this->insert(0, text); }
    void prepend(const char text[], size_t len) { this->insert(0, text, len); }
    void prependUnichar(SkUnichar uni) { this->insertUnichar(0, uni); }
    void prependS32(int32_t value) { this->insertS32(0, value); }
    void prependS64(int32_t value, int minDigits = 0) { this->insertS64(0, value, minDigits); }
    void prependHex(uint32_t value, int minDigits = 0) { this->insertHex(0, value, minDigits); }
    void prependScalar(SkScalar value) { this->insertScalar((size_t)-1, value); }

    void printf(const char format[], ...) SK_PRINTF_LIKE(2, 3);
    void appendf(const char format[], ...) SK_PRINTF_LIKE(2, 3);
    void appendVAList(const char format[], va_list);
    void prependf(const char format[], ...) SK_PRINTF_LIKE(2, 3);

    void remove(size_t offset, size_t length);

    SkString& operator+=(const SkString& s) { this->append(s); return *this; }
    SkString& operator+=(const char text[]) { this->append(text); return *this; }
    SkString& operator+=(const char c) { this->append(&c, 1); return *this; }

    /**
     *  Swap contents between this and other. This function is guaranteed
     *  to never fail or throw.
     */
    void swap(SkString& other);

private:
    struct Rec {
    public:
        uint32_t    fLength; // logically size_t, but we want it to stay 32bits
        int32_t     fRefCnt;
        char        fBeginningOfData;

        char* data() { return &fBeginningOfData; }
        const char* data() const { return &fBeginningOfData; }
    };
    Rec* fRec;

#ifdef SK_DEBUG
    const char* fStr;
    void validate() const;
#else
    void validate() const {}
#endif

    static const Rec gEmptyRec;
    static Rec* AllocRec(const char text[], size_t len);
    static Rec* RefRec(Rec*);
};

/// Creates a new string and writes into it using a printf()-style format.
SkString SkStringPrintf(const char* format, ...);

// Specialized to take advantage of SkString's fast swap path. The unspecialized function is
// declared in SkTypes.h and called by SkTSort.
template <> inline void SkTSwap(SkString& a, SkString& b) {
    a.swap(b);
}

// Split str on any characters in delimiters into out.  (Think, strtok with a sane API.)
void SkStrSplit(const char* str, const char* delimiters, SkTArray<SkString>* out);

#endif
