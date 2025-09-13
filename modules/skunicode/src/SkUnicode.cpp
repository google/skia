/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/private/base/SkDebug.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "src/base/SkBitmaskEnum.h"

using namespace skia_private;

SkString SkUnicode::convertUtf16ToUtf8(const char16_t* utf16Char, int utf16Units) {
    const uint16_t* utf16 = reinterpret_cast<const uint16_t*>(utf16Char);
    int utf8Units = SkUTF::UTF16ToUTF8(nullptr, 0, utf16, utf16Units);
    if (utf8Units < 0) {
        SkDEBUGF("Convert error: Invalid utf16 input");
        return SkString();
    }
    SkString s(utf8Units);
    SkDEBUGCODE(int dstLen =) SkUTF::UTF16ToUTF8(s.data(), utf8Units, utf16, utf16Units);
    SkASSERT(dstLen == utf8Units);
    return s;
}

SkString SkUnicode::convertUtf16ToUtf8(const std::u16string& utf16) {
    return convertUtf16ToUtf8(utf16.c_str(), utf16.size());
}

std::u16string SkUnicode::convertUtf8ToUtf16(const char* utf8, int utf8Units) {
    int utf16Units = SkUTF::UTF8ToUTF16(nullptr, 0, utf8, utf8Units);
    if (utf16Units < 0) {
        SkDEBUGF("Convert error: Invalid utf8 input");
        return std::u16string();
    }
    std::u16string utf16Char(utf16Units, '\0');
    uint16_t* utf16 = reinterpret_cast<uint16_t*>(utf16Char.data());
    SkDEBUGCODE(int dstLen =) SkUTF::UTF8ToUTF16(utf16, utf16Units, utf8, utf8Units);
    SkASSERT(dstLen == utf16Units);
    return utf16Char;
}

std::u16string SkUnicode::convertUtf8ToUtf16(const SkString& utf8) {
    return convertUtf8ToUtf16(utf8.c_str(), utf8.size());
}

bool SkUnicode::hasTabulationFlag(SkUnicode::CodeUnitFlags flags) {
    return (flags & SkUnicode::kTabulation) == SkUnicode::kTabulation;
}

bool SkUnicode::hasHardLineBreakFlag(SkUnicode::CodeUnitFlags flags) {
    return (flags & SkUnicode::kHardLineBreakBefore) == SkUnicode::kHardLineBreakBefore;
}

bool SkUnicode::hasSoftLineBreakFlag(SkUnicode::CodeUnitFlags flags) {
    return (flags & SkUnicode::kSoftLineBreakBefore) == SkUnicode::kSoftLineBreakBefore;
}

bool SkUnicode::hasGraphemeStartFlag(SkUnicode::CodeUnitFlags flags) {
    return (flags & SkUnicode::kGraphemeStart) == SkUnicode::kGraphemeStart;
}

bool SkUnicode::hasControlFlag(SkUnicode::CodeUnitFlags flags) {
    return (flags & SkUnicode::kControl) == SkUnicode::kControl;
}

bool SkUnicode::hasPartOfWhiteSpaceBreakFlag(SkUnicode::CodeUnitFlags flags) {
    return (flags & SkUnicode::kPartOfWhiteSpaceBreak) == SkUnicode::kPartOfWhiteSpaceBreak;
}
