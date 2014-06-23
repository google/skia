/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPictureRecorder_DEFINED
#define SkPictureRecorder_DEFINED

#include "SkBBHFactory.h"
#include "SkPicture.h"
#include "SkRefCnt.h"

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
namespace android {
    class Picture;
};
#endif

class SkCanvas;
class SkPictureRecord;
class SkRecord;
class SkRecorder;

class SK_API SkPictureRecorder : SkNoncopyable {
public:
    SkPictureRecorder();
    ~SkPictureRecorder();

    /** Returns the canvas that records the drawing commands.
        @param width the base width for the picture, as if the recording
                     canvas' bitmap had this width.
        @param height the base width for the picture, as if the recording
                     canvas' bitmap had this height.
        @param bbhFactory factory to create desired acceleration structure
        @param recordFlags optional flags that control recording.
        @return the canvas.
    */
    SkCanvas* beginRecording(int width, int height,
                             SkBBHFactory* bbhFactory = NULL,
                             uint32_t recordFlags = 0);

    /** Same as beginRecording(), using a new faster backend. */
    SkCanvas* EXPERIMENTAL_beginRecording(int width, int height,
                                          SkBBHFactory* bbhFactory = NULL);

    /** Returns the recording canvas if one is active, or NULL if recording is
        not active. This does not alter the refcnt on the canvas (if present).
    */
    SkCanvas* getRecordingCanvas();

    /** Signal that the caller is done recording. This invalidates the canvas
        returned by beginRecording/getRecordingCanvas, and returns the
        created SkPicture. Note that the returned picture has its creation
        ref which the caller must take ownership of.
    */
    SkPicture* endRecording();

    /** Enable/disable all the picture recording optimizations (i.e.,
        those in SkPictureRecord). It is mainly intended for testing the
        existing optimizations (i.e., to actually have the pattern
        appear in an .skp we have to disable the optimization). Call right
        after 'beginRecording'.
    */
    void internalOnly_EnableOpts(bool enableOpts);

private:
    void reset();

    /** Replay the current (partially recorded) operation stream into
        canvas. This call doesn't close the current recording.
    */
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    friend class android::Picture;
#endif
    friend class SkPictureRecorderReplayTester; // for unit testing
    void partialReplay(SkCanvas* canvas) const;

    int fWidth;
    int fHeight;

    // One of these two canvases will be non-NULL.
    SkAutoTUnref<SkPictureRecord> fPictureRecord;  // beginRecording()
    SkAutoTUnref<SkRecorder>      fRecorder;       // EXPERIMENTAL_beginRecording()

    // Used by EXPERIMENTAL_beginRecording().
    SkAutoTDelete<SkRecord> fRecord;

    typedef SkNoncopyable INHERITED;
};

#endif
