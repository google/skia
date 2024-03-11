/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkUnicode_icu4x_DEFINED
#define SkUnicode_icu4x_DEFINED

#include "modules/skunicode/include/SkUnicode.h"

#include <memory>

namespace SkUnicodes::ICU4X {
SKUNICODE_API sk_sp<SkUnicode> Make();
}

#endif
