/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRTreePicture.h"

#include "SkRTree.h"

SkBBoxHierarchy* SkRTreeFactory::operator()(int width, int height) const {
    // These values were empirically determined to produce reasonable
    // performance in most cases.
    static const int kRTreeMinChildren = 6;
    static const int kRTreeMaxChildren = 11;

    SkScalar aspectRatio = SkScalarDiv(SkIntToScalar(width),
                                       SkIntToScalar(height));
    bool sortDraws = false;  // Do not sort draw calls when bulk loading.

    return SkRTree::Create(kRTreeMinChildren, kRTreeMaxChildren,
                           aspectRatio, sortDraws);
}

#ifdef SK_SUPPORT_LEGACY_DERIVED_PICTURE_CLASSES

SkBBoxHierarchy* SkRTreePicture::createBBoxHierarchy() const {
    // These values were empirically determined to produce reasonable
    // performance in most cases.
    static const int kRTreeMinChildren = 6;
    static const int kRTreeMaxChildren = 11;

    SkScalar aspectRatio = SkScalarDiv(SkIntToScalar(fWidth),
                                       SkIntToScalar(fHeight));
    bool sortDraws = false;  // Do not sort draw calls when bulk loading.

    return SkRTree::Create(kRTreeMinChildren, kRTreeMaxChildren,
                           aspectRatio, sortDraws);
}

#endif
