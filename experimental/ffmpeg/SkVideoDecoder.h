/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkVideDecoder_DEFINED
#define SkVideDecoder_DEFINED

#include "include/core/SkImage.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/pixdesc.h"
}

class SkVideoDecoder {
public:
    SkVideoDecoder(GrContext*);
    ~SkVideoDecoder();

    void reset();

    bool loadFile(const char filename[]);
    SkISize dimensions() const;

    sk_sp<SkImage> nextImage();

private:
    GrContext*          fGr = nullptr;  // not owned by us

    AVFormatContext*    fFormatCtx = nullptr;
    AVCodecContext*     fDecoderCtx = nullptr;
    int                 fStreamIndex = -1;  // fFormatCtx->stream[...]

    AVPacket            fPacket;
    AVFrame*            fFrame = nullptr;
};

#endif

