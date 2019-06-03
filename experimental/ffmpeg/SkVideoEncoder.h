/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkVideEncoder_DEFINED
#define SkVideEncoder_DEFINED

#include "include/core/SkImage.h"
#include "include/core/SkStream.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavutil/pixdesc.h"
}

// private to the impl
class SkRandomAccessWStream;
struct SwsContext;

class SkVideoEncoder {
public:
    SkVideoEncoder();
    ~SkVideoEncoder();

    /**
     *  Begina a new recording. Balance this (after adding all of your frames) with a call
     *  to endRecording().
     */
    bool beginRecording(SkISize, int fps);

    /**
     *  Returns the preferred ImageInfo for this recording. Only valid if beginRecording() has
     *  been called, and endRecording has not been called (yet).
     */
    SkImageInfo preferredInfo() const { return fInfo; }

    /**
     *  If you have your own pixmap, call addFrame(). Note this may fail if it uses an unsupported
     *  ColorType or AlphaType, or the dimensions don't match those set in beginRecording.
     *  For best results, use the SkImageInfo returned by preferredInfo().
     */
    bool addFrame(const SkPixmap&);

    /**
     *  As an alternative to calling addFrame(), you can call beginFrame/endFrame, and the encoder
     *  will manage allocating a surface/canvas for you.
     *
     *  SkCanvas* canvas = encoder.beginFrame();
     *  // your drawing code here, drawing into canvas
     *  encoder.endFrame();
     */
    SkCanvas* beginFrame();
    bool endFrame();

    /**
     *  Call this after having added all of your frames. After calling this, no more frames can
     *  be added to this recording. To record a new video, call beginRecording().
     */
    sk_sp<SkData> endRecording();

private:
    void reset();
    bool init(int fps);
    bool sendFrame(AVFrame*);   // frame can be null

    double computeTimeStamp(const AVFrame*) const;

    SwsContext*     fSWScaleCtx = nullptr;
    AVIOContext*    fStreamCtx = nullptr;
    AVFormatContext* fFormatCtx = nullptr;
    AVCodecContext* fEncoderCtx = nullptr;
    AVStream*       fStream = nullptr;  // we do not free this
    AVFrame*        fFrame = nullptr;
    AVPacket*       fPacket = nullptr;

    SkImageInfo     fInfo;  // only defined between beginRecording() and endRecording()
    std::unique_ptr<SkRandomAccessWStream> fWStream;
    int64_t         fCurrentPTS, fDeltaPTS;

    // Lazily allocated, iff the client has called beginFrame() for a given recording session.
    sk_sp<SkSurface> fSurface;

};

#endif

