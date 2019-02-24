/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkWuffsCodec.h"

#include "../private/SkMalloc.h"
#include "SkFrameHolder.h"
#include "SkSampler.h"
#include "SkSwizzler.h"
#include "SkUtils.h"

// Wuffs ships as a "single file C library" or "header file library" as per
// https://github.com/nothings/stb/blob/master/docs/stb_howto.txt
//
// As we have not #define'd WUFFS_IMPLEMENTATION, the #include here is
// including a header file, even though that file name ends in ".c".
#include "wuffs-v0.2.c"
#if WUFFS_VERSION_BUILD_METADATA_COMMIT_COUNT < 1640
#error "Wuffs version is too old. Upgrade to the latest version."
#endif

#define SK_WUFFS_CODEC_BUFFER_SIZE 4096

static bool fill_buffer(wuffs_base__io_buffer* b, SkStream* s) {
    b->compact();
    size_t num_read = s->read(b->data.ptr + b->meta.wi, b->data.len - b->meta.wi);
    b->meta.wi += num_read;
    b->meta.closed = s->isAtEnd();
    return num_read > 0;
}

static bool seek_buffer(wuffs_base__io_buffer* b, SkStream* s, uint64_t pos) {
    // Try to re-position the io_buffer's meta.ri read-index first, which is
    // cheaper than seeking in the backing SkStream.
    if ((pos >= b->meta.pos) && (pos - b->meta.pos <= b->meta.wi)) {
        b->meta.ri = pos - b->meta.pos;
        return true;
    }
    // Seek in the backing SkStream.
    if ((pos > SIZE_MAX) || (!s->seek(pos))) {
        return false;
    }
    b->meta.wi = 0;
    b->meta.ri = 0;
    b->meta.pos = pos;
    b->meta.closed = false;
    return true;
}

static SkEncodedInfo::Alpha wuffs_blend_to_skia_alpha(wuffs_base__animation_blend w) {
    return (w == WUFFS_BASE__ANIMATION_BLEND__OPAQUE) ? SkEncodedInfo::kOpaque_Alpha
                                                      : SkEncodedInfo::kUnpremul_Alpha;
}

static SkCodecAnimation::Blend wuffs_blend_to_skia_blend(wuffs_base__animation_blend w) {
    return (w == WUFFS_BASE__ANIMATION_BLEND__SRC) ? SkCodecAnimation::Blend::kBG
                                                   : SkCodecAnimation::Blend::kPriorFrame;
}

static SkCodecAnimation::DisposalMethod wuffs_disposal_to_skia_disposal(
    wuffs_base__animation_disposal w) {
    switch (w) {
        case WUFFS_BASE__ANIMATION_DISPOSAL__RESTORE_BACKGROUND:
            return SkCodecAnimation::DisposalMethod::kRestoreBGColor;
        case WUFFS_BASE__ANIMATION_DISPOSAL__RESTORE_PREVIOUS:
            return SkCodecAnimation::DisposalMethod::kRestorePrevious;
        default:
            return SkCodecAnimation::DisposalMethod::kKeep;
    }
}

// -------------------------------- Class definitions

class SkWuffsCodec;

class SkWuffsFrame final : public SkFrame {
public:
    SkWuffsFrame(wuffs_base__frame_config* fc);

    SkCodec::FrameInfo frameInfo(bool fullyReceived) const;
    uint64_t           ioPosition() const;

    // SkFrame overrides.
    SkEncodedInfo::Alpha onReportedAlpha() const override;

private:
    uint64_t             fIOPosition;
    SkEncodedInfo::Alpha fReportedAlpha;

    typedef SkFrame INHERITED;
};

// SkWuffsFrameHolder is a trivial indirector that forwards its calls onto a
// SkWuffsCodec. It is a separate class as SkWuffsCodec would otherwise
// inherit from both SkCodec and SkFrameHolder, and Skia style discourages
// multiple inheritance (e.g. with its "typedef Foo INHERITED" convention).
class SkWuffsFrameHolder final : public SkFrameHolder {
public:
    SkWuffsFrameHolder() : INHERITED() {}

    void init(SkWuffsCodec* codec, int width, int height);

    // SkFrameHolder overrides.
    const SkFrame* onGetFrame(int i) const override;

private:
    const SkWuffsCodec* fCodec;

    typedef SkFrameHolder INHERITED;
};

// SkWuffsSpySampler is a placeholder SkSampler implementation. The Skia API
// expects to manipulate the codec's sampler (i.e. call setSampleX and
// setSampleY) in between the startIncrementalDecode (SID) and
// incrementalDecode (ID) calls. But creating the SkSwizzler (the real sampler)
// requires knowing the destination buffer's dimensions, i.e. the animation
// frame's width and height. That width and height are decoded in ID, not SID.
//
// To break that circle, the SkWuffsSpySampler always exists, so its methods
// can be called between SID and ID. It doesn't actually do any sampling, it
// merely records the arguments given to setSampleX (explicitly) and setSampleY
// (implicitly, via the superclass' implementation). Inside ID, those recorded
// arguments are forwarded on to the SkSwizzler (the real sampler) when that
// SkSwizzler is created, after the frame width and height are known.
//
// Roughly speaking, the SkWuffsSpySampler is an eager proxy for the lazily
// constructed real sampler. But that laziness is out of necessity.
//
// The "Spy" name is because it records its arguments. See
// https://martinfowler.com/articles/mocksArentStubs.html#TheDifferenceBetweenMocksAndStubs
class SkWuffsSpySampler final : public SkSampler {
public:
    SkWuffsSpySampler(int imageWidth)
        : INHERITED(), fFillWidth(0), fImageWidth(imageWidth), fSampleX(1) {}

