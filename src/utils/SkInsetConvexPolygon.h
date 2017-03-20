/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkConvexPolygonInset_DEFINED
#define SkConvexPolygonInset_DEFINED

#include "SkTDArray.h"
#include "SkPoint.h"

bool SkInsetConvexPolygon(const SkTDArray<SkPoint>& inputPolygon, SkScalar insetDistance,
                          SkTDArray<SkPoint>* insetPolygon);

#endif
