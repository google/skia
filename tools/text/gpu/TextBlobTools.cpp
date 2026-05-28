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

const AtlasSubRun* TextBlobTools::FirstSubRun(const SubRunContainer* container) {
    SkASSERT(container);
    if (container->fSubRuns.isEmpty()) {
        return nullptr;
    }
    return container->fSubRuns.front().testingOnly_atlasSubRun();
}

const AtlasSubRun* TextBlobTools::FirstSubRun(const TextBlob* blob) {
    SkASSERT(blob);
    return FirstSubRun(blob->fSubRuns.get());
}

}  // namespace sktext::gpu