    void reset();
    int  sampleX() const;

    int fFillWidth;

private:
    // SkSampler overrides.
    int fillWidth() const override;
    int onSetSampleX(int sampleX) override;

    const int fImageWidth;

    int fSampleX;

    typedef SkSampler INHERITED;
};

class SkWuffsCodec final : public SkCodec {
public:
    SkWuffsCodec(SkEncodedInfo&&                                         encodedInfo,
                 std::unique_ptr<SkStream>                               stream,
                 std::unique_ptr<wuffs_gif__decoder, decltype(&sk_free)> dec,
                 std::unique_ptr<uint8_t, decltype(&sk_free)>            pixbuf_ptr,
                 std::unique_ptr<uint8_t, decltype(&sk_free)>            workbuf_ptr,
                 size_t                                                  workbuf_len,
                 wuffs_base__image_config                                imgcfg,
                 wuffs_base__pixel_buffer                                pixbuf,
                 wuffs_base__io_buffer                                   iobuf);

    const SkWuffsFrame* frame(int i) const;

private:
    // SkCodec overrides.
    SkEncodedImageFormat onGetEncodedFormat() const override;
    Result onGetPixels(const SkImageInfo&, void*, size_t, const Options&, int*) override;
    const SkFrameHolder* getFrameHolder() const override;
    Result               onStartIncrementalDecode(const SkImageInfo&      dstInfo,
                                                  void*                   dst,
                                                  size_t                  rowBytes,
                                                  const SkCodec::Options& options) override;
    Result               onIncrementalDecode(int* rowsDecoded) override;
    int                  onGetFrameCount() override;
    bool                 onGetFrameInfo(int, FrameInfo*) const override;
    int                  onGetRepetitionCount() override;
    SkSampler*           getSampler(bool createIfNecessary) override;
    bool                 conversionSupported(const SkImageInfo& dst, bool, bool) override;

    void   readFrames();
    Result seekFrame(int frameIndex);

    Result      resetDecoder();
    const char* decodeFrameConfig();
    const char* decodeFrame();
    void        updateNumFullyReceivedFrames();

    SkWuffsSpySampler                                       fSpySampler;
    SkWuffsFrameHolder                                      fFrameHolder;
    std::unique_ptr<SkStream>                               fStream;
    std::unique_ptr<wuffs_gif__decoder, decltype(&sk_free)> fDecoder;
    std::unique_ptr<uint8_t, decltype(&sk_free)>            fPixbufPtr;
    std::unique_ptr<uint8_t, decltype(&sk_free)>            fWorkbufPtr;
    size_t                                                  fWorkbufLen;

    const uint64_t           fFirstFrameIOPosition;
    wuffs_base__frame_config fFrameConfig;
    wuffs_base__pixel_buffer fPixelBuffer;
    wuffs_base__io_buffer    fIOBuffer;

    // Incremental decoding state.
    uint8_t* fIncrDecDst;
    size_t   fIncrDecRowBytes;

    std::unique_ptr<SkSwizzler> fSwizzler;
    int                         fScaledHeight;

    uint64_t                  fNumFullyReceivedFrames;
    std::vector<SkWuffsFrame> fFrames;
    bool                      fFramesComplete;

    // If calling an fDecoder method returns an incomplete status, then
    // fDecoder is suspended in a coroutine (i.e. waiting on I/O or halted on a
    // non-recoverable error). To keep its internal proof-of-safety invariants
    // consistent, there's only two things you can safely do with a suspended
    // Wuffs object: resume the coroutine, or reset all state (memset to zero
    // and start again).
    //
    // If fDecoderIsSuspended, and we aren't sure that we're going to resume
    // the coroutine, then we will need to call this->resetDecoder before
    // calling other fDecoder methods.
    bool fDecoderIsSuspended;

    uint8_t fBuffer[SK_WUFFS_CODEC_BUFFER_SIZE];

    typedef SkCodec INHERITED;
};

// -------------------------------- SkWuffsFrame implementation

SkWuffsFrame::SkWuffsFrame(wuffs_base__frame_config* fc)
    : INHERITED(fc->index()),
      fIOPosition(fc->io_position()),
      fReportedAlpha(wuffs_blend_to_skia_alpha(fc->blend())) {
    wuffs_base__rect_ie_u32 r = fc->bounds();
    this->setXYWH(r.min_incl_x, r.min_incl_y, r.width(), r.height());
    this->setDisposalMethod(wuffs_disposal_to_skia_disposal(fc->disposal()));
    this->setDuration(fc->duration() / WUFFS_BASE__FLICKS_PER_MILLISECOND);
    this->setBlend(wuffs_blend_to_skia_blend(fc->blend()));
}

