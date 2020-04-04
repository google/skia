/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkWuffsCodec.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/private/SkMalloc.h"
#include "src/codec/SkFrameHolder.h"
#include "src/codec/SkSampler.h"
#include "src/codec/SkScalingCodec.h"
#include "src/core/SkDraw.h"
#include "src/core/SkRasterClip.h"
#include "src/core/SkUtils.h"

#include <limits.h>

// Wuffs ships as a "single file C library" or "header file library" as per
// https://github.com/nothings/stb/blob/master/docs/stb_howto.txt
//
// As we have not #define'd WUFFS_IMPLEMENTATION, the #include here is
// including a header file, even though that file name ends in ".c".
#if defined(WUFFS_IMPLEMENTATION)
#error "SkWuffsCodec should not #define WUFFS_IMPLEMENTATION"
#endif
#include "wuffs-v0.2.c"
#if WUFFS_VERSION_BUILD_METADATA_COMMIT_COUNT < 1942
#error "Wuffs version is too old. Upgrade to the latest version."
#endif

#define SK_WUFFS_CODEC_BUFFER_SIZE 4096

// Configuring a Skia build with
// SK_WUFFS_FAVORS_PERFORMANCE_OVER_ADDITIONAL_MEMORY_SAFETY can improve decode
// performance by some fixed amount (independent of the image size), which can
// be a noticeable proportional improvement if the input is relatively small.
//
// The Wuffs library is still memory-safe either way, in that there are no
// out-of-bounds reads or writes, and the library endeavours not to read
// uninitialized memory. There are just fewer compiler-enforced guarantees
// against reading uninitialized memory. For more detail, see
// https://github.com/google/wuffs/blob/master/doc/note/initialization.md#partial-zero-initialization
#if defined(SK_WUFFS_FAVORS_PERFORMANCE_OVER_ADDITIONAL_MEMORY_SAFETY)
#define SK_WUFFS_INITIALIZE_FLAGS WUFFS_INITIALIZE__LEAVE_INTERNAL_BUFFERS_UNINITIALIZED
#else
#define SK_WUFFS_INITIALIZE_FLAGS WUFFS_INITIALIZE__DEFAULT_OPTIONS
#endif

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

static SkAlphaType to_alpha_type(bool opaque) {
    return opaque ? kOpaque_SkAlphaType : kPremul_SkAlphaType;
}

