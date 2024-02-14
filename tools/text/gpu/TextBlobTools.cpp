/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/text/gpu/TextBlobTools.h"

#include "src/text/gpu/SubRunContainer.h"
#include "src/text/gpu/TextBlob.h"

namespace sktext::gpu {

const AtlasSubRun* TextBlobTools::FirstSubRun(const TextBlob* blob) {
    SkASSERT(blob);
    if (blob->fSubRuns->fSubRuns.isEmpty()) {
        return nullptr;
    }
    return blob->fSubRuns->fSubRuns.front().testingOnly_atlasSubRun();
}

}  // namespace sktext::gpu