SkCodec::FrameInfo SkWuffsFrame::frameInfo(bool fullyReceived) const {
    return ((SkCodec::FrameInfo){
        .fRequiredFrame = getRequiredFrame(),
        .fDuration = getDuration(),
        .fFullyReceived = fullyReceived,
        .fAlphaType = hasAlpha() ? kUnpremul_SkAlphaType : kOpaque_SkAlphaType,
        .fDisposalMethod = getDisposalMethod(),
    });
}

uint64_t SkWuffsFrame::ioPosition() const {
    return fIOPosition;
}

SkEncodedInfo::Alpha SkWuffsFrame::onReportedAlpha() const {
    return fReportedAlpha;
}

// -------------------------------- SkWuffsFrameHolder implementation

void SkWuffsFrameHolder::init(SkWuffsCodec* codec, int width, int height) {
    fCodec = codec;
    // Initialize SkFrameHolder's (the superclass) fields.
    fScreenWidth = width;
    fScreenHeight = height;
}

const SkFrame* SkWuffsFrameHolder::onGetFrame(int i) const {
    return fCodec->frame(i);
};

// -------------------------------- SkWuffsSpySampler implementation

void SkWuffsSpySampler::reset() {
    fFillWidth = 0;
    fSampleX = 1;
    this->setSampleY(1);
}

int SkWuffsSpySampler::sampleX() const {
    return fSampleX;
}

int SkWuffsSpySampler::fillWidth() const {
    return fFillWidth;
}

int SkWuffsSpySampler::onSetSampleX(int sampleX) {
    fSampleX = sampleX;
    return get_scaled_dimension(fImageWidth, sampleX);
}

// -------------------------------- SkWuffsCodec implementation

SkWuffsCodec::SkWuffsCodec(SkEncodedInfo&&                                         encodedInfo,
                           std::unique_ptr<SkStream>                               stream,
                           std::unique_ptr<wuffs_gif__decoder, decltype(&sk_free)> dec,
                           std::unique_ptr<uint8_t, decltype(&sk_free)>            pixbuf_ptr,
                           std::unique_ptr<uint8_t, decltype(&sk_free)>            workbuf_ptr,
                           size_t                                                  workbuf_len,
                           wuffs_base__image_config                                imgcfg,
                           wuffs_base__pixel_buffer                                pixbuf,
                           wuffs_base__io_buffer                                   iobuf)
    : INHERITED(std::move(encodedInfo),
                skcms_PixelFormat_RGBA_8888,
                // Pass a nullptr SkStream to the SkCodec constructor. We
                // manage the stream ourselves, as the default SkCodec behavior
                // is too trigger-happy on rewinding the stream.
                nullptr),
      fSpySampler(imgcfg.pixcfg.width()),
      fFrameHolder(),
      fStream(std::move(stream)),
      fDecoder(std::move(dec)),
      fPixbufPtr(std::move(pixbuf_ptr)),
      fWorkbufPtr(std::move(workbuf_ptr)),
      fWorkbufLen(workbuf_len),
      fFirstFrameIOPosition(imgcfg.first_frame_io_position()),
      fFrameConfig((wuffs_base__frame_config){}),
      fPixelBuffer(pixbuf),
      fIOBuffer((wuffs_base__io_buffer){}),
      fIncrDecDst(nullptr),
      fIncrDecRowBytes(0),
      fSwizzler(nullptr),
      fScaledHeight(0),
      fNumFullyReceivedFrames(0),
      fFramesComplete(false),
      fDecoderIsSuspended(false) {
    fFrameHolder.init(this, imgcfg.pixcfg.width(), imgcfg.pixcfg.height());

    // Initialize fIOBuffer's fields, copying any outstanding data from iobuf to
    // fIOBuffer, as iobuf's backing array may not be valid for the lifetime of
    // this SkWuffsCodec object, but fIOBuffer's backing array (fBuffer) is.
    SkASSERT(iobuf.data.len == SK_WUFFS_CODEC_BUFFER_SIZE);
    memmove(fBuffer, iobuf.data.ptr, iobuf.meta.wi);
    fIOBuffer = ((wuffs_base__io_buffer){
        .data = ((wuffs_base__slice_u8){
            .ptr = fBuffer,
            .len = SK_WUFFS_CODEC_BUFFER_SIZE,
        }),
        .meta = iobuf.meta,
    });
}

const SkWuffsFrame* SkWuffsCodec::frame(int i) const {
    if ((0 <= i) && (static_cast<size_t>(i) < fFrames.size())) {
        return &fFrames[i];
    }
    return nullptr;
}

SkEncodedImageFormat SkWuffsCodec::onGetEncodedFormat() const {
    return SkEncodedImageFormat::kGIF;
}

