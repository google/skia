/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * One day, slugs might be supported on a CPU backend, but for now are GPU-only.
 * CPU-only builds need this file to allow SkPicture deserialization to link w/o errors.
 */

#include "include/private/chromium/Slug.h"

namespace sktext::gpu {
sk_sp<Slug> Slug::MakeFromBuffer(SkReadBuffer&) {
    return nullptr;
}
}
