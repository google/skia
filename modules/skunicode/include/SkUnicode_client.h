/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkUnicode_client_DEFINED
#define SkUnicode_client_DEFINED

#include "include/core/SkSpan.h"
#include "modules/skunicode/include/SkUnicode.h"

#include <memory>
#include <vector>

namespace SkUnicodes::Client {
SKUNICODE_API sk_sp<SkUnicode> Make(
                SkSpan<char> text,
                std::vector<SkUnicode::Position> words,
                std::vector<SkUnicode::Position> graphemeBreaks,
                std::vector<SkUnicode::LineBreakBefore> lineBreaks);
}

#endif // SkUnicode_client_DEFINED