static SkCodec::Result reset_and_decode_image_config(wuffs_gif__decoder*       decoder,
                                                     wuffs_base__image_config* imgcfg,
                                                     wuffs_base__io_buffer*    b,
                                                     SkStream*                 s) {
    // Calling decoder->initialize will memset most or all of it to zero,
    // depending on SK_WUFFS_INITIALIZE_FLAGS.
    const char* status =
        decoder->initialize(sizeof__wuffs_gif__decoder(), WUFFS_VERSION, SK_WUFFS_INITIALIZE_FLAGS);
    if (status != nullptr) {
        SkCodecPrintf("initialize: %s", status);
        return SkCodec::kInternalError;
    }
    while (true) {
        status = decoder->decode_image_config(imgcfg, b);
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
    if (imgcfg) {
        imgcfg->pixcfg.set(pixfmt, WUFFS_BASE__PIXEL_SUBSAMPLING__NONE, imgcfg->pixcfg.width(),
                           imgcfg->pixcfg.height());
    }

    return SkCodec::kSuccess;
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

class SkWuffsCodec final : public SkScalingCodec {
public:
    SkWuffsCodec(SkEncodedInfo&&                                         encodedInfo,
                 std::unique_ptr<SkStream>                               stream,
                 std::unique_ptr<wuffs_gif__decoder, decltype(&sk_free)> dec,
                 std::unique_ptr<uint8_t, decltype(&sk_free)>            workbuf_ptr,
                 size_t                                                  workbuf_len,
                 wuffs_base__image_config                                imgcfg,
                 wuffs_base__io_buffer                                   iobuf);

    const SkWuffsFrame* frame(int i) const;

private:
    // It is valid, in terms of the SkCodec API, to call SkCodec::getFrameCount
    // while in an incremental decode (after onStartIncrementalDecode returns
    // and before the rest of the image is decoded). Some Skia users expect
    // getFrameCount to increase, and the SkStream to advance, when given more
    // data.
    //
    // On the other hand, while in an incremental decode, the underlying Wuffs
    // object is suspended in a coroutine. To keep its internal proof-of-safety
    // invariants consistent, there's only two things you can safely do with a
    // suspended Wuffs object: resume the coroutine, or reset all state (memset
    // to zero and start again).
    //
    // The Wuffs API provides a limited, optional form of seeking, to the start
    // of an animation frame's data, but does not provide arbitrary save and
    // load of its internal state whilst in the middle of an animation frame.
    //
    // SkWuffsCodec therefore uses two Wuffs decoders: a primary decoder
    // (kIncrDecode) to support startIncrementalDecode / incrementalDecode, and
    // a secondary decoder (kFrameCount) to support getFrameCount. The two
    // decoders' states can change independently.
    //
    // As of Wuffs version 0.2, both of these decoders have the same type. A
    // future Wuffs version might let us use a different type for kFrameCount,
    // one that is much lighter weight (in terms of memory requirements), as it
    // doesn't have to handle decompressing pixel data.
    enum WhichDecoder {
        kIncrDecode,
        kFrameCount,
        kNumDecoders,
    };

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

    // Two separate implementations of onStartIncrementalDecode and
    // onIncrementalDecode, named "one pass" and "two pass" decoding. One pass
    // decoding writes directly from the Wuffs image decoder to the dst buffer
    // (the dst argument to onStartIncrementalDecode). Two pass decoding first
    // writes into an intermediate buffer, and then composites and transforms
    // the intermediate buffer into the dst buffer.
    //
    // In the general case, we need the two pass decoder, because of Skia API
    // features that Wuffs doesn't support (e.g. color correction, scaling,
    // RGB565). But as an optimization, we use one pass decoding (it's faster
    // and uses less memory) if applicable (see the assignment to
    // fIncrDecOnePass that calculates when we can do so).
    Result onStartIncrementalDecodeOnePass(const SkImageInfo&       dstInfo,
                                           uint8_t*                 dst,
                                           size_t                   rowBytes,
                                           const SkCodec::Options&  options,
                                           wuffs_base__pixel_format pixelFormat,
                                           size_t                   bytesPerPixel);
    Result onStartIncrementalDecodeTwoPass();
    Result onIncrementalDecodeOnePass();
    Result onIncrementalDecodeTwoPass();

    void        onGetFrameCountInternal();
    Result      seekFrame(WhichDecoder which, int frameIndex);
    Result      resetDecoder(WhichDecoder which);
    const char* decodeFrameConfig(WhichDecoder which);
    const char* decodeFrame(WhichDecoder which);
    void        updateNumFullyReceivedFrames(WhichDecoder which);

    SkWuffsFrameHolder                           fFrameHolder;
    std::unique_ptr<SkStream>                    fStream;
    std::unique_ptr<uint8_t, decltype(&sk_free)> fWorkbufPtr;
    size_t                                       fWorkbufLen;

    std::unique_ptr<wuffs_gif__decoder, decltype(&sk_free)> fDecoders[WhichDecoder::kNumDecoders];

    const uint64_t           fFirstFrameIOPosition;
    wuffs_base__frame_config fFrameConfigs[WhichDecoder::kNumDecoders];
    wuffs_base__pixel_config fPixelConfig;
    wuffs_base__pixel_buffer fPixelBuffer;
    wuffs_base__io_buffer    fIOBuffer;

    // Incremental decoding state.
    uint8_t* fIncrDecDst;
    uint64_t fIncrDecReaderIOPosition;
    size_t   fIncrDecRowBytes;
    bool     fIncrDecOnePass;
    bool     fFirstCallToIncrementalDecode;

    // Lazily allocated intermediate pixel buffer, for two pass decoding.
    std::unique_ptr<uint8_t, decltype(&sk_free)> fTwoPassPixbufPtr;
    size_t                                       fTwoPassPixbufLen;

    uint64_t                  fFrameCountReaderIOPosition;
    uint64_t                  fNumFullyReceivedFrames;
    std::vector<SkWuffsFrame> fFrames;
    bool                      fFramesComplete;

    // If calling an fDecoders[which] method returns an incomplete status, then
    // fDecoders[which] is suspended in a coroutine (i.e. waiting on I/O or
    // halted on a non-recoverable error). To keep its internal proof-of-safety
    // invariants consistent, there's only two things you can safely do with a
    // suspended Wuffs object: resume the coroutine, or reset all state (memset
    // to zero and start again).
    //
    // If fDecoderIsSuspended[which], and we aren't sure that we're going to
    // resume the coroutine, then we will need to call this->resetDecoder
    // before calling other fDecoders[which] methods.
    bool fDecoderIsSuspended[WhichDecoder::kNumDecoders];

    uint8_t fBuffer[SK_WUFFS_CODEC_BUFFER_SIZE];

    typedef SkScalingCodec INHERITED;
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
    SkCodec::FrameInfo ret;
    ret.fRequiredFrame = getRequiredFrame();
    ret.fDuration = getDuration();
    ret.fFullyReceived = fullyReceived;
    ret.fAlphaType = hasAlpha() ? kUnpremul_SkAlphaType : kOpaque_SkAlphaType;
    ret.fDisposalMethod = getDisposalMethod();
    return ret;
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

// -------------------------------- SkWuffsCodec implementation

SkWuffsCodec::SkWuffsCodec(SkEncodedInfo&&                                         encodedInfo,
                           std::unique_ptr<SkStream>                               stream,
                           std::unique_ptr<wuffs_gif__decoder, decltype(&sk_free)> dec,
                           std::unique_ptr<uint8_t, decltype(&sk_free)>            workbuf_ptr,
                           size_t                                                  workbuf_len,
                           wuffs_base__image_config                                imgcfg,
                           wuffs_base__io_buffer                                   iobuf)
    : INHERITED(std::move(encodedInfo),
                skcms_PixelFormat_RGBA_8888,
                // Pass a nullptr SkStream to the SkCodec constructor. We
                // manage the stream ourselves, as the default SkCodec behavior
                // is too trigger-happy on rewinding the stream.
                nullptr),
      fFrameHolder(),
      fStream(std::move(stream)),
      fWorkbufPtr(std::move(workbuf_ptr)),
      fWorkbufLen(workbuf_len),
      fDecoders{
          std::move(dec),
          std::unique_ptr<wuffs_gif__decoder, decltype(&sk_free)>(nullptr, sk_free),
      },
      fFirstFrameIOPosition(imgcfg.first_frame_io_position()),
      fFrameConfigs{
          wuffs_base__null_frame_config(),
          wuffs_base__null_frame_config(),
      },
      fPixelConfig(imgcfg.pixcfg),
      fPixelBuffer(wuffs_base__null_pixel_buffer()),
      fIOBuffer(wuffs_base__empty_io_buffer()),
      fIncrDecDst(nullptr),
      fIncrDecReaderIOPosition(0),
      fIncrDecRowBytes(0),
      fIncrDecOnePass(false),
      fFirstCallToIncrementalDecode(false),
      fTwoPassPixbufPtr(nullptr, &sk_free),
      fTwoPassPixbufLen(0),
      fFrameCountReaderIOPosition(0),
      fNumFullyReceivedFrames(0),
      fFramesComplete(false),
      fDecoderIsSuspended{
          false,
          false,
      } {
    fFrameHolder.init(this, imgcfg.pixcfg.width(), imgcfg.pixcfg.height());

    // Initialize fIOBuffer's fields, copying any outstanding data from iobuf to
    // fIOBuffer, as iobuf's backing array may not be valid for the lifetime of
    // this SkWuffsCodec object, but fIOBuffer's backing array (fBuffer) is.
    SkASSERT(iobuf.data.len == SK_WUFFS_CODEC_BUFFER_SIZE);
    memmove(fBuffer, iobuf.data.ptr, iobuf.meta.wi);
    fIOBuffer.data = wuffs_base__make_slice_u8(fBuffer, SK_WUFFS_CODEC_BUFFER_SIZE);
    fIOBuffer.meta = iobuf.meta;
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
    if (!dst) {
        return SkCodec::kInvalidParameters;
    }
    if (options.fSubset) {
        return SkCodec::kUnimplemented;
    }
    if (options.fFrameIndex > 0 && SkColorTypeIsAlwaysOpaque(dstInfo.colorType())) {
        return SkCodec::kInvalidConversion;
    }
    SkCodec::Result result = this->seekFrame(WhichDecoder::kIncrDecode, options.fFrameIndex);
    if (result != SkCodec::kSuccess) {
        return result;
    }

    const char* status = this->decodeFrameConfig(WhichDecoder::kIncrDecode);
    if (status == wuffs_base__suspension__short_read) {
        return SkCodec::kIncompleteInput;
    } else if (status != nullptr) {
        SkCodecPrintf("decodeFrameConfig: %s", status);
        return SkCodec::kErrorInInput;
    }

    wuffs_base__pixel_format pixelFormat = WUFFS_BASE__PIXEL_FORMAT__INVALID;
    size_t                   bytesPerPixel = 0;

    switch (dstInfo.colorType()) {
        case kBGRA_8888_SkColorType:
            pixelFormat = WUFFS_BASE__PIXEL_FORMAT__BGRA_NONPREMUL;
            bytesPerPixel = 4;
            break;
        case kRGBA_8888_SkColorType:
            pixelFormat = WUFFS_BASE__PIXEL_FORMAT__RGBA_NONPREMUL;
            bytesPerPixel = 4;
            break;
        default:
            break;
    }

    // We can use "one pass" decoding if we have a Skia pixel format that Wuffs
    // supports...
    fIncrDecOnePass =
        (pixelFormat != WUFFS_BASE__PIXEL_FORMAT__INVALID) &&
        // ...and no color profile (as Wuffs does not support them)...
        (!getEncodedInfo().profile()) &&
        // ...and we have an independent frame (as Wuffs does not support the
        // equivalent of SkBlendMode::kSrcOver)...
        ((options.fFrameIndex == 0) ||
         (this->frame(options.fFrameIndex)->getRequiredFrame() == SkCodec::kNoFrame)) &&
        // ...and we use the identity transform (as Wuffs does not support
        // scaling).
        (this->dimensions() == dstInfo.dimensions());

    result = fIncrDecOnePass ? this->onStartIncrementalDecodeOnePass(
                                   dstInfo, static_cast<uint8_t*>(dst), rowBytes, options,
                                   pixelFormat, bytesPerPixel)
                             : this->onStartIncrementalDecodeTwoPass();
    if (result != SkCodec::kSuccess) {
        return result;
    }

    fIncrDecDst = static_cast<uint8_t*>(dst);
    fIncrDecReaderIOPosition = fIOBuffer.reader_io_position();
    fIncrDecRowBytes = rowBytes;
    fFirstCallToIncrementalDecode = true;
    return SkCodec::kSuccess;
}

SkCodec::Result SkWuffsCodec::onStartIncrementalDecodeOnePass(const SkImageInfo&       dstInfo,
                                                              uint8_t*                 dst,
                                                              size_t                   rowBytes,
                                                              const SkCodec::Options&  options,
                                                              wuffs_base__pixel_format pixelFormat,
                                                              size_t bytesPerPixel) {
    wuffs_base__pixel_config pixelConfig;
    pixelConfig.set(pixelFormat, WUFFS_BASE__PIXEL_SUBSAMPLING__NONE, dstInfo.width(),
                    dstInfo.height());

    wuffs_base__table_u8 table;
    table.ptr = dst;
    table.width = static_cast<size_t>(dstInfo.width()) * bytesPerPixel;
    table.height = dstInfo.height();
    table.stride = rowBytes;

    const char* status = fPixelBuffer.set_from_table(&pixelConfig, table);
    if (status != nullptr) {
        SkCodecPrintf("set_from_table: %s", status);
        return SkCodec::kInternalError;
    }

    SkSampler::Fill(dstInfo, dst, rowBytes, options.fZeroInitialized);
    return SkCodec::kSuccess;
}

SkCodec::Result SkWuffsCodec::onStartIncrementalDecodeTwoPass() {
    // Either re-use the previously allocated "two pass" pixel buffer (and
    // memset to zero), or allocate (and zero initialize) a new one.
    bool already_zeroed = false;

    if (!fTwoPassPixbufPtr) {
        uint64_t pixbuf_len = fPixelConfig.pixbuf_len();
        void*    pixbuf_ptr_raw = (pixbuf_len <= SIZE_MAX)
                                   ? sk_malloc_flags(pixbuf_len, SK_MALLOC_ZERO_INITIALIZE)
                                   : nullptr;
        if (!pixbuf_ptr_raw) {
            return SkCodec::kInternalError;
        }
        fTwoPassPixbufPtr.reset(reinterpret_cast<uint8_t*>(pixbuf_ptr_raw));
        fTwoPassPixbufLen = SkToSizeT(pixbuf_len);
        already_zeroed = true;
    }

    const char* status = fPixelBuffer.set_from_slice(
        &fPixelConfig, wuffs_base__make_slice_u8(fTwoPassPixbufPtr.get(), fTwoPassPixbufLen));
    if (status != nullptr) {
        SkCodecPrintf("set_from_slice: %s", status);
        return SkCodec::kInternalError;
    }

    if (!already_zeroed) {
        uint32_t src_bits_per_pixel =
            wuffs_base__pixel_format__bits_per_pixel(fPixelConfig.pixel_format());
        if ((src_bits_per_pixel == 0) || (src_bits_per_pixel % 8 != 0)) {
            return SkCodec::kInternalError;
        }
        size_t src_bytes_per_pixel = src_bits_per_pixel / 8;

        wuffs_base__rect_ie_u32 frame_rect = fFrameConfigs[WhichDecoder::kIncrDecode].bounds();
        wuffs_base__table_u8    pixels = fPixelBuffer.plane(0);

        uint8_t* ptr = pixels.ptr + (frame_rect.min_incl_y * pixels.stride) +
                       (frame_rect.min_incl_x * src_bytes_per_pixel);
        size_t len = frame_rect.width() * src_bytes_per_pixel;

        // As an optimization, issue a single sk_bzero call, if possible.
        // Otherwise, zero out each row separately.
        if ((len == pixels.stride) && (frame_rect.min_incl_y < frame_rect.max_excl_y)) {
            sk_bzero(ptr, len * (frame_rect.max_excl_y - frame_rect.min_incl_y));
        } else {
            for (uint32_t y = frame_rect.min_incl_y; y < frame_rect.max_excl_y; y++) {
                sk_bzero(ptr, len);
                ptr += pixels.stride;
            }
        }
    }

    return SkCodec::kSuccess;
}

SkCodec::Result SkWuffsCodec::onIncrementalDecode(int* rowsDecoded) {
    if (!fIncrDecDst) {
        return SkCodec::kInternalError;
    }

    // If multiple SkCodec::incrementalDecode calls are made consecutively (or
    // if SkCodec::incrementalDecode is called immediately after
    // SkCodec::startIncrementalDecode), then this seek should be a no-op.
    // However, it is possible to interleave SkCodec::getFrameCount calls in
    // between SkCodec::incrementalDecode calls, and those other calls may
    // advance the stream. This seek restores the stream to where the last
    // SkCodec::startIncrementalDecode or SkCodec::incrementalDecode stopped.
    if (!seek_buffer(&fIOBuffer, fStream.get(), fIncrDecReaderIOPosition)) {
        return SkCodec::kInternalError;
    }

    if (rowsDecoded) {
        *rowsDecoded = dstInfo().height();
    }

    SkCodec::Result result =
        fIncrDecOnePass ? this->onIncrementalDecodeOnePass() : this->onIncrementalDecodeTwoPass();
    if (result == SkCodec::kSuccess) {
        fIncrDecDst = nullptr;
        fIncrDecReaderIOPosition = 0;
        fIncrDecRowBytes = 0;
        fIncrDecOnePass = false;
    } else {
        fIncrDecReaderIOPosition = fIOBuffer.reader_io_position();
    }
    return result;
}

SkCodec::Result SkWuffsCodec::onIncrementalDecodeOnePass() {
    const char* status = this->decodeFrame(WhichDecoder::kIncrDecode);
    if (status != nullptr) {
        if (status == wuffs_base__suspension__short_read) {
            return SkCodec::kIncompleteInput;
        } else {
            SkCodecPrintf("decodeFrame: %s", status);
            return SkCodec::kErrorInInput;
        }
    }
    return SkCodec::kSuccess;
}

SkCodec::Result SkWuffsCodec::onIncrementalDecodeTwoPass() {
    SkCodec::Result result = SkCodec::kSuccess;
    const char*     status = this->decodeFrame(WhichDecoder::kIncrDecode);
    bool            independent;
    SkAlphaType     alphaType;
    const int       index = options().fFrameIndex;
    if (index == 0) {
        independent = true;
        alphaType = to_alpha_type(getEncodedInfo().opaque());
    } else {
        const SkWuffsFrame* f = this->frame(index);
        independent = f->getRequiredFrame() == SkCodec::kNoFrame;
        alphaType = to_alpha_type(f->reportedAlpha() == SkEncodedInfo::kOpaque_Alpha);
    }
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

    wuffs_base__rect_ie_u32 frame_rect = fFrameConfigs[WhichDecoder::kIncrDecode].bounds();
    if (fFirstCallToIncrementalDecode) {
        if (frame_rect.width() > (SIZE_MAX / src_bytes_per_pixel)) {
            return SkCodec::kInternalError;
        }

        auto bounds = SkIRect::MakeLTRB(frame_rect.min_incl_x, frame_rect.min_incl_y,
                                        frame_rect.max_excl_x, frame_rect.max_excl_y);

        // If the frame rect does not fill the output, ensure that those pixels are not
        // left uninitialized.
        if (independent && (bounds != this->bounds() || result != kSuccess)) {
            SkSampler::Fill(dstInfo(), fIncrDecDst, fIncrDecRowBytes, options().fZeroInitialized);
        }
        fFirstCallToIncrementalDecode = false;
    } else {
        // Existing clients intend to only show frames beyond the first if they
        // are complete (based on FrameInfo::fFullyReceived), since it might
        // look jarring to draw a partial frame over an existing frame. If they
        // changed their behavior and expected to continue decoding a partial
        // frame after the first one, we'll need to update our blending code.
        // Otherwise, if the frame were interlaced and not independent, the
        // second pass may have an overlapping dirty_rect with the first,
        // resulting in blending with the first pass.
        SkASSERT(index == 0);
    }

    // If the frame's dirty rect is empty, no need to swizzle.
    wuffs_base__rect_ie_u32 dirty_rect = fDecoders[WhichDecoder::kIncrDecode]->frame_dirty_rect();
    if (!dirty_rect.is_empty()) {
        wuffs_base__table_u8 pixels = fPixelBuffer.plane(0);

        // The Wuffs model is that the dst buffer is the image, not the frame.
        // The expectation is that you allocate the buffer once, but re-use it
        // for the N frames, regardless of each frame's top-left co-ordinate.
        //
        // To get from the start (in the X-direction) of the image to the start
        // of the dirty_rect, we adjust s by (dirty_rect.min_incl_x * src_bytes_per_pixel).
        uint8_t* s = pixels.ptr + (dirty_rect.min_incl_y * pixels.stride) +
                     (dirty_rect.min_incl_x * src_bytes_per_pixel);

        // Currently, this is only used for GIF, which will never have an ICC profile. When it is
        // used for other formats that might have one, we will need to transform from profiles that
        // do not have corresponding SkColorSpaces.
        SkASSERT(!getEncodedInfo().profile());

        auto srcInfo =
            getInfo().makeWH(dirty_rect.width(), dirty_rect.height()).makeAlphaType(alphaType);
        SkBitmap src;
        src.installPixels(srcInfo, s, pixels.stride);
        SkPaint paint;
        if (independent) {
            paint.setBlendMode(SkBlendMode::kSrc);
        }

        SkDraw draw;
        draw.fDst.reset(dstInfo(), fIncrDecDst, fIncrDecRowBytes);
        SkMatrix matrix = SkMatrix::MakeRectToRect(SkRect::Make(this->dimensions()),
                                                   SkRect::Make(this->dstInfo().dimensions()),
                                                   SkMatrix::kFill_ScaleToFit);
        draw.fMatrix = &matrix;
        SkRasterClip rc(SkIRect::MakeSize(this->dstInfo().dimensions()));
        draw.fRC = &rc;

        SkMatrix translate = SkMatrix::MakeTrans(dirty_rect.min_incl_x, dirty_rect.min_incl_y);
        draw.drawBitmap(src, translate, nullptr, paint);
    }

    if (result == SkCodec::kSuccess) {
        // On success, we are done using the "two pass" pixel buffer for this
        // frame. We have the option of releasing its memory, but there is a
        // trade-off. If decoding a subsequent frame will also need "two pass"
        // decoding, it would have to re-allocate the buffer instead of just
        // re-using it. On the other hand, if there is no subsequent frame, and
        // the SkWuffsCodec object isn't deleted soon, then we are holding
        // megabytes of memory longer than we need to.
        //
        // For example, when the Chromium web browser decodes the <img> tags in
        // a HTML page, the SkCodec object can live until navigating away from
        // the page, which can be much longer than when the pixels are fully
        // decoded, especially for a still (non-animated) image. Even for
        // looping animations, caching the decoded frames (at the higher HTML
        // renderer layer) may mean that each frame is only decoded once (at
        // the lower SkCodec layer), in sequence.
        //
        // The heuristic we use here is to free the memory if we have decoded
        // the last frame of the animation (or, for still images, the only
        // frame). The output of the next decode request (if any) should be the
        // same either way, but the steady state memory use should hopefully be
        // lower than always keeping the fTwoPassPixbufPtr buffer up until the
        // SkWuffsCodec destructor runs.
        //
        // This only applies to "two pass" decoding. "One pass" decoding does
        // not allocate, free or otherwise use fTwoPassPixbufPtr.
        if (fFramesComplete && (static_cast<size_t>(options().fFrameIndex) == fFrames.size() - 1)) {
            fTwoPassPixbufPtr.reset(nullptr);
            fTwoPassPixbufLen = 0;
        }
    }

    return result;
}

int SkWuffsCodec::onGetFrameCount() {
    if (!fFramesComplete && seek_buffer(&fIOBuffer, fStream.get(), fFrameCountReaderIOPosition)) {
        this->onGetFrameCountInternal();
        fFrameCountReaderIOPosition =
            fDecoders[WhichDecoder::kFrameCount] ? fIOBuffer.reader_io_position() : 0;
    }
    return fFrames.size();
}

void SkWuffsCodec::onGetFrameCountInternal() {
    if (!fDecoders[WhichDecoder::kFrameCount]) {
        void* decoder_raw = sk_malloc_canfail(sizeof__wuffs_gif__decoder());
        if (!decoder_raw) {
            return;
        }
        std::unique_ptr<wuffs_gif__decoder, decltype(&sk_free)> decoder(
            reinterpret_cast<wuffs_gif__decoder*>(decoder_raw), &sk_free);
        reset_and_decode_image_config(decoder.get(), nullptr, &fIOBuffer, fStream.get());
        fDecoders[WhichDecoder::kFrameCount] = std::move(decoder);
    }

    // Iterate through the frames, converting from Wuffs'
    // wuffs_base__frame_config type to Skia's SkWuffsFrame type.
    while (true) {
        const char* status = this->decodeFrameConfig(WhichDecoder::kFrameCount);
        if (status == nullptr) {
            // No-op.
        } else if (status == wuffs_base__warning__end_of_data) {
            break;
        } else {
            return;
        }

        uint64_t i = fDecoders[WhichDecoder::kFrameCount]->num_decoded_frame_configs();
        if (i > INT_MAX) {
            break;
        }
        if ((i == 0) || (static_cast<size_t>(i - 1) != fFrames.size())) {
            continue;
        }
        fFrames.emplace_back(&fFrameConfigs[WhichDecoder::kFrameCount]);
        SkWuffsFrame* f = &fFrames[fFrames.size() - 1];
        fFrameHolder.setAlphaAndRequiredFrame(f);
    }

    fFramesComplete = true;

    // We've seen the end of the animation. There'll be no more frames, so we
    // no longer need the kFrameCount decoder. Releasing it earlier than the
    // SkWuffsCodec destructor might help peak memory use.
    fDecoders[WhichDecoder::kFrameCount].reset(nullptr);
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
    uint32_t n = fDecoders[WhichDecoder::kIncrDecode]->num_animation_loops();
    if (n == 0) {
        return SkCodec::kRepetitionCountInfinite;
    }
    n--;
    return n < INT_MAX ? n : INT_MAX;
}

SkCodec::Result SkWuffsCodec::seekFrame(WhichDecoder which, int frameIndex) {
    if (fDecoderIsSuspended[which]) {
        SkCodec::Result res = this->resetDecoder(which);
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
    const char* status =
        fDecoders[which]->restart_frame(frameIndex, fIOBuffer.reader_io_position());
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

SkCodec::Result SkWuffsCodec::resetDecoder(WhichDecoder which) {
    if (!fStream->rewind()) {
        return SkCodec::kInternalError;
    }
    fIOBuffer.meta = wuffs_base__empty_io_buffer_meta();

    SkCodec::Result result =
        reset_and_decode_image_config(fDecoders[which].get(), nullptr, &fIOBuffer, fStream.get());
    if (result == SkCodec::kIncompleteInput) {
        return SkCodec::kInternalError;
    } else if (result != SkCodec::kSuccess) {
        return result;
    }

    fDecoderIsSuspended[which] = false;
    return SkCodec::kSuccess;
}

const char* SkWuffsCodec::decodeFrameConfig(WhichDecoder which) {
    while (true) {
        const char* status =
            fDecoders[which]->decode_frame_config(&fFrameConfigs[which], &fIOBuffer);
        if ((status == wuffs_base__suspension__short_read) &&
            fill_buffer(&fIOBuffer, fStream.get())) {
            continue;
        }
        fDecoderIsSuspended[which] = !wuffs_base__status__is_complete(status);
        this->updateNumFullyReceivedFrames(which);
        return status;
    }
}

const char* SkWuffsCodec::decodeFrame(WhichDecoder which) {
    while (true) {
        const char* status = fDecoders[which]->decode_frame(
            &fPixelBuffer, &fIOBuffer, wuffs_base__make_slice_u8(fWorkbufPtr.get(), fWorkbufLen),
            NULL);
        if ((status == wuffs_base__suspension__short_read) &&
            fill_buffer(&fIOBuffer, fStream.get())) {
            continue;
        }
        fDecoderIsSuspended[which] = !wuffs_base__status__is_complete(status);
        this->updateNumFullyReceivedFrames(which);
        return status;
    }
}

void SkWuffsCodec::updateNumFullyReceivedFrames(WhichDecoder which) {
    // num_decoded_frames's return value, n, can change over time, both up and
    // down, as we seek back and forth in the underlying stream.
    // fNumFullyReceivedFrames is the highest n we've seen.
    uint64_t n = fDecoders[which]->num_decoded_frames();
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
    uint8_t               buffer[SK_WUFFS_CODEC_BUFFER_SIZE];
    wuffs_base__io_buffer iobuf =
        wuffs_base__make_io_buffer(wuffs_base__make_slice_u8(buffer, SK_WUFFS_CODEC_BUFFER_SIZE),
                                   wuffs_base__empty_io_buffer_meta());
    wuffs_base__image_config imgcfg = wuffs_base__null_image_config();

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
    return std::unique_ptr<SkCodec>(new SkWuffsCodec(std::move(encodedInfo), std::move(stream),
                                                     std::move(decoder), std::move(workbuf_ptr),
                                                     workbuf_len, imgcfg, iobuf));
}
