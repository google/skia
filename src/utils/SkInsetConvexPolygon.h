/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkConvexPolygonInset_DEFINED
#define SkConvexPolygonInset_DEFINED

#include "SkTArray.h"
#include "SkPoint.h"

bool SkInsetConvexPolygon(const SkTArray<SkPoint>& inputPolygon, SkScalar insetDistance,
                          SkTArray<SkPoint>* insetPolygon);

#endif
