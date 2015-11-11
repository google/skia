/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCodec.h"

/**
 *  Abstract subclass of SkBitmap's allocator.
 *  Allows the allocator to indicate if the memory it allocates
 *  is zero initialized.
 */
class SkBRDAllocator : public SkBitmap::Allocator {
public:

    /**
     *  Indicates if the memory allocated by this allocator is
     *  zero initialized.
     */
    virtual SkCodec::ZeroInitialized zeroInit() const = 0;
};
