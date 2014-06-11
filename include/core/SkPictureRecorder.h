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

class SkCanvas;
class SkPictureRecord;

class SK_API SkPictureRecorder : SkNoncopyable {
public:
    SkPictureRecorder() : fCanvas(NULL) { }
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
    /** Replay the current (partially recorded) operation stream into
        canvas. This call doesn't close the current recording.
    */
    friend class AndroidPicture;
    friend class SkPictureRecorderReplayTester; // for unit testing
    void partialReplay(SkCanvas* canvas) const;

    int                     fWidth;
    int                     fHeight;
    SkPictureRecord*        fCanvas;   // ref counted

    typedef SkNoncopyable INHERITED;
};

#endif
