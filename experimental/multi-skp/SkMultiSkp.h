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
#include "SkString.h"

namespace multiskp {

// A class to manage the creation of a multi-frame skp file
// To be used as an alternative to using SkPictureRecorder to record one frame.
// this class manages an SkPictureRecorder for you.
// TODO(nifong): create a reader class or reading capability.
class SkMultiSkpWriter : SkNoncopyable {
public:
    // Constructor is not meant to be used publicly, use Make instead.
    // It is public only so the factory can use make_unique
    SkMultiSkpWriter(int frames, std::unique_ptr<SkFILEWStream> stream);
    ~SkMultiSkpWriter();

    // Create a writer with a given filename, expecting a given number of frames.
    // May return null if the filesystem was not writable or some other problem. (look for logs)
    static std::unique_ptr<SkMultiSkpWriter> Make(int frames, SkString filename);

    // Start recording the next frame. Returns a recording canvas. Issue commands to this canvas.
    SkCanvas* startFrame(SkRect bounds);

    // Finish a frame.
    // Should be called from the same thread startFrame was called from. (the render thread)
    void finishFrame();

private:

    // The main function of the file writing thread
    void writingFn();

    // Optionally wait to be notified of new data, then pop one item from the queue
    // and serialize it to the output file.
    void tryPop(bool wait);

    std::string fFilename;

    // The number of frames remaining to be written out. Used as a signal to terminate the
    // background thread when the work is done.
    int fFramesRemaining;

    // The stream where the data is written
    std::unique_ptr<SkFILEWStream> fStream;

    // A picture recorder. This one is used for the currently recorded frame.
    // When the frame finishes, it is moved onto the queue consumed by the background thread.
    // A new one is initialized at the start of the next frame.
    std::unique_ptr<SkPictureRecorder> fRecorder;

    // A queue of picture recorders waiting to be serialized and written in the background thread.
    // After being serialized, they are released.
    std::queue<std::unique_ptr<SkPictureRecorder>> fWorkQueue;

    // A mutex to be locked by whichever thread is making changes to mWorkQueue
    std::mutex fWorkQueueMutex;

    // A condition variable for notifying the background thread that new work is available.
    std::condition_variable fCv;

    typedef SkNoncopyable INHERITED;
};

} // namespace multiskp
#endif
