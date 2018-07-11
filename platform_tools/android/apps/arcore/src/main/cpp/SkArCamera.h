/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkArCamera_DEFINED
#define SkArCamera_DEFINED

#include <memory>
#include "SkArTrackingState.h"

class ArCamera;
class SkArFrame;
class SkArSession;

/**
 * Provides information about the camera that is used to capture images. Such information
 * includes projection matrices, pose of camera...
 */

class SkArCamera {

public:
    /**
     * Factory method used to construct an SkArCamera from the current frame, using the current
     * session
     * @param session   raw pointer to the current SkArSession
     * @param frame     raw pointer to the current SkArFrame
     * @return          unique pointer to an SkArCamera. Never nullptr
     */
    static std::unique_ptr<SkArCamera> Make(SkArSession* session, SkArFrame* frame);

    ~SkArCamera();

    /**
     * Fills outColMajor with the values of the camera's current View matrix in column-major order
     * @param session       current SkArSession
     * @param outColMajor   16-float array that will contain the View matrix content
     */
    void getViewMatrix(const SkArSession* session, float outColMajor[16]);

    /**
     * Fills outColMajor with the values of the camera's current Projection matrix in
     * column-major order
     * @param session       current SkArSession
     * @param nearClip      wanted near clip value for the camera
     * @param farClip       wanted far clip value for the camera
     * @param outColMajor   16-float array that will contain the Projection matrix content
     */
    void getProjectionMatrix(const SkArSession* session, float nearClip, float farClip,
                             float outColMajor[16]);

    /**
     * Used to check the current SkArTrackingState of the camera
     * @param session   current SkArSession
     * @return          tracking state of the SkArCamera described by the SkArTrackingState enum
     */
    SkArTrackingState getTrackingState(const SkArSession* session);

private:
    SkArCamera(SkArSession* session, SkArFrame* frame);

    // This is a raw pointer. Its lifetime matches that of this class (SkArCamera)
    ArCamera* fArCamera;
};
#endif  // SkArCamera_DEFINED
