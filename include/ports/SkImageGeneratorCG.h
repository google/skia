/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../core/SkTypes.h"
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

#include "../core/SkData.h"
#include "../core/SkImageGenerator.h"

#include <memory>

namespace SkImageGeneratorCG {
SK_API std::unique_ptr<SkImageGenerator> MakeFromEncodedCG(sk_sp<SkData>);
}

#endif //defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
