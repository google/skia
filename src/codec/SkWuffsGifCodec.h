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

class SkWuffsGifCodec;

class SkWuffsGifFrame final : public SkFrame {
public:
    SkWuffsGifFrame(int id) : INHERITED(id) {}

    SkCodec::FrameInfo frameInfo() const;

protected:
    // SkFrame overrides.
    SkEncodedInfo::Alpha onReportedAlpha() const override;

private:
    typedef SkFrame INHERITED;
};

// SkWuffsGifFrameHolder is a trivial indirector that forwards its calls onto a
// SkWuffsGifCodec. It is a separate class as SkWuffsGifCodec would otherwise
// inherit from both SkCodec and SkFrameHolder, and Skia style avoids multiple
// inheritance (e.g. with its "typedef Foo INHERITED" convention).
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
    SkWuffsGifCodec(const SkEncodedInfo& encodedInfo,
                    const SkImageInfo& imageInfo,
                    std::unique_ptr<SkStream> stream,
                    std::unique_ptr<wuffs_gif__decoder> dec,
                    std::unique_ptr<uint8_t[]> pixbuf_ptr,
                    wuffs_base__image_config imgcfg,
                    wuffs_base__pixel_buffer pixbuf,
                    wuffs_base__io_buffer iobuf);

    SkWuffsGifFrame* frame(int i) const;
    void readFrames();
    bool rewind();
    bool seek(int frameIndex);

    wuffs_base__status decodeFrameConfig(wuffs_base__frame_config* fc);
    wuffs_base__status decodeFrame();

    SkWuffsGifFrameHolder fFrameHolder;
    std::unique_ptr<SkStream> fStream;
    std::unique_ptr<wuffs_gif__decoder> fDecoder;
    std::unique_ptr<uint8_t[]> fPixbufPtr;

    uint32_t fNumLoops;
    wuffs_base__pixel_buffer fPixelBuffer;
    wuffs_base__io_buffer fIOBuffer;

    // Incremental decoding state.
    uint8_t* fIncrDecDst;
    int fIncrDecFrameIndex;
    size_t fIncrDecRowBytes;

    // TODO(nigeltao): instead of copying from the SkStream into this buffer,
    // can we access the underlying SkStream's buffer directly, if available
    // (e.g. for mmap'ed files)?
    uint8_t fBuffer[SK_WUFFS_GIF_CODEC_BUFFER_SIZE];

    // TODO(nigeltao): the std::unique_ptr layer-of-indirection is only needed
    // because the SkFrame base class is declared SkNoncopyable. I'm not sure
    // why that's necessary, as SkFrame's fields don't have any pointers, just
    // int-like things. Check with scroggo@ if I'm missing something.
    //
    // Ideally, it'd be a std::vector<SkWuffsGifFrame>.
    std::vector<std::unique_ptr<SkWuffsGifFrame>> fFrames;
    bool fFramesComplete;

    friend class SkWuffsGifFrameHolder;
    typedef SkCodec INHERITED;
};

#endif  // SkCodec_wuffs_gif_DEFINED
