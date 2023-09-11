/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/encode/SkPngEncoder.h"
#include "include/private/base/SkAssert.h"

class GrDirectContext;
class SkImage;
class SkPixmap;
class SkWStream;

namespace SkPngEncoder {

bool Encode(SkWStream*, const SkPixmap&, const Options&) {
    SkDEBUGFAIL("Using encoder stub");
    return false;
}

sk_sp<SkData> Encode(GrDirectContext*, const SkImage*, const Options&) {
    SkDEBUGFAIL("Using encoder stub");
    return nullptr;
}

}  // namespace SkPngEncoder
