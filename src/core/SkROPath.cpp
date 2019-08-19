/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkROPath.h"

SkROPath::SkROPath() : fBounds({0, 0, 0, 0})
{
}

SkROPath::SkROPath(SkTDArray<SkPoint>&& pts, SkTDArray<char>&& verbs,
                   SkTDArray<SkScalar>&& conicWeights)
    : fPts(std::move(pts))
    , fVerbs(std::move(verbs))
    , fConicWeights(std::move(conicWeights))
{
    fPts.shrinkToFit();
    fVerbs.shrinkToFit();
    fConicWeights.shrinkToFit();
}

