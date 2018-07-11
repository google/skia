/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <vector>
#include "SkArSession.h"
#include "arcore_c_api.h"
#include "SkArFrame.h"
#include "SkArSessionPriv.h"
#include "SkArUtil.h"


using namespace std;

// Helper function to convert from DisplayRotation to the correct backend display rotation
int32_t convert_display_rotation(SkArSession::DisplayRotation rot) {
    switch (rot) {
        case SkArSession::kRotation0:
            return 0;
        case SkArSession::kRotation90:
            return 1;
        case SkArSession::kRotation180:
            return 2;
        case SkArSession::kRotation270:
            return 3;
        default:
            return 0;
    }
}

sk_sp<SkArSession> SkArSession::Make(void* env, void* context) {
    sk_sp<SkArSession> outSession(new SkArSession(env, context));

    if (!outSession->fArSession) {
        SKAR_LOGI("SkArSession: Failure Creating Session");
        return nullptr;
    }
    SKAR_LOGI("SkArSession: Success Creating Session");
    return outSession;
}

SkArSession::SkArSession(void* env, void* context) : fArSession(nullptr) {
    if (ArSession_create(env, context, &fArSession) != AR_SUCCESS) {
        fArSession = nullptr;
    }
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

bool SkArSession::update(SkArFrame* arFrame) {
    return ArSession_update(fArSession, arFrame->getArFrame()) == AR_SUCCESS;
}

void SkArSession::setDisplayGeometry(DisplayRotation rotation, int width, int height) {
    ArSession_setDisplayGeometry(fArSession, convert_display_rotation(rotation), width, height);
}

void SkArSession::setBackendTextureFromCamera(uint32_t textureId) {
    ArSession_setCameraTextureName(fArSession, textureId);
}

vector<unique_ptr<SkArTrackable>> SkArSession::getAllTrackables(SkArTrackableType filter) {
    // Initialize structs
    ArTrackableList* trackableList = nullptr;
    int32_t listSize = 0;
    ArTrackableList_create(fArSession, &trackableList);
    if (!trackableList) {
        return vector<unique_ptr<SkArTrackable>>(0);
    }

    // Get all trackables from current session
    ArSession_getAllTrackables(fArSession, SkArUtil::MakeArTrackableType(filter), trackableList);

    // Populate the vector with SkArTrackable objects
    vector<unique_ptr<SkArTrackable>> outVector(listSize);
    for (int i = 0; i < listSize; i++) {
        ArTrackable* currTrackable = nullptr;
        ArTrackableList_acquireItem(fArSession, trackableList, i, &currTrackable);

        unique_ptr<SkArTrackable> currSkArTrackable(new SkArTrackable(this, currTrackable));
        outVector.push_back(std::move(currSkArTrackable));
    }

    ArTrackableList_destroy(trackableList); // NOTE: THIS MIGHT DE-ALLOCATE TRACKABLE LIST, CHECK IT
    return outVector;
}

const ArSession* SkArSession::getArSession() const {
    return fArSession;
}

ArSession* SkArSession::getArSession() {
    return fArSession;
}
