/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkArSession.h"
#include "SkArUtil.h"
#include "SkArFrame.h"

/**
 * Given an environment pointer and an app context pointer, create a shared pointer to an
 * SkArSession.
 * @param env       Native Application handle
 * @param context   Application Context
 * @return          nullptr if failed to create session, or shared ptr to SkArSession otherwise
 */
sk_sp<SkArSession> SkArSession::Make(void* env, void* context) {
    sk_sp<SkArSession> outSession(new SkArSession(env, context));
    if (!outSession.get()) {
        return nullptr;
    } else {
        return outSession;
    }
}

SkArSession::SkArSession(void* env, void* context)
        : fArSession(nullptr) {
    if (ArSession_create(env, context, &fArSession) != AR_SUCCESS) {
        fArSession = nullptr;
        SKAR_LOGI("SkArSession: Failure Creating Session");
    }
    SKAR_LOGI("SkArSession: Success Creating Session");
}

SkArSession::~SkArSession() {
    ArSession_destroy(fArSession);
}

bool SkArSession::pause() {
    return ArSession_pause(fArSession) == AR_SUCCESS;
}

bool SkArSession::resume() {
    return ArSession_resume(fArSession) == AR_SUCCESS;
}

bool SkArSession::update(sk_sp<SkArFrame> arFrame) {
    return ArSession_update(fArSession, arFrame->getArFrame()) == AR_SUCCESS;
}

void SkArSession::setDisplayGeometry(int32_t rotation, int32_t width, int32_t height) {
    ArSession_setDisplayGeometry(fArSession, rotation, width, height);
}

void SkArSession::setCameraTextureName(uint32_t textureId) {
    ArSession_setCameraTextureName(fArSession, textureId);
}

const ArSession* SkArSession::getArSession() const {
    return fArSession;
}

ArSession* SkArSession::getArSession() {
    return fArSession;
}