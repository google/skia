/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCodec_wuffs_gif_DEFINED
#define SkCodec_wuffs_gif_DEFINED

#include "SkCodec.h"
#include "SkFrameHolder.h"
#include "wuffs-v0.2.h"

#define SK_WUFFS_GIF_CODEC_BUFFER_SIZE 4096

class SkWuffsFrame final : public SkFrame {
public:
    SkWuffsFrame(wuffs_base__frame_config* fc);

    SkCodec::FrameInfo frameInfo(bool fullyReceived) const;
    uint64_t ioPosition() const;

protected:
    // SkFrame overrides.
    SkEncodedInfo::Alpha onReportedAlpha() const override;

private:
    uint64_t fIOPosition;
    SkEncodedInfo::Alpha fReportedAlpha;

    typedef SkFrame INHERITED;
};

class SkWuffsGifCodec;

// SkWuffsGifFrameHolder is a trivial indirector that forwards its calls onto a
// SkWuffsGifCodec. It is a separate class as SkWuffsGifCodec would otherwise
// inherit from both SkCodec and SkFrameHolder, and Skia style discourages
// multiple inheritance (e.g. with its "typedef Foo INHERITED" convention).
class SkWuffsGifFrameHolder final : public SkFrameHolder {
public:
    SkWuffsGifFrameHolder() : INHERITED() {}

protected:
    // SkFrameHolder overrides.
    const SkFrame* onGetFrame(int i) const override;

private:
    void init(SkWuffsGifCodec* codec, int width, int height);

    SkWuffsGifCodec* fCodec;

    friend class SkWuffsGifCodec;
    typedef SkFrameHolder INHERITED;
};

class SkWuffsGifCodec final : public SkCodec {
public:
    static bool IsGif(const void*, size_t);

    static std::unique_ptr<SkCodec> MakeFromStream(std::unique_ptr<SkStream>, Result*);

protected:
    // SkCodec overrides.
    SkEncodedImageFormat onGetEncodedFormat() const override;
    Result onGetPixels(const SkImageInfo&, void*, size_t, const Options&, int*) override;
    const SkFrameHolder* getFrameHolder() const override;
    Result onStartIncrementalDecode(const SkImageInfo& dstInfo,
                                    void* dst,
                                    size_t rowBytes,
                                    const SkCodec::Options&) override;
    Result onIncrementalDecode(int* rowsDecoded) override;
    int onGetFrameCount() override;
    bool onGetFrameInfo(int, FrameInfo*) const override;
    int onGetRepetitionCount() override;

private:
    SkWuffsGifCodec(SkEncodedInfo&& encodedInfo,
                    std::unique_ptr<SkStream> stream,
                    std::unique_ptr<wuffs_gif__decoder> dec,
                    std::unique_ptr<uint8_t[]> workbuf_ptr,
                    size_t workbuf_len,
                    std::unique_ptr<uint8_t[]> pixbuf_ptr,
                    wuffs_base__image_config imgcfg,
                    wuffs_base__pixel_buffer pixbuf,
                    wuffs_base__io_buffer iobuf);

    const SkWuffsFrame* frame(int i) const;
    void readFrames();

    Result resetDecoder();
    Result seekByte(uint64_t byteIndex);
    Result seekFrame(int frameIndex);

    uint64_t numFullyReceived() const;

    wuffs_base__status decodeFrameConfig(wuffs_base__frame_config* fc);
    wuffs_base__status decodeFrame();

    SkWuffsGifFrameHolder fFrameHolder;
    std::unique_ptr<SkStream> fStream;
    std::unique_ptr<wuffs_gif__decoder> fDecoder;
    std::unique_ptr<uint8_t[]> fWorkbufPtr;
    size_t fWorkbufLen;
    std::unique_ptr<uint8_t[]> fPixbufPtr;

    uint64_t fFirstFrameIOPosition;
    uint32_t fNumLoops;
    wuffs_base__pixel_buffer fPixelBuffer;
    wuffs_base__io_buffer fIOBuffer;

    // Incremental decoding state.
    SkColorType fIncrDecColorType;
    uint8_t* fIncrDecDst;
    wuffs_base__frame_config fIncrDecFrameConfig;
    int fIncrDecFrameIndex;
    size_t fIncrDecRowBytes;
    int fIncrDecStage;

    std::vector<SkWuffsFrame> fFrames;
    bool fFramesComplete;

    bool fNeedResetDecoder;

    // Updated during seekFrame.
    uint64_t fPreviousBestNumFullyReceived;

    uint8_t fBuffer[SK_WUFFS_GIF_CODEC_BUFFER_SIZE];

    friend class SkWuffsGifFrameHolder;
    typedef SkCodec INHERITED;
};

#endif  // SkCodec_wuffs_gif_DEFINED
