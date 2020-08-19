/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkYUVAIndex.h"
#include "include/core/SkYUVASizeInfo.h"
#include "include/core/SkYUVASpec.h"

#ifndef SkYUVASpecPriv_DEFINED
#define SkYUVASpecPriv_DEFINED

class SkPixmap;

namespace SkYUVASpecPriv {

bool InitLegacyInfo(SkYUVASpec spec,
                    const SkPixmap planes[SkYUVASpec::kMaxPlanes],
                    SkYUVASizeInfo* yuvaSizeInfo,
                    SkYUVAIndex yuvaIndices[SkYUVAIndex::kIndexCount]);

}  // namespace SkYUVASpecPriv

#endif