SkCodec::Result SkWuffsCodec::onGetPixels(const SkImageInfo& dstInfo,
                                          void*              dst,
                                          size_t             rowBytes,
                                          const Options&     options,
                                          int*               rowsDecoded) {
    SkCodec::Result result = this->onStartIncrementalDecode(dstInfo, dst, rowBytes, options);
    if (result != kSuccess) {
        return result;
    }
    return this->onIncrementalDecode(rowsDecoded);
}

const SkFrameHolder* SkWuffsCodec::getFrameHolder() const {
    return &fFrameHolder;
}

SkCodec::Result SkWuffsCodec::onStartIncrementalDecode(const SkImageInfo&      dstInfo,
                                                       void*                   dst,
                                                       size_t                  rowBytes,
                                                       const SkCodec::Options& options) {
    if (options.fSubset) {
        return SkCodec::kUnimplemented;
    }
    SkCodec::Result result = this->seekFrame(options.fFrameIndex);
    if (result != SkCodec::kSuccess) {
        return result;
    }

    fSpySampler.reset();
    fSwizzler = nullptr;
    fScaledHeight = 0;

    const char* status = this->decodeFrameConfig();
    if (status == wuffs_base__suspension__short_read) {
        return SkCodec::kIncompleteInput;
    } else if (status != nullptr) {
        SkCodecPrintf("decodeFrameConfig: %s", status);
        return SkCodec::kErrorInInput;
    }

    uint32_t src_bits_per_pixel =
        wuffs_base__pixel_format__bits_per_pixel(fPixelBuffer.pixcfg.pixel_format());
    if ((src_bits_per_pixel == 0) || (src_bits_per_pixel % 8 != 0)) {
        return SkCodec::kInternalError;
    }
    size_t src_bytes_per_pixel = src_bits_per_pixel / 8;

    // Zero-initialize Wuffs' buffer covering the frame rect.
    wuffs_base__rect_ie_u32 frame_rect = fFrameConfig.bounds();
    wuffs_base__table_u8    pixels = fPixelBuffer.plane(0);
    for (uint32_t y = frame_rect.min_incl_y; y < frame_rect.max_excl_y; y++) {
        sk_bzero(pixels.ptr + (y * pixels.stride) + (frame_rect.min_incl_x * src_bytes_per_pixel),
                 frame_rect.width() * src_bytes_per_pixel);
    }

    fIncrDecDst = static_cast<uint8_t*>(dst);
    fIncrDecRowBytes = rowBytes;
    return SkCodec::kSuccess;
}

static bool independent_frame(SkCodec* codec, int frameIndex) {
    if (frameIndex == 0) {
        return true;
    }

    SkCodec::FrameInfo frameInfo;
    SkAssertResult(codec->getFrameInfo(frameIndex, &frameInfo));
    return frameInfo.fRequiredFrame == SkCodec::kNoFrame;
}

static void blend(uint32_t* dst, const uint32_t* src, int width) {
    while (width --> 0) {
        if (*src != 0) {
            *dst = *src;
        }
        src++;
        dst++;
    }
}

