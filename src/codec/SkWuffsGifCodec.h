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

class SkWuffsGifFrame final : public SkFrame {
   public:
    SkWuffsGifFrame(int id) : INHERITED(id) {}

    SkCodec::FrameInfo frameInfo() const;

   protected:
    SkEncodedInfo::Alpha onReportedAlpha() const override;

   private:
    typedef SkFrame INHERITED;
};

// TODO(nigeltao): is it in good Skia C++ style to use multiple inheritance?
// How does that work with the "typedef foo INHERITED" convention?
class SkWuffsGifCodec final : public SkCodec, public SkFrameHolder {
   public:
    static bool IsGif(const void*, size_t);

    static std::unique_ptr<SkCodec> MakeFromStream(std::unique_ptr<SkStream>, Result*);

    ~SkWuffsGifCodec() override;

   protected:
    // SkCodec overrides.
    SkEncodedImageFormat onGetEncodedFormat() const override;
    bool onRewind() override;
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

    // SkFrameHolder overrides.
    const SkFrame* onGetFrame(int i) const override;

   private:
    SkWuffsGifCodec(const SkEncodedInfo& encodedInfo,
                    const SkImageInfo& imageInfo,
                    std::unique_ptr<SkStream>,
                    uint8_t* pixbuf_ptr,
                    wuffs_gif__decoder* dec,
                    wuffs_base__image_config imgcfg,
                    wuffs_base__pixel_buffer pixbuf,
                    wuffs_base__io_buffer iobuf);

    SkWuffsGifFrame* frame(int i) const;

    wuffs_base__status decodeFrameConfig(wuffs_base__frame_config* fc);
    wuffs_base__status decodeFrame();

    uint8_t* fPixbufPtr;

    // TODO(nigeltao): ideally, we'd like to drop the * on the next line of
    // code, to avoid the indirection, and to avoid having to fiddle with malloc
    // and free calls. But the way Skia works is that the SkCodec constructor
    // has to be called *after* decoding the width and height, so we have to
    // allocate the wuffs_gif__decoder (in order to determine that width and
    // height) *before* we construct the SkWuffsGifCodec (as SkWuffsGifCodec
    // inherits from SkCodec), so we have to be indirect here and pass the
    // pointer via the constructor.
    //
    // Note that (sizeof wuffs_gif__decoder) is currently around 22904 bytes
    // (this can change over time, up until Wuffs hits version 1.0 ABI
    // stability), as Wuffs 'allocates' all the memory it will need up-front,
    // inside the wuffs_gif__decoder struct.
    //
    // TODO(nigeltao): use a C++ smart pointer type, such as std::unique_ptr?
    wuffs_gif__decoder* fDecoder;

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

    typedef SkCodec INHERITED;
};

#endif  // SkCodec_wuffs_gif_DEFINED
