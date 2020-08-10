/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
#include "include/core/SkRefCnt.h"

class SkCodec;
class SkData;

namespace SkCGCodec {
    std::unique_ptr<SkCodec> MakeFromEncoded(sk_sp<SkData>);
}
#endif
