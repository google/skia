/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkYUVAIndex.h"
#include "include/core/SkYUVAInfo.h"
#include "include/core/SkYUVASizeInfo.h"

#ifndef SkYUVAInfoPriv_DEFINED
#define SkYUVAInfoPriv_DEFINED

class SkPixmap;

namespace SkYUVAInfoPriv {

bool InitLegacyInfo(SkYUVAInfo yuvaInfo,
                    const SkPixmap planes[SkYUVAInfo::kMaxPlanes],
                    SkYUVASizeInfo* yuvaSizeInfo,
                    SkYUVAIndex yuvaIndices[SkYUVAIndex::kIndexCount]);

}  // namespace SkYUVAInfoPriv

#endif
