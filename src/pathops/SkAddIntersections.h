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
#include "SkTArray.h"

bool AddIntersectTs(SkOpContour* test, SkOpContour* next);
void AddSelfIntersectTs(SkOpContour* test);
void CoincidenceCheck(SkTArray<SkOpContour*, true>* contourList, int total);

#endif
