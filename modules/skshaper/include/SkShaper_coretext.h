/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShaper_coretext_DEFINED
#define SkShaper_coretext_DEFINED

#include "modules/skshaper/include/SkShaper.h"

#include <memory>

namespace SkShapers::CT {
SKSHAPER_API std::unique_ptr<SkShaper> CoreText();
}

#endif
