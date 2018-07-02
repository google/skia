/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkArSession.h"

namespace SkAR {
    sk_sp<SkArSession> SkArSession::Make(void* env, void* context, void* activity) {
        return sk_sp<SkArSession>(new SkArSession(env, context, activity));
    }

    SkArSession::SkArSession(void* env, void* context, void* activity)
            : fArSession(nullptr) {
        SKAR_CHECK(ArSession_create(env, context, &fArSession) == AR_SUCCESS)
        SKAR_LOGI("SkArSession: Success Creating Session");
        sk_sp<SkArSession>(new SkArSession(env, context, activity));
    }

    SkArSession::~SkArSession() {
        ArSession_destroy(fArSession);
    }

    const ArSession* SkArSession::getArSession() const {
        return fArSession;
    }

    ArSession* SkArSession::getArSession() {
        return fArSession;
    }
}  // namespace SkAr
