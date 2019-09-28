// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkUTF_DEFINED
#define SkUTF_DEFINED

#include <cstddef>
#include <cstdint>

typedef int32_t SkUnichar;

namespace SkUTF {

/** Given a sequence of UTF-8 bytes, return the number of unicode codepoints.
    If the sequence is invalid UTF-8, return -1.
*/
int CountUTF8(const char* utf8, size_t byteLength);

/** Given a sequence of aligned UTF-16 characters in machine-endian form,
    return the number of unicode codepoints.  If the sequence is invalid
    UTF-16, return -1.
*/
int CountUTF16(const uint16_t* utf16, size_t byteLength);

/** Given a sequence of aligned UTF-32 characters in machine-endian form,
    return the number of unicode codepoints.  If the sequence is invalid
    UTF-32, return -1.
*/
int CountUTF32(const int32_t* utf32, size_t byteLength);

/** Given a sequence of UTF-8 bytes, return the first unicode codepoint.
    The pointer will be incremented to point at the next codepoint's start.  If
    invalid UTF-8 is encountered, set *ptr to end and return -1.
*/
SkUnichar NextUTF8(const char** ptr, const char* end);

/** Given a sequence of aligned UTF-16 characters in machine-endian form,
    return the first unicode codepoint.  The pointer will be incremented to
    point at the next codepoint's start.  If invalid UTF-16 is encountered,
    set *ptr to end and return -1.
*/
SkUnichar NextUTF16(const uint16_t** ptr, const uint16_t* end);

/** Given a sequence of aligned UTF-32 characters in machine-endian form,
    return the first unicode codepoint.  The pointer will be incremented to
    point at the next codepoint's start.  If invalid UTF-32 is encountered,
    set *ptr to end and return -1.
*/
SkUnichar NextUTF32(const int32_t** ptr, const int32_t* end);

constexpr unsigned kMaxBytesInUTF8Sequence = 4;

/** Convert the unicode codepoint into UTF-8.  If `utf8` is non-null, place the
    result in that array.  Return the number of bytes in the result.  If `utf8`
    is null, simply return the number of bytes that would be used.  For invalid
    unicode codepoints, return 0.
*/
size_t ToUTF8(SkUnichar uni, char utf8[kMaxBytesInUTF8Sequence] = nullptr);

/** Convert the unicode codepoint into UTF-16.  If `utf16` is non-null, place
    the result in that array.  Return the number of UTF-16 code units in the
    result (1 or 2).  If `utf16` is null, simply return the number of code
    units that would be used.  For invalid unicode codepoints, return 0.
*/
size_t ToUTF16(SkUnichar uni, uint16_t utf16[2] = nullptr);

}  // namespace SkUTF

#endif  // SkUTF_DEFINED
