/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkArCamera_DEFINED
#define SkArCamera_DEFINED

#include <memory>
#include "SkArFrame.h"
#include "SkArSession.h"
#include "SkArTrackingState.h"

class SkArSession;

class SkArCamera {
    /**
     * Provides information about the camera that is used to capture images. Such information
     * includes projection matrices, pose of camera...
     */

public:
    /**
     * Factory method used to construct an SkArCamera from the current frame, using the current
     * session
     * @param session   shared pointer to the current SkArSession
     * @param frame     shared pointer to the current SkArFrame
     * @return          unique pointer to an SkArCamera. Never nullptr
     */
    static std::unique_ptr<SkArCamera> Make(sk_sp<SkArSession> session, sk_sp<SkArFrame> frame);

    ~SkArCamera();

    /**
     * Fills outColMajor with the values of the camera's current View matrix in column-major order
     * @param session       current SkArSession
     * @param outColMajor   16-float array that will contain the View matrix content
     */
    void getViewMatrix(const sk_sp<SkArSession> session, float outColMajor[16]);

    /**
     * Fills outColMajor with the values of the camera's current Projection matrix in
     * column-major order
     * @param session       current SkArSession
     * @param nearClip      wanted near clip value for the camera
     * @param farClip       wanted far clip value for the camera
     * @param outColMajor   16-float array that will contain the Projection matrix content
     */
    void getProjectionMatrix(const sk_sp<SkArSession> session, float nearClip, float farClip,
                             float outColMajor[16]);

    /**
     * Used to check the current SkArTrackingState of the camera
     * @param session           current SkArSession
     * @param trackingState     out variable containing state
     */
    void getTrackingState(const sk_sp<SkArSession> session, SkArTrackingState& trackingState);

private:
    SkArCamera(sk_sp<SkArSession> session, sk_sp<SkArFrame> frame);

    // This is a raw pointer. Its lifetime matches that of this class (SkArCamera)
    ArCamera* fArCamera;
};
#endif  // SkArCamera_DEFINED