SkCodec::Result SkWuffsCodec::onIncrementalDecode(int* rowsDecoded) {
    if (!fIncrDecDst) {
        return SkCodec::kInternalError;
    }

    SkCodec::Result result = SkCodec::kSuccess;
    const char*     status = this->decodeFrame();
    const bool      independent = independent_frame(this, options().fFrameIndex);
    if (status != nullptr) {
        if (status == wuffs_base__suspension__short_read) {
            result = SkCodec::kIncompleteInput;
        } else {
            SkCodecPrintf("decodeFrame: %s", status);
            result = SkCodec::kErrorInInput;
        }

        if (!independent) {
            // For a dependent frame, we cannot blend the partial result, since
            // that will overwrite the contribution from prior frames.
            return result;
        }
    }

    uint32_t src_bits_per_pixel =
        wuffs_base__pixel_format__bits_per_pixel(fPixelBuffer.pixcfg.pixel_format());
    if ((src_bits_per_pixel == 0) || (src_bits_per_pixel % 8 != 0)) {
        return SkCodec::kInternalError;
    }
    size_t src_bytes_per_pixel = src_bits_per_pixel / 8;

    wuffs_base__rect_ie_u32 frame_rect = fFrameConfig.bounds();
    if (!fSwizzler) {
        auto bounds = SkIRect::MakeLTRB(frame_rect.min_incl_x, frame_rect.min_incl_y,
                                        frame_rect.max_excl_x, frame_rect.max_excl_y);
        fSwizzler =
            SkSwizzler::Make(this->getEncodedInfo(), nullptr, dstInfo(), this->options(), &bounds);
        fSwizzler->setSampleX(fSpySampler.sampleX());
        fSwizzler->setSampleY(fSpySampler.sampleY());
        fScaledHeight = get_scaled_dimension(dstInfo().height(), fSpySampler.sampleY());

        if (frame_rect.width() > (SIZE_MAX / src_bytes_per_pixel)) {
            return SkCodec::kInternalError;
        }

        // If the frame rect does not fill the output, ensure that those pixels are not
        // left uninitialized.
        if (independent && (bounds != this->bounds() || result != kSuccess)) {
            auto fillInfo = dstInfo().makeWH(fSwizzler->fillWidth(), fScaledHeight);
            SkSampler::Fill(fillInfo, fIncrDecDst, fIncrDecRowBytes, options().fZeroInitialized);
        }
    }
    if (fScaledHeight == 0) {
        return SkCodec::kInternalError;
    }

    // The semantics of *rowsDecoded is: say you have a 10 pixel high image
    // (both the frame and the image). If you only decoded the first 3 rows,
    // set this to 3, and then SkCodec (or the caller of incrementalDecode)
    // would zero-initialize the remaining 7 (unless the memory was already
    // zero-initialized).
    //
    // Now let's say that the image is still 10 pixels high, but the frame is
    // from row 5 to 9. If you only decoded 3 rows, but you initialized the
    // first 5, you could return 8, and the caller would zero-initialize the
    // final 2. For GIF (where a frame can be smaller than the image and can be
    // interlaced), we just zero-initialize all 10 rows ahead of time and
    // return the height of the image, so the caller knows it doesn't need to
    // do anything.
    //
    // Similarly, if the output is scaled, we zero-initialized all
    // |fScaledHeight| rows (the scaled image height), so we inform the caller
    // that it doesn't need to do anything.
    if (rowsDecoded) {
        *rowsDecoded = fScaledHeight;
    }

    // If the frame's dirty rect is empty, no need to swizzle.
    wuffs_base__rect_ie_u32 dirty_rect = fDecoder->frame_dirty_rect();
    if (!dirty_rect.is_empty()) {
        std::unique_ptr<uint8_t[]> tmpBuffer;
        if (!independent) {
            tmpBuffer.reset(new uint8_t[dstInfo().minRowBytes()]);
        }
        wuffs_base__table_u8 pixels = fPixelBuffer.plane(0);
        const int            sampleY = fSwizzler->sampleY();
        for (uint32_t y = dirty_rect.min_incl_y; y < dirty_rect.max_excl_y; y++) {
            int dstY = y;
            if (sampleY != 1) {
                if (!fSwizzler->rowNeeded(y)) {
                    continue;
                }
                dstY /= sampleY;
                if (dstY >= fScaledHeight) {
                    break;
                }
            }

            // We don't adjust d by (frame_rect.min_incl_x * dst_bpp) as we
            // have already accounted for that in swizzleRect, above.
            uint8_t* d = fIncrDecDst + (dstY * fIncrDecRowBytes);

            // The Wuffs model is that the dst buffer is the image, not the frame.
            // The expectation is that you allocate the buffer once, but re-use it
            // for the N frames, regardless of each frame's top-left co-ordinate.
            //
            // To get from the start (in the X-direction) of the image to the start
            // of the frame, we adjust s by (frame_rect.min_incl_x *
            // src_bytes_per_pixel).
            //
            // We adjust (in the X-direction) by the frame rect, not the dirty
            // rect, because the swizzler (which operates on rows) was
            // configured with the frame rect's X range.
            uint8_t* s =
                pixels.ptr + (y * pixels.stride) + (frame_rect.min_incl_x * src_bytes_per_pixel);
            if (independent) {
                fSwizzler->swizzle(d, s);
            } else {
                SkASSERT(tmpBuffer.get());
                fSwizzler->swizzle(tmpBuffer.get(), s);
                d = SkTAddOffset<uint8_t>(d, fSwizzler->swizzleOffsetBytes());
                const auto* swizzled = SkTAddOffset<uint32_t>(tmpBuffer.get(),
                                                              fSwizzler->swizzleOffsetBytes());
                blend(reinterpret_cast<uint32_t*>(d), swizzled, fSwizzler->swizzleWidth());
            }
        }
    }

    if (result == SkCodec::kSuccess) {
        fSpySampler.reset();
        fIncrDecDst = nullptr;
        fIncrDecRowBytes = 0;
        fSwizzler = nullptr;
    } else {
        // Make fSpySampler return whatever fSwizzler would have for fillWidth.
        fSpySampler.fFillWidth = fSwizzler->fillWidth();
    }
    return result;
}

int SkWuffsCodec::onGetFrameCount() {
    if (!fFramesComplete) {
        this->readFrames();
        this->updateNumFullyReceivedFrames();
    }
    return fFrames.size();
}

bool SkWuffsCodec::onGetFrameInfo(int i, SkCodec::FrameInfo* frameInfo) const {
    const SkWuffsFrame* f = this->frame(i);
    if (!f) {
        return false;
    }
    if (frameInfo) {
        *frameInfo = f->frameInfo(static_cast<uint64_t>(i) < this->fNumFullyReceivedFrames);
    }
    return true;
}

int SkWuffsCodec::onGetRepetitionCount() {
    // Convert from Wuffs's loop count to Skia's repeat count. Wuffs' uint32_t
    // number is how many times to play the loop. Skia's int number is how many
    // times to play the loop *after the first play*. Wuffs and Skia use 0 and
    // kRepetitionCountInfinite respectively to mean loop forever.
    uint32_t n = fDecoder->num_animation_loops();
    if (n == 0) {
        return SkCodec::kRepetitionCountInfinite;
    }
    n--;
    return n < INT_MAX ? n : INT_MAX;
}

