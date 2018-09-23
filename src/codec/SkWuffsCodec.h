/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkWuffsCodec_DEFINED
#define SkWuffsCodec_DEFINED

#include "SkCodec.h"
#include "SkFrameHolder.h"
#include "wuffs-v0.2.h"

#define SK_WUFFS_CODEC_BUFFER_SIZE 4096

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

class SkWuffsCodec;

// SkWuffsFrameHolder is a trivial indirector that forwards its calls onto a
// SkWuffsCodec. It is a separate class as SkWuffsCodec would otherwise
// inherit from both SkCodec and SkFrameHolder, and Skia style discourages
// multiple inheritance (e.g. with its "typedef Foo INHERITED" convention).
class SkWuffsFrameHolder final : public SkFrameHolder {
public:
    SkWuffsFrameHolder() : INHERITED() {}

protected:
    // SkFrameHolder overrides.
    const SkFrame* onGetFrame(int i) const override;

private:
    void init(SkWuffsCodec* codec, int width, int height);

    SkWuffsCodec* fCodec;

    friend class SkWuffsCodec;
    typedef SkFrameHolder INHERITED;
};

class SkWuffsCodec final : public SkCodec {
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
    SkWuffsCodec(SkEncodedInfo&& encodedInfo,
                 std::unique_ptr<SkStream> stream,
                 std::unique_ptr<wuffs_gif__decoder, decltype(&free)> dec,
                 std::unique_ptr<uint8_t, decltype(&free)> workbuf_ptr,
                 size_t workbuf_len,
                 std::unique_ptr<uint8_t, decltype(&free)> pixbuf_ptr,
                 wuffs_base__image_config imgcfg,
                 wuffs_base__pixel_buffer pixbuf,
                 wuffs_base__io_buffer iobuf);

    const SkWuffsFrame* frame(int i) const;
    void readFrames();
    Result seekFrame(int frameIndex);
    uint64_t numFullyReceived() const;

    Result resetDecoder();
    wuffs_base__status decodeFrameConfig();
    wuffs_base__status decodeFrame();

    SkWuffsFrameHolder fFrameHolder;
    std::unique_ptr<SkStream> fStream;
    std::unique_ptr<wuffs_gif__decoder, decltype(&free)> fDecoder;
    std::unique_ptr<uint8_t, decltype(&free)> fWorkbufPtr;
    size_t fWorkbufLen;
    std::unique_ptr<uint8_t, decltype(&free)> fPixbufPtr;

    uint64_t fFirstFrameIOPosition;
    uint32_t fNumLoops;
    wuffs_base__frame_config fFrameConfig;
    wuffs_base__pixel_buffer fPixelBuffer;
    wuffs_base__io_buffer fIOBuffer;

    // Incremental decoding state.
    SkColorType fIncrDecColorType;
    uint8_t* fIncrDecDst;
    bool fIncrDecHaveFrameConfig;
    size_t fIncrDecRowBytes;

    std::vector<SkWuffsFrame> fFrames;
    bool fFramesComplete;

    bool fNeedResetDecoder;

    // Updated during seekFrame.
    uint64_t fPreviousBestNumFullyReceived;

    uint8_t fBuffer[SK_WUFFS_CODEC_BUFFER_SIZE];

    friend class SkWuffsFrameHolder;
    friend std::unique_ptr<SkCodec> SkWuffsCodec_MakeFromStream(std::unique_ptr<SkStream>,
                                                                SkCodec::Result*);
    typedef SkCodec INHERITED;
};

#endif  // SkWuffsCodec_DEFINED
