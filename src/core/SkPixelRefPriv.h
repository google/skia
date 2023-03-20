// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef SkPixelRefPriv_DEFINED
#define SkPixelRefPriv_DEFINED

#include "include/core/SkRefCnt.h"

#include <cstddef>

class SkPixelRef;

/**
 *  Return a new SkMallocPixelRef with the provided pixel storage and
 *  rowBytes. On destruction, ReleaseProc will be called.
 *
 *  If ReleaseProc is NULL, the pixels will never be released. This
 *  can be useful if the pixels were stack allocated. However, such an
 *  SkMallocPixelRef must not live beyond its pixels (e.g. by copying
 *  an SkBitmap pointing to it, or drawing to an SkPicture).
 *
 *  Returns NULL on failure.
 */
sk_sp<SkPixelRef> SkMakePixelRefWithProc(int w, int h, size_t rowBytes, void* addr,
                                         void (*releaseProc)(void* addr, void* ctx), void* ctx);

#endif  // SkPixelRefPriv_DEFINED
