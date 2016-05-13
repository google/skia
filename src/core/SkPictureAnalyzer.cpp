/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPicture.h"
#include "SkPictureAnalyzer.h"

#if SK_SUPPORT_GPU

namespace {

inline bool veto_predicate(uint32_t numSlowPaths) {
    return numSlowPaths > 5;
}

} // anonymous namespace

SkPictureGpuAnalyzer::SkPictureGpuAnalyzer(sk_sp<GrContextThreadSafeProxy> /* unused ATM */)
    : fNumSlowPaths(0) { }

SkPictureGpuAnalyzer::SkPictureGpuAnalyzer(const sk_sp<SkPicture>& picture,
                                           sk_sp<GrContextThreadSafeProxy> ctx)
    : SkPictureGpuAnalyzer(std::move(ctx)) {
    this->analyze(picture.get());
}

void SkPictureGpuAnalyzer::analyze(const SkPicture* picture) {
    if (!picture || veto_predicate(fNumSlowPaths)) {
        return;
    }

    fNumSlowPaths += picture->numSlowPaths();
}

void SkPictureGpuAnalyzer::reset() {
    fNumSlowPaths = 0;
}

bool SkPictureGpuAnalyzer::suitableForGpuRasterization(const char** whyNot) const {
    if(veto_predicate(fNumSlowPaths)) {
        if (whyNot) { *whyNot = "Too many slow paths (either concave or dashed)."; }
        return false;
    }
    return true;
}

#endif // SK_SUPPORT_GPU
