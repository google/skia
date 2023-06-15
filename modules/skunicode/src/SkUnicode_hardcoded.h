/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkUnicode_hardcoded_DEFINED
#define SkUnicode_hardcoded_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "src/base/SkUTF.h"

using namespace skia_private;

class SkUnicodeHardCodedCharProperties : public SkUnicode {
public:
    ~SkUnicodeHardCodedCharProperties() override;

    bool isControl(SkUnichar utf8) override;
    bool isWhitespace(SkUnichar utf8) override;
    bool isSpace(SkUnichar utf8) override;
    bool isTabulation(SkUnichar utf8) override;
    bool isHardBreak(SkUnichar utf8) override;
    bool isEmoji(SkUnichar utf8) override;
    bool isIdeographic(SkUnichar utf8) override;
};

#endif // SkUnicode_hardcoded_DEFINED
