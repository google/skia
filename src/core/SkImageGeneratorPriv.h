/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageGeneratorPriv_DEFINED
#define SkImageGeneratorPriv_DEFINED

#include "SkImageGenerator.h"
#include "SkDiscardableMemory.h"

/**
 *  Takes ownership of SkImageGenerator.  If this method fails for
 *  whatever reason, it will return false and immediatetely delete
 *  the generator.  If it succeeds, it will modify destination
 *  bitmap.
 *
 *  If generator is nullptr, will safely return false.
 *
 *  If this fails or when the SkDiscardablePixelRef that is
 *  installed into destination is destroyed, it will call
 *  `delete` on the generator.  Therefore, generator should be
 *  allocated with `new`.
 *
 *  @param destination Upon success, this bitmap will be
 *  configured and have a pixelref installed.
 *
 *  @param factory If not nullptr, this object will be used as a
 *  source of discardable memory when decoding.  If nullptr, then
 *  SkDiscardableMemory::Create() will be called.
 *
 *  @return true iff successful.
 */
bool SkDEPRECATED_InstallDiscardablePixelRef(SkImageGenerator*, const SkIRect* subset,
                                             SkBitmap* destination,
                                             SkDiscardableMemory::Factory* factory);

#endif
