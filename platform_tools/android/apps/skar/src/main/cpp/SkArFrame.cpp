/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArFrame.h"
#include "SkArUtil.h"

sk_sp<SkArFrame> SkArFrame::Make(sk_sp<SkArSession> arSession) {
    sk_sp<SkArFrame> outFrame(new SkArFrame(std::move(arSession)));
    if (!outFrame->fArFrame) {
        SKAR_LOGI("SkArFrame: Failure Creating Frame");
        return nullptr;
    }
    SKAR_LOGI("SkArFrame: Success Creating Frame");
    return outFrame;
}

SkArFrame::~SkArFrame() {
    ArFrame_destroy(fArFrame);
}

const ArFrame* SkArFrame::getArFrame() const {
    return fArFrame;
}

ArFrame* SkArFrame::getArFrame() {
    return fArFrame;
}

SkArFrame::SkArFrame(sk_sp<SkArSession> arSession)
    : fArFrame(nullptr) {
    ArFrame_create(arSession->getArSession(), &fArFrame);
}