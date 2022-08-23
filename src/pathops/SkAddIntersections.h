/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkAddIntersections_DEFINED
#define SkAddIntersections_DEFINED

class SkOpCoincidence;
class SkOpContour;

bool AddIntersectTs(SkOpContour* test, SkOpContour* next, SkOpCoincidence* coincidence);

#endif
