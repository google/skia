/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBBoxHierarchy_DEFINED
#define SkBBoxHierarchy_DEFINED

#include "include/core/SkBBHFactory.h"
#include "include/core/SkRect.h"
#include "include/private/SkTDArray.h"

/**
 * Interface for a spatial data structure that stores axis-aligned bounding
 * boxes and allows efficient retrieval of intersections with query rectangles.
 */
class SkBBoxHierarchy_Base : public SkBBoxHierarchy {
public:
    SkBBoxHierarchy_Base() {}

    /**
     * Insert N bounding boxes into the hierarchy.
     */
    virtual void insert(const SkRect[], int N) = 0;

    /**
     * Populate results with the indices of bounding boxes interesecting that query.
     */
    virtual void search(const SkRect& query, SkTDArray<int>* results) const = 0;

    virtual size_t bytesUsed() const = 0;

    // Get the root bound.
    virtual SkRect getRootBound() const = 0;
};

#endif
