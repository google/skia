/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkUnicode_bidi_DEFINED
#define SkUnicode_bidi_DEFINED

#include "modules/skunicode/include/SkUnicode.h"

#include <memory>

namespace SkUnicodes::Bidi {
SKUNICODE_API sk_sp<SkUnicode> Make(); // It's used for bidi only (and possibly some hardcode)
}

#endif // SkUnicode_bidi_DEFINED
