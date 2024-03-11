/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkUnicode_libgrapheme_DEFINED
#define SkUnicode_libgrapheme_DEFINED

#include "modules/skunicode/include/SkUnicode.h"

#include <memory>

namespace SkUnicodes::Libgrapheme {
SKUNICODE_API sk_sp<SkUnicode> Make();
}

#endif
