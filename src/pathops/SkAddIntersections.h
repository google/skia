/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkAddIntersections_DEFINED
#define SkAddIntersections_DEFINED

#include "SkIntersectionHelper.h"
#include "SkIntersections.h"

class SkOpCoincidence;

bool AddIntersectTs(SkOpContour* test, SkOpContour* next, SkOpCoincidence* coincidence,
        SkChunkAlloc* allocator);

#endif