SkSampler* SkWuffsCodec::getSampler(bool createIfNecessary) {
    // fIncrDst being non-nullptr means that we are between an
    // onStartIncrementalDecode call and the matching final (successful)
    // onIncrementalDecode call.
    if (createIfNecessary || fIncrDecDst) {
        return &fSpySampler;
    }
    return nullptr;
}

bool SkWuffsCodec::conversionSupported(const SkImageInfo& dst, bool srcIsOpaque, bool needsColorXform) {
    if (!this->INHERITED::conversionSupported(dst, srcIsOpaque, needsColorXform)) {
        return false;
    }

    switch (dst.colorType()) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
            return true;
        default:
            // FIXME: Add skcms to support F16
            // FIXME: Add support for 565 on the first frame
            return false;
    }
}

void SkWuffsCodec::readFrames() {
    size_t n = fFrames.size();
    int    i = n ? n - 1 : 0;
    if (this->seekFrame(i) != SkCodec::kSuccess) {
        return;
    }

    // Iterate through the frames, converting from Wuffs'
    // wuffs_base__frame_config type to Skia's SkWuffsFrame type.
    for (; i < INT_MAX; i++) {
        const char* status = this->decodeFrameConfig();
        if (status == nullptr) {
            // No-op.
        } else if (status == wuffs_base__warning__end_of_data) {
            break;
        } else {
            return;
        }

        if (static_cast<size_t>(i) < fFrames.size()) {
            continue;
        }
        fFrames.emplace_back(&fFrameConfig);
        SkWuffsFrame* f = &fFrames[fFrames.size() - 1];
        fFrameHolder.setAlphaAndRequiredFrame(f);
    }

    fFramesComplete = true;
}

SkCodec::Result SkWuffsCodec::seekFrame(int frameIndex) {
    if (fDecoderIsSuspended) {
        SkCodec::Result res = this->resetDecoder();
        if (res != SkCodec::kSuccess) {
            return res;
        }
    }

    uint64_t pos = 0;
    if (frameIndex < 0) {
        return SkCodec::kInternalError;
    } else if (frameIndex == 0) {
        pos = fFirstFrameIOPosition;
    } else if (static_cast<size_t>(frameIndex) < fFrames.size()) {
        pos = fFrames[frameIndex].ioPosition();
    } else {
        return SkCodec::kInternalError;
    }

    if (!seek_buffer(&fIOBuffer, fStream.get(), pos)) {
        return SkCodec::kInternalError;
    }
    const char* status = fDecoder->restart_frame(frameIndex, fIOBuffer.reader_io_position());
    if (status != nullptr) {
        return SkCodec::kInternalError;
    }
    return SkCodec::kSuccess;
}

// An overview of the Wuffs decoding API:
//
// An animated image (such as GIF) has an image header and then N frames. The
// image header gives e.g. the overall image's width and height. Each frame
// consists of a frame header (e.g. frame rectangle bounds, display duration)
// and a payload (the pixels).
//
// In Wuffs terminology, there is one image config and then N pairs of
// (frame_config, frame). To decode everything (without knowing N in advance)
// sequentially:
//  - call wuffs_gif__decoder::decode_image_config
//  - while (true) {
//  -   call wuffs_gif__decoder::decode_frame_config
//  -   if that returned wuffs_base__warning__end_of_data, break
//  -   call wuffs_gif__decoder::decode_frame
//  - }
//
// The first argument to each decode_foo method is the destination struct to
// store the decoded information.
//
// For random (instead of sequential) access to an image's frames, call
// wuffs_gif__decoder::restart_frame to prepare to decode the i'th frame.
// Essentially, it restores the state to be at the top of the while loop above.
// The wuffs_base__io_buffer's reader position will also need to be set at the
// right point in the source data stream. The position for the i'th frame is
// calculated by the i'th decode_frame_config call. You can only call
// restart_frame after decode_image_config is called, explicitly or implicitly
// (see below), as decoding a single frame might require for-all-frames
// information like the overall image dimensions and the global palette.
//
// All of those decode_xxx calls are optional. For example, if
// decode_image_config is not called, then the first decode_frame_config call
// will implicitly parse and verify the image header, before parsing the first
// frame's header. Similarly, you can call only decode_frame N times, without
// calling decode_image_config or decode_frame_config, if you already know
// metadata like N and each frame's rectangle bounds by some other means (e.g.
// this is a first party, statically known image).
//
// Specifically, starting with an unknown (but re-windable) GIF image, if you
// want to just find N (i.e. count the number of frames), you can loop calling
// only the decode_frame_config method and avoid calling the more expensive
// decode_frame method. In terms of the underlying GIF image format, this will
// skip over the LZW-encoded pixel data, avoiding the costly LZW decompression.
//
// Those decode_xxx methods are also suspendible. They will return early (with
// a status code that is_suspendible and therefore isn't is_complete) if there
// isn't enough source data to complete the operation: an incremental decode.
// Calling decode_xxx again with additional source data will resume the
// previous operation, instead of starting a new operation. Calling decode_yyy
// whilst decode_xxx is suspended will result in an error.
//
// Once an error is encountered, whether from invalid source data or from a
// programming error such as calling decode_yyy while suspended in decode_xxx,
// all subsequent calls will be no-ops that return an error. To reset the
// decoder into something that does productive work, memset the entire struct
// to zero, check the Wuffs version and then, in order to be able to call
// restart_frame, call decode_image_config. The io_buffer and its associated
// stream will also need to be rewound.

