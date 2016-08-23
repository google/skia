/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMultiPictureDocumentPriv_DEFINED
#define SkMultiPictureDocumentPriv_DEFINED

#include "SkTArray.h"
#include "SkSize.h"

namespace SkMultiPictureDocumentProtocol {
static constexpr char kMagic[] = "Skia Multi-Picture Doc\n\n";

static constexpr char kEndPage[] = "SkMultiPictureEndPage";

const uint32_t kVersion = 2;

inline SkSize Join(const SkTArray<SkSize>& sizes) {
    SkSize joined = SkSize::Make(0, 0);
    for (SkSize s : sizes) {
        joined = SkSize::Make(SkTMax(joined.width(), s.width()),
                              SkTMax(joined.height(), s.height()));
    }
    return joined;
}

}

#endif  // SkMultiPictureDocumentPriv_DEFINED
