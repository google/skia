/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifdef SK_SUPPORT_LEGACY_DERIVED_PICTURE_CLASSES

#include "SkQuadTreePicture.h"

#include "SkQuadTree.h"

SkBBoxHierarchy* SkQuadTreePicture::createBBoxHierarchy() const {
    return SkNEW_ARGS(SkQuadTree, (fBounds));
}

#endif
