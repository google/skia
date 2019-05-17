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
#include "libavformat/avio.h"
#include "libavutil/pixdesc.h"
}

class SkVideoDecoder {
public:
    SkVideoDecoder(GrContext* gr = nullptr);
    ~SkVideoDecoder();

    void reset();
    void setGrContext(GrContext* gr) { fGr = gr; }

    bool loadStream(std::unique_ptr<SkStream>);
    bool rewind();

    SkISize dimensions() const;
    double duration() const;    // in seconds

    // Returns each image in the video, or nullptr on eof
    sk_sp<SkImage> nextImage(double* timeStamp = nullptr);

private:
    sk_sp<SkImage> convertFrame(const AVFrame*);
    double computeTimeStamp(const AVFrame*) const;

    struct ConvertedColorSpace {
        AVColorPrimaries              fPrimaries;
        AVColorTransferCharacteristic fTransfer;
        // fCS is the converted skia form of the above enums
        sk_sp<SkColorSpace> fCS;

        // Init with illegal values, so our first compare will fail, forcing us to compute
        // the skcolorspace.
        ConvertedColorSpace();

        void update(AVColorPrimaries, AVColorTransferCharacteristic);
    };

    GrContext*          fGr = nullptr;  // not owned by us

    std::unique_ptr<SkStream>   fStream;

    AVIOContext*        fStreamCtx = nullptr;
    AVFormatContext*    fFormatCtx = nullptr;
    AVCodecContext*     fDecoderCtx = nullptr;
    int                 fStreamIndex = -1;  // fFormatCtx->stream[...]

    AVPacket            fPacket;
    AVFrame*            fFrame = nullptr;
    ConvertedColorSpace fCSCache;

    enum Mode {
        kProcessing_Mode,
        kDraining_Mode,
        kDone_Mode,
    };
    Mode    fMode = kDone_Mode;
};

#endif

