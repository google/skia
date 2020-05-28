// Copyright 2019 Google LLC.

#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTo.h"
#include "modules/skparagraph/src/ParagraphUtil.h"

#include <unicode/umachine.h>
#include <unicode/ustring.h>
#include <unicode/utypes.h>
#include <string>
#include <third_party/externals/icu/source/i18n/dt_impl.h>

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

    icu::UnicodeString unicode((UChar*)utf16text.data(), SkToS32(utf16text.size()));
    std::string str;
    unicode.toUTF8String(str);
    SkString check(str.data(), str.size());
    if (check.size() != dst.size()) {
        SkDebugf("'%s' '%s'\n", dst.c_str(), check.c_str());
    }
    return dst;
}

}
}
