/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStringDebug_DEFINED
#define SkStringDebug_DEFINED

#include <algorithm>
#include <string>

/** The debug implementation of SkString wraps a std::string. This allows debuggers to visualize
 *  the contents of the string. This class does not attempt to replicate the exact performance, size
 *  or safety characteristics of a SkString since it is intended for debug use. (It should never be
 *  *less* safe than a SkString, but may err on the side of being more safe.)
 *
 *  Notable differences in the debug implementation:
 *  -
 */
class SK_API SkString {
public:
    SkString() = default;
    SkString(const char text[], size_t len)
            : fString(text ? std::string(text, len) : std::string(len, '\0')) {}
    SkString(const SkString& that) { fString = that.fString; }
    SkString(SkString&& that) { fString = std::move(that.fString); }
    explicit SkString(size_t len) : fString(len, '\0') {}
    explicit SkString(const char text[]) : fString(text ? text : "") {}
    explicit SkString(const std::string& that) { fString = that; }
    ~SkString() = default;

    bool isEmpty() const { return fString.empty(); }
    size_t size() const { return fString.size(); }
    const char* c_str() const { return fString.c_str(); }
    char operator[](size_t n) const { return fString[n]; }

    bool equals(const SkString& that) const { return fString == that.fString; }
    bool equals(const char text[]) const { return 0 == strcmp(fString.c_str(), text); }
    bool equals(const char text[], size_t len) const {
        return len == fString.size() && 0 == memcmp(text, fString.c_str(), len);
    }

    bool startsWith(const char prefixStr[]) const {
        return SkStrStartsWith(fString.data(), prefixStr);
    }
    bool startsWith(const char prefixChar) const {
        return SkStrStartsWith(fString.data(), prefixChar);
    }
    bool endsWith(const char suffixStr[]) const {
        return SkStrEndsWith(fString.data(), suffixStr);
    }
    bool endsWith(const char suffixChar) const {
        return SkStrEndsWith(fString.data(), suffixChar);
    }
    bool contains(const char substring[]) const {
        return SkStrContains(fString.data(), substring);
    }
    bool contains(const char subchar) const {
        return SkStrContains(fString.data(), subchar);
    }
    int find(const char substring[]) const {
        return SkStrFind(fString.data(), substring);
    }
    int findLastOf(const char subchar) const {
        return SkStrFindLastOf(fString.data(), subchar);
    }

    friend bool operator==(const SkString& a, const SkString& b) {
        return a.fString == b.fString;
    }
    friend bool operator!=(const SkString& a, const SkString& b) {
        return a.fString != b.fString;
    }

    // these methods edit the string

    SkString& operator=(const SkString& that) { fString = that.fString; return *this; }
    SkString& operator=(SkString&& that) { fString = std::move(that.fString); return *this; }
    SkString& operator=(const char text[]) { fString = text; return *this; }

    char* writable_str() { return fString.data(); }
    char& operator[](size_t n) { return fString.data()[n]; }

    void reset() { fString.clear(); }

    /** String contents are preserved on resize. (For destructive resize, `set(nullptr, length)`.)
     * `resize` automatically reserves an extra byte at the end of the buffer for a null terminator.
     */
    void resize(size_t len) { fString.resize(len); }
    void set(const SkString& src) { *this = src; }
    void set(const char text[]) { *this = SkString(text); }
    void set(const char text[], size_t len) { *this = SkString(text, len); }

    void insert(size_t offset, const SkString& that) {
        fString.insert(this->fixOffset(offset), that.fString);
    }
    void insert(size_t offset, const char text[]) {
        fString.insert(this->fixOffset(offset), text);
    }
    void insert(size_t offset, const char text[], size_t len) {
        fString.insert(this->fixOffset(offset), text, len);
    }
    void insertUnichar(size_t offset, SkUnichar c) {
        fString.insert(this->fixOffset(offset), 1, c);
    }
    void insertS32(size_t offset, int32_t value) {
        char    buffer[SkStrAppendS32_MaxSize];
        char*   stop = SkStrAppendS32(buffer, value);
        fString.insert(this->fixOffset(offset), buffer, stop - buffer);
    }
    void insertS64(size_t offset, int64_t value, int minDigits = 0) {
        char    buffer[SkStrAppendS64_MaxSize];
        char*   stop = SkStrAppendS64(buffer, value, minDigits);
        fString.insert(this->fixOffset(offset), buffer, stop - buffer);
    }
    void insertU32(size_t offset, uint32_t value) {
        char    buffer[SkStrAppendU32_MaxSize];
        char*   stop = SkStrAppendU32(buffer, value);
        fString.insert(this->fixOffset(offset), buffer, stop - buffer);
    }
    void insertU64(size_t offset, uint64_t value, int minDigits = 0) {
        char    buffer[SkStrAppendU64_MaxSize];
        char*   stop = SkStrAppendU64(buffer, value, minDigits);
        fString.insert(this->fixOffset(offset), buffer, stop - buffer);
    }
    void insertHex(size_t offset, uint32_t value, int minDigits = 0) {
        char buffer[9];
        int length = std::snprintf(buffer, SK_ARRAY_COUNT(buffer), "%0*X", minDigits, value);
        fString.insert(this->fixOffset(offset), buffer, length);
    }
    void insertScalar(size_t offset, SkScalar value) {
        char    buffer[SkStrAppendScalar_MaxSize];
        char*   stop = SkStrAppendScalar(buffer, value);
        fString.insert(this->fixOffset(offset), buffer, stop - buffer);
    }

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
    void prependScalar(SkScalar value) { this->insertScalar(0, value); }

    void printf(const char format[], ...) SK_PRINTF_LIKE(2, 3);
    void printVAList(const char format[], va_list args);
    void appendf(const char format[], ...) SK_PRINTF_LIKE(2, 3);
    void appendVAList(const char format[], va_list args);
    void prependf(const char format[], ...) SK_PRINTF_LIKE(2, 3);
    void prependVAList(const char format[], va_list args);

    void remove(size_t offset, size_t length) { fString.erase(offset, length); }

    SkString& operator+=(const SkString& s) { this->append(s); return *this; }
    SkString& operator+=(const char text[]) { this->append(text); return *this; }
    SkString& operator+=(const char c) { this->append(&c, 1); return *this; }

    /**
     *  Swap contents between this and other. This function is guaranteed
     *  to never fail or throw.
     */
    void swap(SkString& that) { that.fString.swap(fString); }

protected:
    size_t fixOffset(size_t offset) { return std::min<>(offset, fString.size()); }
    std::string fString;
};

#endif  // SkStringDebug_DEFINED