static SkCodec::Result reset_and_decode_image_config(wuffs_gif__decoder*       decoder,
                                                     wuffs_base__image_config* imgcfg,
                                                     wuffs_base__io_buffer*    b,
                                                     SkStream*                 s) {
    memset(decoder, 0, sizeof__wuffs_gif__decoder());
    const char* status = decoder->check_wuffs_version(sizeof__wuffs_gif__decoder(), WUFFS_VERSION);
    if (status != nullptr) {
        SkCodecPrintf("check_wuffs_version: %s", status);
        return SkCodec::kInternalError;
    }
    while (true) {
        status = decoder->decode_image_config(imgcfg, b->reader());
        if (status == nullptr) {
            break;
        } else if (status != wuffs_base__suspension__short_read) {
            SkCodecPrintf("decode_image_config: %s", status);
            return SkCodec::kErrorInInput;
        } else if (!fill_buffer(b, s)) {
            return SkCodec::kIncompleteInput;
        }
    }

    // A GIF image's natural color model is indexed color: 1 byte per pixel,
    // indexing a 256-element palette.
    //
    // For Skia, we override that to decode to 4 bytes per pixel, BGRA or RGBA.
    wuffs_base__pixel_format pixfmt = 0;
    switch (kN32_SkColorType) {
        case kBGRA_8888_SkColorType:
            pixfmt = WUFFS_BASE__PIXEL_FORMAT__BGRA_NONPREMUL;
            break;
        case kRGBA_8888_SkColorType:
            pixfmt = WUFFS_BASE__PIXEL_FORMAT__RGBA_NONPREMUL;
            break;
        default:
            return SkCodec::kInternalError;
    }
    imgcfg->pixcfg.set(pixfmt, WUFFS_BASE__PIXEL_SUBSAMPLING__NONE, imgcfg->pixcfg.width(),
                       imgcfg->pixcfg.height());

    return SkCodec::kSuccess;
}

SkCodec::Result SkWuffsCodec::resetDecoder() {
    if (!fStream->rewind()) {
        return SkCodec::kInternalError;
    }
    fIOBuffer.meta = ((wuffs_base__io_buffer_meta){});

    SkCodec::Result result =
        reset_and_decode_image_config(fDecoder.get(), nullptr, &fIOBuffer, fStream.get());
    if (result == SkCodec::kIncompleteInput) {
        return SkCodec::kInternalError;
    } else if (result != SkCodec::kSuccess) {
        return result;
    }

    fDecoderIsSuspended = false;
    return SkCodec::kSuccess;
}

const char* SkWuffsCodec::decodeFrameConfig() {
    while (true) {
        const char* status = fDecoder->decode_frame_config(&fFrameConfig, fIOBuffer.reader());
        if ((status == wuffs_base__suspension__short_read) &&
            fill_buffer(&fIOBuffer, fStream.get())) {
            continue;
        }
        fDecoderIsSuspended = !wuffs_base__status__is_complete(status);
        this->updateNumFullyReceivedFrames();
        return status;
    }
}

const char* SkWuffsCodec::decodeFrame() {
    while (true) {
        const char* status = fDecoder->decode_frame(&fPixelBuffer, fIOBuffer.reader(),
                                                    ((wuffs_base__slice_u8){
                                                        .ptr = fWorkbufPtr.get(),
                                                        .len = fWorkbufLen,
                                                    }),
                                                    NULL);
        if ((status == wuffs_base__suspension__short_read) &&
            fill_buffer(&fIOBuffer, fStream.get())) {
            continue;
        }
        fDecoderIsSuspended = !wuffs_base__status__is_complete(status);
        this->updateNumFullyReceivedFrames();
        return status;
    }
}

void SkWuffsCodec::updateNumFullyReceivedFrames() {
    // num_decoded_frames's return value, n, can change over time, both up and
    // down, as we seek back and forth in the underlying stream.
    // fNumFullyReceivedFrames is the highest n we've seen.
    uint64_t n = fDecoder->num_decoded_frames();
    if (fNumFullyReceivedFrames < n) {
        fNumFullyReceivedFrames = n;
    }
}

// -------------------------------- SkWuffsCodec.h functions

bool SkWuffsCodec_IsFormat(const void* buf, size_t bytesRead) {
    constexpr const char* gif_ptr = "GIF8";
    constexpr size_t      gif_len = 4;
    return (bytesRead >= gif_len) && (memcmp(buf, gif_ptr, gif_len) == 0);
}

