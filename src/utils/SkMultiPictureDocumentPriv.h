/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMultiPictureDocumentPriv_DEFINED
#define SkMultiPictureDocumentPriv_DEFINED

#include "stdint.h"

namespace SkMultiPictureDocumentProtocol {
static constexpr char kMagic[] = "Skia Multi-Picture Doc\n\n";

struct Entry {
    uint64_t offset;
    float sizeX;
    float sizeY;
};
}

#endif  // SkMultiPictureDocumentPriv_DEFINED
