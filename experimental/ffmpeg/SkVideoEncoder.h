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

class SkVideoEncoder {
public:
    SkVideoEncoder();
    ~SkVideoEncoder();

    bool beginRecording(const SkImageInfo&, int fps);

    SkCanvas* beginFrame();
    bool endFrame();

    sk_sp<SkData> endRecording();

private:
    void reset();
    bool init(const SkImageInfo&, int fps);
    bool sendFrame(AVFrame*);   // frame can be null

    double computeTimeStamp(const AVFrame*) const;

    AVIOContext*    fStreamCtx = nullptr;
    AVFormatContext* fFormatCtx = nullptr;
    AVCodecContext* fEncoderCtx = nullptr;
    AVStream*       fStream = nullptr;  // we do not free this
    AVFrame*        fFrame = nullptr;
    AVPacket*       fPacket = nullptr;

    sk_sp<SkSurface> fSurface;
    std::unique_ptr<SkRandomAccessWStream> fWStream;

    int64_t          fCurrentPTS, fDeltaPTS;
};

#endif

