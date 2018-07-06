/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkArSession_DEFINED
#define SkArSession_DEFINED

#include <SkRefCnt.h>
#include <arcore_c_api.h>

class SkArFrame;

class SkArSession : public SkRefCnt {
public:

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
     * @param rotation Display rotation specified by @c android.view.Surface
     *                 constants: @c ROTATION_0, @c ROTATION_90, @c ROTATION_180 and
     *                 @c ROTATION_270
     * @param width    Width of the view, in pixels
     * @param height   Height of the view, in pixels
     */
    void setDisplayGeometry(int32_t rotation, int32_t width, int32_t height);

    //REVIEWER NOTE: This will change soon once I decide how to create the background texture
    void setBackendTextureFromCamera(uint32_t textureId);

    const ArSession* getArSession() const;

    ArSession* getArSession();

private:
    SkArSession(void* env, void* context);

    // ArSession is managed by SkArSession
    ArSession* fArSession;

    typedef SkRefCnt INHERITED;
};

#endif  // SkArSession_DEFINED
