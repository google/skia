/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkMultiSkp.h"

#include <thread>
#include <queue>
#include <memory>

#include "SkStream.h"
#include "SkString.h"

namespace multiskp {
namespace {

// A header written at the beginning of the file.
struct FileHeader {
    char fMagic[8];// = "multiskp";

    // The number of frames that are written to this file
    int32_t numFrames;
};

// A header written at the beginning of each frame.
struct FrameHeader {
    // Indicates that after this struct there follows this many bytes representing the contents of
    // the buffer that was provided to this frame. This is non-zero on the first few recorded frames
    // so that the skp can be-replayed in the same way it is on Android, re-using buffer contents
    // where possible.
    int32_t hardwareBufferBytes;

    // The age of the buffer in frames.
    // For example, 1 means this buffer contains the last frame's content.
    int32_t bufferAge;

    // Further information about each frame is recorded in the SkPictInfo at the head of each
    // serialized SkPicture
    // The dirty region for this frame is recorded as SkPictInfo.fCullRect
};

} // namespace

SkMultiSkpWriter::SkMultiSkpWriter(int frames, std::unique_ptr<SkFILEWStream> stream) {
    fStream = std::move(stream);
    fFramesRemaining = frames;
    // create and detach background thread to handle file writing.
    std::thread writingThread(&SkMultiSkpWriter::writingFn, this);
    writingThread.detach();
}

SkMultiSkpWriter::~SkMultiSkpWriter() {}

std::unique_ptr<SkMultiSkpWriter> SkMultiSkpWriter::Make(int frames, SkString filename) {
    auto stream = std::make_unique<SkFILEWStream>(filename.c_str());
    if (!stream->isValid()) {
        SkDebugf("Could not open \"%s\" for writing.", filename.c_str());
        return nullptr;
    }

    return std::unique_ptr<SkMultiSkpWriter>(new SkMultiSkpWriter(frames, std::move(stream)));
}

// Called from UI thread at the beginning of each recorded frame
SkCanvas* SkMultiSkpWriter::startFrame(SkRect bounds) {
    fRecorder.reset(new SkPictureRecorder());
    return fRecorder->beginRecording(bounds, nullptr,
        SkPictureRecorder::kPlaybackDrawPicture_RecordFlag);
}

// Called from the UI thread at the end of each recorded frame.
void SkMultiSkpWriter::finishFrame() {
    SkDebugf("frame finished");
    // enqueue recorded data for writing thread to handle in background.
    std::unique_lock<std::mutex> lk(fWorkQueueMutex);
    fWorkQueue.push(std::move(fRecorder));
    lk.unlock();
    // TODO(nifong): investigate if trying to wake up the background thread every frame is harming
    // render thread smoothness
    fCv.notify_one();
}

// Called once in background thread at the beginning of recording.
void SkMultiSkpWriter::writingFn() {
    // The purpose of the separate thread is for the UI thread to not be
    // excessively blocked by the work of serializing SkPictures. The UI thread records the
    // SkPicture and enqueues it in a shared queue protected by a mutex. A background thread
    // consumes all the data in that queue, serializes the SkPictures and writes them to the file.
    // It knows it's done when it counts off the correct number of pictures.

    // It does not use SkExecutor because the order of the written data must be guaranteed.
    // It not use absl because that library is not used anywhere else in Skia.

    FileHeader fileheader;
    strcpy(fileheader.fMagic, "multiskp");
    fileheader.numFrames = fFramesRemaining;
    fStream->write(&fileheader, sizeof(fileheader));

    while (fFramesRemaining > 0) {
        auto recorder = tryPop();

         // Write a frame header at the beginning of each frame
        FrameHeader fh;
        fh.hardwareBufferBytes = 0;
        fh.bufferAge = 1; // TODO(nifong): determine the real value of buffer age
        fStream->write(&fh, sizeof(fh));

        sk_sp<SkPicture> picture = recorder->finishRecordingAsPicture();
        auto data = picture->serialize();
        fStream->write(data->data(), data->size());
        fStream->flush();
        fFramesRemaining--;
    }
    // The expected number of frames for this file were popped from the queue. File is done.
    SkDebugf("Serializing SKP - Finished");
}

std::unique_ptr<SkPictureRecorder> SkMultiSkpWriter::tryPop() {
    SkDebugf("Serializing SKP - %d frames remaining", fFramesRemaining);
    std::unique_lock<std::mutex> lk(fWorkQueueMutex);
    // Only enter wait if queue is empty and we're expecting more frames.
    while (fWorkQueue.empty()) {
        fCv.wait(lk);
    }
    // Remove one SkPicture from the work queue.
    auto recorder = std::move(fWorkQueue.front());
    fWorkQueue.pop();
    return recorder;
}


} // namespace multiskp
