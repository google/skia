/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkArSession_DEFINED
#define SkArSession_DEFINED

#include "SkRefCnt.h"

class ArSession;
class SkArFrame;
class SkArSessionPriv;
class SkArTrackable;

/**
 * Manages the lifecycle of the AR engine running on the app. Use this to query updates made on the
 * frame, and to pause/resume the session itself. Each AR app should manage one SkArSession only,
 * and as a smart pointer.
 */

class SkArSession : public SkRefCnt {
public:

    /**
     * Enum denoting the View orientation
     */
    enum DisplayRotation {
        kRotation0,
        kRotation90,
        kRotation180,
        kRotation270,
    };

    /**
     * Factory method used to create a shared pointer to an SkArSession given an environment pointer
     * and an app context pointer
     * @param env       Native Application handle
     * @param context   Application Context
     * @return          nullptr if failed to create session, or shared ptr to SkArSession otherwise
     */
    static sk_sp<SkArSession> Make(void* env, void* context);

    ~SkArSession();

    /**
     * Pauses the current session. Stops camera feed and releases resources. Resume session by
     * calling resume().
     * @return true if pause() executed successfully, false otherwise
     */
    bool pause();

    /**
     * Starts or resumes the session.
     * @return true if resume() executed successfully, false otherwise
     */
    bool resume();

    /**
     * Updates state of the AR system. This includes receiving a new camera frame, updating device's
     * location, updating tracking anchors, updating detected planes...
     * @param arFrame the frame object to populate the updated world state. Must have been
     *                initialized previously though SkArFrame::Make()
     * @return true if update() executed successfully, false otherwise
     */
    bool update(SkArFrame* arFrame);

    /**
     * Sets the aspect ratio, coordinate scaling, and display rotation. This data
     * is used by UV conversion, projection matrix generation, and hit test logic.
     * Note: this function doesn't fail. If given invalid input, it logs a error
     * and doesn't apply the changes.
     * @param rotation Display rotation specified by the App's View
     * @param width    Width of the view, in pixels
     * @param height   Height of the view, in pixels
     */
    void setDisplayGeometry(DisplayRotation rotation, int width, int height);

    /**
     * TODO: switch this over to using a GrBackendTexture
     */
    void setBackendTextureFromCamera(uint32_t textureId);

    /**
     * Returns a vector of unique pointers to SkArTrackables managed by the current session.
     * @param filter   type of SkArTrackable to return (Plane, Point, ...)
     */
    std::vector<std::unique_ptr<SkArTrackable>> getAllTrackables(SkArTrackableType filter);

    // Provides access to functions that aren't part of the public API.
    SkArSessionPriv priv();
    const SkArSessionPriv priv() const;

private:
    SkArSession(void* env, void* context);

    /**
     * @return immutable ARCore-backed Session contained within this SkArsession
     */
    const ArSession* getArSession() const;

    /**
     * @return ARCore-backed Session contained within this SkArsession
     */
    ArSession* getArSession();

    // ArSession is managed by SkArSession
    ArSession* fArSession;

    friend SkArSessionPriv;

    typedef SkRefCnt INHERITED;
};

#endif  // SkArSession_DEFINED
