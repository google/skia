/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/graphite/precompile/PipelineCallbackHandler.h"

namespace skiatools::graphite {

void PipelineCallBackHandler::add(sk_sp<SkData> payload) {
    SkAutoSpinlock lock{ fSpinLock };

    const sk_sp<SkData>* data = fMap.find({ payload.get() });
    if (!data) {
        fMap.set(std::move(payload));
    }
}

void PipelineCallBackHandler::retrieve(
        std::vector<sk_sp<SkData>>* result) {
    SkAutoSpinlock lock{ fSpinLock };

    result->reserve(fMap.count());

    fMap.foreach([result](sk_sp<SkData>* data) {
        result->push_back(*data);
    });
}

void PipelineCallBackHandler::reset() {
    SkAutoSpinlock lock{ fSpinLock };

    fMap.reset();
}

}  // namespace skiatools::graphite
