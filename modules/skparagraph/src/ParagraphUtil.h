// Copyright 2020 Google LLC.
#ifndef ParagraphUtil_DEFINED
#define ParagraphUtil_DEFINED

#include "include/core/SkString.h"
#include <string>

namespace skia {
namespace textlayout {
SkString SkStringFromU16String(const std::u16string& utf16text);
}
}

#endif
