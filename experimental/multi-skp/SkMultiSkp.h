/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMultiSkp_DEFINED
#define SkMultiSkp_DEFINED

#include <queue>
#include <condition_variable>
#include <mutex>

#include "../private/SkNoncopyable.h"
#include "SkPictureRecorder.h"
#include "SkStream.h"

namespace multiskp {

// A header written at the beginning of the file.
struct FileHeader {
public:
    char fMagic[8];// = "multiskp";

    // The number of frames that are written to this file
    int32_t numFrames;
};

// A header written at the beginning of each frame.
struct FrameHeader {
public:
    // Indicates that after this struct there follows this many bytes representing the contents of
    // the buffer that was provided to this frame. This is non-zero on the first few recorded frames
    // so that the skp can be-replayed in the same way it is on Android, re-using buffer contents
    // where possible.
    int32_t hardwareBufferBytes;

    // The age of the buffer in frames.
    // For example, 1 means this buffer contains the last frame's content.
    int32_t bufferAge;

    // Further information about each frame, is recorded in the SkPictInfo at the head of each
    // serialized SkPicture
    // The dirty region for this frame is recorded as info.fCullRect
};

// A class to manage the creation of a multi-frame skp file
// To be used as an alternative to using SkPictureRecorder to record one frame.
// this class manages a SkPictureRecorder for you.
// TODO(nifong): create a reader class or reading capability.
class SkMultiSkpWriter : SkNoncopyable {
public:
    // Create a multi-frame SKP writer to capture n frames with the given filename.
    SkMultiSkpWriter(int frames, std::string fname);
    ~SkMultiSkpWriter();

    // Start recording the next frame. Returns a recording canvas. Issue commands to this canvas.
    SkCanvas* startFrame(SkRect bounds);

    // Finish a frame.
    //
    void finishFrame();

private:
    // The main fuction of the file writing thread
    void writingFn();

    // Optionally wait to be notified of new data, then pop one item from the queue
    // and serialize it to the output file.
    void tryPop(bool wait);

    std::string filename;

    // The number of frames remaining to be written out. Used as a signal to terminate the
    // background thread when the work is done.
    int framesRemaining;

    std::unique_ptr<SkFILEWStream> stream;

    // A picture recorder. This one is used for the currently recorded frame.
    // When the frame finishes, it is moved onto the queue consumed by the background thread.
    // A new one is initialized at the start of the next frame.
    std::unique_ptr<SkPictureRecorder> mRecorder;

    // A queue of picture recorders waiting to be serialized and written in the background thread.
    // After being serialized, they are released.
    std::queue<std::unique_ptr<SkPictureRecorder>> mWorkQueue;

    // A mutex to be locked by whichever thread is making changes to mWorkQueue
    std::mutex mWorkQueueMutex;

    // A condition variable for notifying the background thread that new work is available.
    std::condition_variable cv;

    typedef SkNoncopyable INHERITED;
};

} // namespace multiskp
#endif
