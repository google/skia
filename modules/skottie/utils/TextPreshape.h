/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieTextPreshape_DEFINED
#define SkottieTextPreshape_DEFINED

#include <cstddef>

#include "include/core/SkRefCnt.h"

class SkData;
class SkFontMgr;
class SkWStream;

namespace skresources { class ResourceProvider; }
namespace SkShapers { class Factory; }

namespace skottie_utils {

bool Preshape(const char* json, size_t size, SkWStream*,
              const sk_sp<SkFontMgr>&,
              const sk_sp<SkShapers::Factory>&,
              const sk_sp<skresources::ResourceProvider>&);

bool Preshape(const sk_sp<SkData>&, SkWStream*,
              const sk_sp<SkFontMgr>&,
              const sk_sp<SkShapers::Factory>&,
              const sk_sp<skresources::ResourceProvider>&);

} //  namespace skottie_utils

#endif //  SkottieTextPreshape_DEFINED
