/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGrPriv_DEFINED
#define SkGrPriv_DEFINED

#include "GrTypes.h"
#include "SkPoint.h"

class GrCaps;
class GrUniqueKey;

/**
 *  Our key includes the offset, width, and height so that bitmaps created by extractSubset()
 *  are unique.
 *
 *  The imageID is in the shared namespace (see SkNextID::ImageID()
 *      - SkBitmap/SkPixelRef
 *      - SkImage
 *      - SkImageGenerator
 *
 *  Note: width/height must fit in 16bits for this impl.
 */
void GrMakeKeyFromImageID(GrUniqueKey* key, uint32_t imageID,
                          U16CPU width, U16CPU height, SkIPoint origin,
                          const GrCaps&, SkImageUsageType);

#endif
