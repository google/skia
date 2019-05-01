/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <thread>
#include <queue>
#include <memory>
#include <cstring>

// for ALOGE
#include <utils/Log.h>

#include "SkMultiSkpWriter.h"
#include "SkStream.h"

namespace multiskp {

// The file header always includes a magic number indicating this is a multi-frame skp.
//static const char kMultiMagic[] = { 'm', 'u', 'l', 't', 'i', 's', 'k', 'p' };

SkMultiSkpWriter::SkMultiSkpWriter(int frames, std::string fname) {
    filename = fname;
    framesRemaining = frames;
    // create and detach backround thread to handle file writing.
    std::thread writingThread(&SkMultiSkpWriter::writingFn, this);
    writingThread.detach();
}

SkMultiSkpWriter::~SkMultiSkpWriter() {}

// Called from UI thread at the beginning of each recorded frame
SkCanvas* SkMultiSkpWriter::startFrame(SkRect bounds) {
    mRecorder.reset(new SkPictureRecorder());
    SkCanvas* canvas = mRecorder->beginRecording(bounds, nullptr,
                                              SkPictureRecorder::kPlaybackDrawPicture_RecordFlag);
    return canvas;
}

// Called from the UI thread at the end of each recorded frame.
void SkMultiSkpWriter::finishFrame() {
    ALOGE("finishFrame");
    // enqueue recorded data for writing thread to handle in background.
    std::unique_lock<std::mutex> lk(mWorkQueueMutex);
    mWorkQueue.push(std::move(mRecorder));
    lk.unlock();
    cv.notify_one();
}

// Called once in background thread at the beginning of recording.
void SkMultiSkpWriter::writingFn() {
    ALOGE("writingFn");
    // TODO how do we get access to this?
    //setpriority(0, 0, 10 /* 10 = PRIORITY_BACKGROUND */);

    // The purpose of the seperate thread is for the UI thread to not be
    // excessively blocked by the work of serializing SkPictures. The UI thread records the
    // SkPicture and enqueues it in a shared queue protected by a mutex. A background thread
    // consumes all the data in that queue, serializes the SkPictures and writes them to the file.
    // It knows it's done when it counts off the correct number of pictures.

    // The threading here does not use absl because this is meant to be included in Android.

    // Open file
    stream.reset(new SkFILEWStream(filename.c_str()));

    FileHeader fileheader;
    strcpy(fileheader.fMagic, "multiskp");
    fileheader.numFrames = framesRemaining;
    stream->write(&fileheader, sizeof(fileheader));

    while(framesRemaining > 0) {
        // Wait for data, when we get something, serialize and write the frame.
        tryPop(true);
        // As long as we're awake, consume all the data in queue, since we may not
        // be woken up again until more is written.
        while(!mWorkQueue.empty()) {
            // Pass false to indicate that we should not enter wait between processing records
            tryPop(false);
        }
    }
    // The expected number of frames for this file were written. File is done.
}

void SkMultiSkpWriter::tryPop(bool wait) {
    ALOGE("Serializing SKP - %d frames remaining", framesRemaining);
    std::unique_ptr<SkPictureRecorder> recorder;
    {
        std::unique_lock<std::mutex> lk(mWorkQueueMutex);
        if (wait) {
            // Wait is passed a predicate which returns false if the waiting should be continued.
            cv.wait(lk, [this]{ return framesRemaining>0 && !mWorkQueue.empty(); });
            // After the wait, we own the lock.
        }
        // Remove one SkPicture from the work queue.
        recorder = std::move(mWorkQueue.front());
        mWorkQueue.pop();
        // Lock is released at the end of this scope.
    }
    // Write a frame header at the begining of each frame
    FrameHeader fh;
    fh.hardwareBufferBytes = 0;
    fh.bufferAge = 1; // TODO(nifong): determine the real value of buffer age
    stream->write(&fh, sizeof(fh));

    sk_sp<SkPicture> picture = recorder->finishRecordingAsPicture();
    auto data = picture->serialize();
    if (stream->isValid()) {
        stream->write(data->data(), data->size());
        stream->flush();
    }
    framesRemaining--;
}

} // namespace multiskp
