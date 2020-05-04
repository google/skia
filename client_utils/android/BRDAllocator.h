/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef BRDAllocator_DEFINED
#define BRDAllocator_DEFINED

#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"

namespace android {
namespace skia {

/**
 *  Abstract subclass of SkBitmap's allocator.
 *  Allows the allocator to indicate if the memory it allocates
 *  is zero initialized.
 */
class BRDAllocator : public SkBitmap::Allocator {
public:

    /**
     *  Indicates if the memory allocated by this allocator is
     *  zero initialized.
     */
    virtual SkCodec::ZeroInitialized zeroInit() const = 0;
};

} // namespace skia
} // namespace android

#endif // BRDAllocator_DEFINED
