// Copyright 2019 Google LLC.

#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTo.h"
#include "modules/skparagraph/src/ParagraphUtil.h"
#include "src/utils/SkUTF.h"

#include <unicode/umachine.h>
#include <unicode/uchar.h>
#include <unicode/ustring.h>
#include <unicode/utypes.h>
#include <string>

namespace skia {
namespace textlayout {

SkString SkStringFromU16String(const std::u16string& utf16text) {
    SkString dst;
    UErrorCode status = U_ZERO_ERROR;
    int32_t dstSize;
    // Getting the length like this seems to always set U_BUFFER_OVERFLOW_ERROR
    u_strToUTF8(nullptr, 0, &dstSize, (UChar*)utf16text.data(), SkToS32(utf16text.size()), &status);
    dst.resize(dstSize);
    status = U_ZERO_ERROR;
    u_strToUTF8(dst.writable_str(), dst.size(), nullptr,
                (UChar*)utf16text.data(), SkToS32(utf16text.size()), &status);
    if (U_FAILURE(status)) {
        SkDEBUGF("Invalid UTF-16 input: %s", u_errorName(status));
        return dst;
    }
    return dst;
}

SkUnichar nextUtf8Unit(const char** ptr, const char* end) {
    SkUnichar val = SkUTF::NextUTF8(ptr, end);
    return val < 0 ? 0xFFFD : val;
}

bool isControl(SkUnichar utf8) {
    return u_iscntrl(utf8);
}

bool isWhitespace(SkUnichar utf8) {
    return u_isWhitespace(utf8);
}

}
}
