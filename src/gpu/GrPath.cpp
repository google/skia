/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPath.h"

void GrPath::ComputeKey(const SkPath& path, const GrStrokeInfo& stroke, GrUniqueKey* key) {
    static const GrUniqueKey::Domain kPathDomain = GrUniqueKey::GenerateDomain();
    int strokeDataCnt = stroke.computeUniqueKeyFragmentData32Cnt();
    GrUniqueKey::Builder builder(key, kPathDomain, 2 + strokeDataCnt);
    builder[0] = path.getGenerationID();
    builder[1] = path.getFillType();
    if (strokeDataCnt > 0) {
        stroke.asUniqueKeyFragment(&builder[2]);
    }
}

