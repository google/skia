/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShaper_skunicode_DEFINED
#define SkShaper_skunicode_DEFINED

#include "modules/skshaper/include/SkShaper.h"

#include <cstddef>
#include <cstdint>
#include <memory>

class SkUnicode;

namespace SkShapers::unicode {
SKSHAPER_API std::unique_ptr<SkShaper::BiDiRunIterator> BidiRunIterator(SkUnicode* unicode,
                                                                        const char* utf8,
                                                                        size_t utf8Bytes,
                                                                        uint8_t bidiLevel);

}  // namespace SkShapers::unicode

#endif