std::unique_ptr<SkCodec> SkWuffsCodec_MakeFromStream(std::unique_ptr<SkStream> stream,
                                                     SkCodec::Result*          result) {
    uint8_t                  buffer[SK_WUFFS_CODEC_BUFFER_SIZE];
    wuffs_base__io_buffer    iobuf = ((wuffs_base__io_buffer){
        .data = ((wuffs_base__slice_u8){
            .ptr = buffer,
            .len = SK_WUFFS_CODEC_BUFFER_SIZE,
        }),
        .meta = ((wuffs_base__io_buffer_meta){}),
    });
    wuffs_base__image_config imgcfg = ((wuffs_base__image_config){});

    // Wuffs is primarily a C library, not a C++ one. Furthermore, outside of
    // the wuffs_base__etc types, the sizeof a file format specific type like
    // GIF's wuffs_gif__decoder can vary between Wuffs versions. If p is of
    // type wuffs_gif__decoder*, then the supported API treats p as a pointer
    // to an opaque type: a private implementation detail. The API is always
    // "set_foo(p, etc)" and not "p->foo = etc".
    //
    // See https://en.wikipedia.org/wiki/Opaque_pointer#C
    //
    // Thus, we don't use C++'s new operator (which requires knowing the sizeof
    // the struct at compile time). Instead, we use sk_malloc_canfail, with
    // sizeof__wuffs_gif__decoder returning the appropriate value for the
    // (statically or dynamically) linked version of the Wuffs library.
    //
    // As a C (not C++) library, none of the Wuffs types have constructors or
    // destructors.
    //
    // In RAII style, we can still use std::unique_ptr with these pointers, but
    // we pair the pointer with sk_free instead of C++'s delete.
    void* decoder_raw = sk_malloc_canfail(sizeof__wuffs_gif__decoder());
    if (!decoder_raw) {
        *result = SkCodec::kInternalError;
        return nullptr;
    }
    std::unique_ptr<wuffs_gif__decoder, decltype(&sk_free)> decoder(
        reinterpret_cast<wuffs_gif__decoder*>(decoder_raw), &sk_free);

    SkCodec::Result reset_result =
        reset_and_decode_image_config(decoder.get(), &imgcfg, &iobuf, stream.get());
    if (reset_result != SkCodec::kSuccess) {
        *result = reset_result;
        return nullptr;
    }

    uint32_t width = imgcfg.pixcfg.width();
    uint32_t height = imgcfg.pixcfg.height();
    if ((width == 0) || (width > INT_MAX) || (height == 0) || (height > INT_MAX)) {
        *result = SkCodec::kInvalidInput;
        return nullptr;
    }

    uint64_t workbuf_len = decoder->workbuf_len().max_incl;
    void*    workbuf_ptr_raw = nullptr;
    if (workbuf_len) {
        workbuf_ptr_raw = workbuf_len <= SIZE_MAX ? sk_malloc_canfail(workbuf_len) : nullptr;
        if (!workbuf_ptr_raw) {
            *result = SkCodec::kInternalError;
            return nullptr;
        }
    }
    std::unique_ptr<uint8_t, decltype(&sk_free)> workbuf_ptr(
        reinterpret_cast<uint8_t*>(workbuf_ptr_raw), &sk_free);

    uint64_t pixbuf_len = imgcfg.pixcfg.pixbuf_len();
    void*    pixbuf_ptr_raw = pixbuf_len <= SIZE_MAX ? sk_malloc_canfail(pixbuf_len) : nullptr;
    if (!pixbuf_ptr_raw) {
        *result = SkCodec::kInternalError;
        return nullptr;
    }
    std::unique_ptr<uint8_t, decltype(&sk_free)> pixbuf_ptr(
        reinterpret_cast<uint8_t*>(pixbuf_ptr_raw), &sk_free);
    wuffs_base__pixel_buffer pixbuf = ((wuffs_base__pixel_buffer){});

    const char* status = pixbuf.set_from_slice(&imgcfg.pixcfg, ((wuffs_base__slice_u8){
                                                                   .ptr = pixbuf_ptr.get(),
                                                                   .len = pixbuf_len,
                                                               }));
    if (status != nullptr) {
        SkCodecPrintf("set_from_slice: %s", status);
        *result = SkCodec::kInternalError;
        return nullptr;
    }

    SkEncodedInfo::Color color =
        (imgcfg.pixcfg.pixel_format() == WUFFS_BASE__PIXEL_FORMAT__BGRA_NONPREMUL)
            ? SkEncodedInfo::kBGRA_Color
            : SkEncodedInfo::kRGBA_Color;

    // In Skia's API, the alpha we calculate here and return is only for the
    // first frame.
    SkEncodedInfo::Alpha alpha = imgcfg.first_frame_is_opaque() ? SkEncodedInfo::kOpaque_Alpha
                                                                : SkEncodedInfo::kBinary_Alpha;

    SkEncodedInfo encodedInfo = SkEncodedInfo::Make(width, height, color, alpha, 8);

    *result = SkCodec::kSuccess;
    return std::unique_ptr<SkCodec>(new SkWuffsCodec(
        std::move(encodedInfo), std::move(stream), std::move(decoder), std::move(pixbuf_ptr),
        std::move(workbuf_ptr), workbuf_len, imgcfg, pixbuf, iobuf));
}
