/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkUnicode_icu_DEFINED
#define SkUnicode_icu_DEFINED

#include "include/core/SkRefCnt.h"
#include "modules/skunicode/include/SkUnicode.h"

namespace SkUnicodes::ICU {
SKUNICODE_API sk_sp<SkUnicode> Make();
}

#endif //SkUnicode_icu_DEFINED
