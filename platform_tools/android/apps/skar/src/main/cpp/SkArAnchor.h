/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKAR_SKARANCHOR
#define SKAR_SKARANCHOR

#include <memory>
#include <SkRefCnt.h>
#include "arcore_c_api.h"
#include "glm.h"
#include "SkArSession.h"
#include "SkArPose.h"

class SkArAnchor {
public:
    static std::unique_ptr<SkArAnchor> Make(sk_sp<SkArSession> session,
                                            std::unique_ptr<SkArPose> pose);
    ~SkArAnchor();

    glm::vec3 getAnchorPos() const;
    glm::vec4 getAnchorQuat() const;
    glm::mat4 getAnchorModelMatrix() const;
private:
    SkArAnchor(sk_sp<SkArSession> arSession, std::unique_ptr<SkArPose> arPose);

    // ArAnchor is managed by SkArAnchor
    ArAnchor* fArAnchor;

    std::unique_ptr<SkArPose> fSkArPose;
    const sk_sp<SkArSession> fSkArSession;
};
#endif
