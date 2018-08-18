/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkWuffsGifCodec.h"

// TODO(nigeltao): use a swizzler instead of load_u32le and store_u32le.

static inline uint32_t load_u32le(uint8_t* p) {
    return ((uint32_t)(p[0]) << 0) | ((uint32_t)(p[1]) << 8) | ((uint32_t)(p[2]) << 16) |
           ((uint32_t)(p[3]) << 24);
}

static inline void store_u32le(uint8_t* p, uint32_t x) {
    p[0] = x >> 0;
    p[1] = x >> 8;
    p[2] = x >> 16;
    p[3] = x >> 24;
}

static SkCodecAnimation::Blend wuffs_blend_to_skia_blend(bool w) {
    return w ? SkCodecAnimation::Blend::kPriorFrame : SkCodecAnimation::Blend::kBG;
}

static SkCodecAnimation::DisposalMethod wuffs_disposal_to_skia_disposal(
    wuffs_base__animation_disposal w) {
    switch (w) {
        case WUFFS_BASE__ANIMATION_DISPOSAL__RESTORE_BACKGROUND:
            return SkCodecAnimation::DisposalMethod::kRestoreBGColor;
        case WUFFS_BASE__ANIMATION_DISPOSAL__RESTORE_PREVIOUS:
            return SkCodecAnimation::DisposalMethod::kRestorePrevious;
    }
    return SkCodecAnimation::DisposalMethod::kKeep;
}

SkCodec::FrameInfo SkWuffsGifFrame::frameInfo() const {
    // TODO(nigeltao): this conversion isn't specific to Wuffs or GIF. Should
    // it live elsewhere in Skia (but still under src/codec)??
    SkCodec::FrameInfo i = ((SkCodec::FrameInfo){});
    i.fRequiredFrame = getRequiredFrame();
    i.fDuration = getDuration();

    // TODO(nigeltao): look for wuffs_base__suspension__end_of_data during
    // onGetFrameCount and, if seen, set fFullyReceived of the last FrameInfo
    // (if present) to false?
    i.fFullyReceived = true;

    // TODO(nigeltao): should we sometimes set kOpaque_Alpha instead?? See the
    // comment in SkWuffsGifFrame::onReportedAlpha.
    i.fAlphaType = kUnpremul_SkAlphaType;
    i.fDisposalMethod = getDisposalMethod();
    return i;
}

SkEncodedInfo::Alpha SkWuffsGifFrame::onReportedAlpha() const {
    // TODO(nigeltao): return SkEncodedInfo::kOpaque_Alpha (or the equivalent
    // Alpha value, not AlphaType value) where possible?? See the "can we really
    // claim kOpaque_Alpha" comment below.
    return SkEncodedInfo::kBinary_Alpha;
}

void SkWuffsGifFrameHolder::init(SkWuffsGifCodec* codec, int width, int height) {
    fCodec = codec;
    fScreenWidth = width;
    fScreenHeight = height;
}

const SkFrame* SkWuffsGifFrameHolder::onGetFrame(int i) const {
    return fCodec->frame(i);
};

#define GIF87_STAMP "GIF87a"
#define GIF89_STAMP "GIF89a"
#define GIF_STAMP_LEN 6

bool SkWuffsGifCodec::IsGif(const void* buf, size_t bytesRead) {
    return (bytesRead >= GIF_STAMP_LEN) && (memcmp(GIF87_STAMP, buf, GIF_STAMP_LEN) == 0 ||
                                            memcmp(GIF89_STAMP, buf, GIF_STAMP_LEN) == 0);
}

static bool fill_buffer(wuffs_base__io_buffer* b, SkStream* s) {
    b->compact();
    size_t num_read = s->read(b->ptr + b->wi, b->len - b->wi);
    b->wi += num_read;
    b->closed = s->isAtEnd();
    return num_read > 0;
}

std::unique_ptr<SkCodec> SkWuffsGifCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
                                                         Result* result) {
    std::unique_ptr<wuffs_gif__decoder> decoder(new wuffs_gif__decoder);
    if (!decoder) {
        *result = SkCodec::kInternalError;
        return nullptr;
    }

    memset(decoder.get(), 0, sizeof wuffs_gif__decoder{});
    if (decoder->check_wuffs_version(sizeof wuffs_gif__decoder{}, WUFFS_VERSION) != nullptr) {
        *result = SkCodec::kInternalError;
        return nullptr;
    }

    wuffs_base__image_config imgcfg = ((wuffs_base__image_config){});

    uint8_t buffer[SK_WUFFS_GIF_CODEC_BUFFER_SIZE];
    wuffs_base__io_buffer iobuf = ((wuffs_base__io_buffer){});
    iobuf.ptr = buffer;
    iobuf.len = SK_WUFFS_GIF_CODEC_BUFFER_SIZE;
    while (1) {
        wuffs_base__status z = decoder->decode_image_config(&imgcfg, iobuf.reader());
        if (z == nullptr) {
            break;
        } else if (z == wuffs_base__suspension__end_of_data) {
            // The GIF image has no frames, and is therefore invalid.
            *result = SkCodec::kIncompleteInput;
            return nullptr;
        } else if (z != wuffs_base__suspension__short_read) {
            *result = SkCodec::kInternalError;
            return nullptr;
        } else if (!fill_buffer(&iobuf, stream.get())) {
            *result = SkCodec::kIncompleteInput;
            return nullptr;
        }
    }

    if (!imgcfg.is_valid()) {
        *result = SkCodec::kInternalError;
        return nullptr;
    }

    if ((imgcfg.pixcfg.width() > INT_MAX) || (imgcfg.pixcfg.height() > INT_MAX)) {
        // TODO: is kInvalidConversion the right Result code? Here, the image's
        // dimensions, which Wuffs reports as a uint32_t, overflows the int that
        // Skia's API uses.
        //
        // For GIF, the max height is 65535, which won't overflow. But if we
        // generalize this Wuffs SkCodec implementation to handle other file
        // formats (e.g. use the Wuffs PNG implementation), we might have to
        // care about a 4 billion pixel high image, which the PNG file format
        // allows.
        *result = SkCodec::kInvalidConversion;
        return nullptr;
    }

    if ((imgcfg.pixcfg.width() == 0) || (imgcfg.pixcfg.height() == 0)) {
        // TODO: is kInvalidConversion the right Result code?
        //
        // The GIF spec, and the Wuffs implementation, doesn't rule out a
        // 0-width or 0-height image, but the Skia convention is to reject.
        *result = SkCodec::kInvalidConversion;
        return nullptr;
    }

    size_t pixbuf_len = imgcfg.pixcfg.pixbuf_size();
    std::unique_ptr<uint8_t[]> pixbuf_ptr = std::make_unique<uint8_t[]>(pixbuf_len);
    if (!pixbuf_ptr) {
        *result = SkCodec::kInternalError;
        return nullptr;
    }
    wuffs_base__pixel_buffer pixbuf = ((wuffs_base__pixel_buffer){});
    wuffs_base__status z = pixbuf.set_from_slice(&imgcfg.pixcfg, ((wuffs_base__slice_u8){
                                                                     .ptr = pixbuf_ptr.get(),
                                                                     .len = pixbuf_len,
                                                                 }));
    if (z != nullptr) {
        *result = SkCodec::kInternalError;
        return nullptr;
    }

    bool ffio = imgcfg.first_frame_is_opaque();

    // TODO(nigeltao): can we really claim kOpaque_Alpha, even if the first
    // frame is completely opaque and fills the entire image bounds? A later
    // frame might be SkCodecAnimation::DisposalMethod::kRestoreBGColor, and so
    // frames after that might contain some transparent black pixels, which are
    // obviously not opaque.
    //
    // Note that, at this point in time, we have only decoded the GIF header,
    // and have not decoded every frame (and in particular, their disposal
    // method).
    const auto alpha = ffio ? SkEncodedInfo::kOpaque_Alpha : SkEncodedInfo::kBinary_Alpha;

    const auto encodedInfo = SkEncodedInfo::Make(SkEncodedInfo::kPalette_Color, alpha, 8);

    const auto alphaType = ffio ? kOpaque_SkAlphaType : kUnpremul_SkAlphaType;

    const auto imageInfo =
        SkImageInfo::Make(imgcfg.pixcfg.width(), imgcfg.pixcfg.height(),
                          // TODO(nigeltao): check that kBGRA_8888_SkColorType is the correct Skia
                          // constant (should it be kN32_SkColorType??). Wuffs uses (B, G, R, A)
                          // in memory order, regardless of endianness.
                          //
                          // When manually testing this code on linux / x86_64,
                          // kBGRA_8888_SkColorType and kN32_SkColorType happen to be the same,
                          // but that isn't true on all platform / arches.
                          kBGRA_8888_SkColorType, alphaType, SkColorSpace::MakeSRGB());

    *result = kSuccess;
    return std::unique_ptr<SkCodec>(new SkWuffsGifCodec(encodedInfo, imageInfo, std::move(stream),
                                                        std::move(decoder), std::move(pixbuf_ptr),
                                                        imgcfg, pixbuf, iobuf));
}

SkWuffsGifCodec::SkWuffsGifCodec(const SkEncodedInfo& encodedInfo,
                                 const SkImageInfo& imageInfo,
                                 std::unique_ptr<SkStream> stream,
                                 std::unique_ptr<wuffs_gif__decoder> dec,
                                 std::unique_ptr<uint8_t[]> pixbuf_ptr,
                                 wuffs_base__image_config imgcfg,
                                 wuffs_base__pixel_buffer pixbuf,
                                 wuffs_base__io_buffer iobuf)
    : INHERITED(encodedInfo,
                imageInfo,
                SkColorSpaceXform::kBGRA_8888_ColorFormat,
                // Pass a nullptr SkStream to the SkCodec constructor. We
                // manage the stream ourselves, as the default SkCodec behavior
                // is too trigger-happy on rewinding the stream.
                nullptr),
      fStream(std::move(stream)),
      fDecoder(std::move(dec)),
      fPixbufPtr(std::move(pixbuf_ptr)),
      fNumLoops(imgcfg.num_loops()),
      fPixelBuffer(pixbuf),
      fIOBuffer((wuffs_base__io_buffer){}),
      fIncrDecDst(nullptr),
      fIncrDecFrameIndex(0),
      fIncrDecRowBytes(0),
      fFramesComplete(false),
      fSuspendedInDecodeFrame(false) {
    fFrameHolder.init(this, imgcfg.pixcfg.width(), imgcfg.pixcfg.height());
    // Initialize fIOBuffer's fields, copying any outstanding data from iobuf to
    // fIOBuffer, as iobuf's backing array may not be valid for the lifetime of
    // this SkWuffsGifCodec object, but fIOBuffer's backing array (fBuffer) is.
    //
    // The wuffs_base__io_buffer type is defined in C code, not C++, so it
    // doesn't have a constructor function.
    fIOBuffer.ptr = fBuffer;
    fIOBuffer.len = SK_WUFFS_GIF_CODEC_BUFFER_SIZE;
    fIOBuffer.wi = iobuf.wi - iobuf.ri;
    if (fIOBuffer.wi) {
        memmove(fIOBuffer.ptr, iobuf.ptr + iobuf.ri, fIOBuffer.wi);
    }
    fIOBuffer.ri = 0;
    fIOBuffer.closed = iobuf.closed;
}

SkEncodedImageFormat SkWuffsGifCodec::onGetEncodedFormat() const {
    return SkEncodedImageFormat::kGIF;
}

SkCodec::Result SkWuffsGifCodec::onGetPixels(const SkImageInfo& dstInfo,
                                             void* dst,
                                             size_t rowBytes,
                                             const Options& options,
                                             int* rowsDecoded) {
    SkCodec::Result result = onStartIncrementalDecode(dstInfo, dst, rowBytes, options);
    if (result != kSuccess) {
        return result;
    }
    return onIncrementalDecode(rowsDecoded);
}

const SkFrameHolder* SkWuffsGifCodec::getFrameHolder() const {
    return &fFrameHolder;
}

SkCodec::Result SkWuffsGifCodec::onStartIncrementalDecode(const SkImageInfo& dstInfo,
                                                          void* dst,
                                                          size_t rowBytes,
                                                          const SkCodec::Options& options) {
    if (options.fSubset) {
        return SkCodec::kUnimplemented;
    }

    // TODO(nigeltao): use a swizzler.
    if (dstInfo.colorType() != kBGRA_8888_SkColorType) {
        return SkCodec::kUnimplemented;
    }

    fIncrDecDst = static_cast<uint8_t*>(dst);
    fIncrDecFrameIndex = options.fFrameIndex;
    fIncrDecRowBytes = rowBytes;

    // TODO(nigeltao): is "clear the dst image to transparent black" the right
    // thing to do? The AnimatedImage test passes on images/alphabetAnim.gif
    // with it and fails without it.
    //
    // If not here, where? If not always, when? SkGifCodec.cpp does something
    // conditionally, calling fSwizzler->fill(etc, opts.fZeroInitialized);
    // near a comment: "We may need to clear to transparent for one of the
    // following reasons".
    {
        uint32_t width = dstInfo.width();
        uint32_t height = dstInfo.height();
        for (uint32_t y = 0; y < height; y++) {
            for (uint32_t x = 0; x < width; x++) {
                uint8_t* d = fIncrDecDst + (y * fIncrDecRowBytes) + (x * 4);
                store_u32le(d, 0);
            }
        }
    }

    return SkCodec::kSuccess;
}

SkCodec::Result SkWuffsGifCodec::onIncrementalDecode(int* rowsDecoded) {
    if (!fIncrDecDst) {
        return SkCodec::kInternalError;
    }

    if (!seek(fIncrDecFrameIndex)) {
        return SkCodec::kInternalError;
    }

    wuffs_base__frame_config fc = ((wuffs_base__frame_config){});
    wuffs_base__status z = decodeFrameConfig(&fc);
    if (z == nullptr) {
        // No-op.
    } else if ((z == wuffs_base__suspension__end_of_data) ||
               (z == wuffs_base__suspension__short_read)) {
        return SkCodec::kIncompleteInput;
    } else {
        return SkCodec::kInternalError;
    }

    SkCodec::Result result = SkCodec::kSuccess;
    z = decodeFrame();
    if (z == nullptr) {
        // No-op.
    } else if (z == wuffs_base__suspension__end_of_data) {
        // TODO(nigeltao): there were no more frames to decode. We've
        // reached the end of the animation (other than looping back to the
        // start). How do we report this in Skia's API?
        return SkCodec::kIncompleteInput;
    } else if (z == wuffs_base__suspension__short_read) {
        // The input is truncated. We set result to kIncompleteInput, but we
        // don't return it straight away. We first decode as much as we can.
        result = SkCodec::kIncompleteInput;
    } else {
        return SkCodec::kInternalError;
    }

    // TODO(nigeltao): use a swizzler, once I figure out how it works. For now,
    // a C style load/store loop gets the job done.
    wuffs_base__rect_ie_u32 r = fc.bounds().intersect(fPixelBuffer.pixcfg.bounds());
    wuffs_base__table_u8 pixels = fPixelBuffer.plane(0);
    wuffs_base__slice_u8 palette = fPixelBuffer.palette();
    SkASSERT(palette.len == 4 * 256);
    for (uint32_t y = r.min_incl_y; y < r.max_excl_y; y++) {
        uint8_t* d = fIncrDecDst + (y * fIncrDecRowBytes) + (r.min_incl_x * 4);
        uint8_t* s = pixels.ptr + (y * pixels.stride) + (r.min_incl_x * 1);
        for (uint32_t x = r.min_incl_x; x < r.max_excl_x; x++) {
            uint8_t index = *s++;
            uint32_t argb = load_u32le(palette.ptr + 4 * static_cast<size_t>(index));
            store_u32le(d, argb);
            d += 4;
        }
    }

    if (rowsDecoded) {
        // TODO(nigeltao): the (clipped?) height of the frame or of the image?
        *rowsDecoded = static_cast<int>(r.height());
    }

    fIncrDecDst = nullptr;
    fIncrDecFrameIndex = 0;
    fIncrDecRowBytes = 0;
    return result;
}

int SkWuffsGifCodec::onGetFrameCount() {
    if (!fFramesComplete) {
        readFrames();
    }
    return fFrames.size();
}

bool SkWuffsGifCodec::onGetFrameInfo(int i, SkCodec::FrameInfo* frameInfo) const {
    SkWuffsGifFrame* f = frame(i);
    if (!f) {
        return false;
    }
    if (frameInfo) {
        *frameInfo = f->frameInfo();
    }
    return true;
}

int SkWuffsGifCodec::onGetRepetitionCount() {
    return fNumLoops < INT_MAX ? fNumLoops : INT_MAX;
}

SkWuffsGifFrame* SkWuffsGifCodec::frame(int i) const {
    if ((0 <= i) && (static_cast<size_t>(i) < fFrames.size())) {
        return fFrames[i].get();
    }
    return nullptr;
}

void SkWuffsGifCodec::readFrames() {
    if (!seek(fFrames.size())) {
        return;
    }

    // Iterate through the entire GIF image, converting from Wuffs'
    // wuffs_base__frame_config type to Skia's SkWuffsGifFrame type.
    for (int i = fFrames.size(); i < INT_MAX; i++) {
        wuffs_base__frame_config fc = ((wuffs_base__frame_config){});
        wuffs_base__status z = decodeFrameConfig(&fc);
        if (z == nullptr) {
            // No-op.
        } else if (z == wuffs_base__suspension__end_of_data) {
            fFramesComplete = true;
            return;
        } else {
            return;
        }

        fFrames.emplace_back(new SkWuffsGifFrame(i));

        // Get the SkWuffsGifFrame* we just new'ed back out of the fFrames
        // vector, so we can fill in its fields.
        //
        // TODO(nigeltao): this awkwardness is only because the fFrames
        // vector is a vector of unique_ptr's, and they're unique_ptr's only
        // because a SkFrame is SkNoncopyable. Check with scroggo@ if we
        // should drop the SkNoncopyable-ness or leave this as is.
        SkWuffsGifFrame* f = fFrames[fFrames.size() - 1].get();

        // TODO(nigeltao): is this clipping (the intersect call) correct?
        // The GIF spec explicitly disallows the frame rect extending beyond
        // the image rect, but in practice, GIFs found in the wild flout
        // this, including resources/images/alphabetAnim.gif in the Skia
        // repository. We clip inside onIncrementalDecode, since to do
        // otherwise invites a buffer overflow, but it's not obvious here
        // whether, in terms of the Skia API, the SkFrame or SkFrameInfo
        // frame rect should be clipped or not.
        wuffs_base__rect_ie_u32 r = fc.bounds().intersect(fPixelBuffer.pixcfg.bounds());

        f->setXYWH(r.min_incl_x, r.min_incl_y, r.width(), r.height());
        f->setDisposalMethod(wuffs_disposal_to_skia_disposal(fc.disposal()));
        f->setDuration(fc.duration() / WUFFS_BASE__FLICKS_PER_MILLISECOND);
        f->setBlend(wuffs_blend_to_skia_blend(fc.blend()));
        fFrameHolder.setAlphaAndRequiredFrame(f);
    }
}

bool SkWuffsGifCodec::rewind() {
    if (!fStream->rewind()) {
        return false;
    }

    fSuspendedInDecodeFrame = false;

    // Reset fIOBuffer, dropping any read-but-as-yet-unprocessed bytes.
    fIOBuffer.wi = 0;
    fIOBuffer.ri = 0;
    fIOBuffer.closed = false;

    // Reset fDecoder.
    memset(fDecoder.get(), 0, sizeof wuffs_gif__decoder{});
    return fDecoder->check_wuffs_version(sizeof wuffs_gif__decoder{}, WUFFS_VERSION) == nullptr;
}

bool SkWuffsGifCodec::seek(int frameIndex) {
    if (frameIndex < 0) {
        return false;
    }
    uint64_t i = frameIndex;
    // Skia is random access: "decode the i'th frame". Wuffs is sequential
    // access: "decode the next frame". If Skia asks for a future frame, we have
    // to iterate forward. If Skia asks for a past frame, we have to rewind to
    // the beginning and then iterate forward.
    //
    // First, rewind if necessary.
    if (((fDecoder->num_decoded_frame_configs() > i) || fSuspendedInDecodeFrame) && !rewind()) {
        return false;
    }
    // Second, iterate forward if necessary.
    while (fDecoder->num_decoded_frame_configs() < i) {
        if (decodeFrameConfig(nullptr) != nullptr) {
            return false;
        }
    }
    return true;
}

// An overview of the Wuffs decoding API:
//
// An animated image (such as GIF) has an image header and then N frames. The
// image header gives e.g. the overall image's width and height. Each frame
// consists of a frame header (e.g. frame rectangle bounds, display duration)
// and a payload (the pixels).
//
// In Wuffs terminology, there is one image config and then N pairs of
// (frame_config, frame). To decode everything (without knowing N in advance):
//  - call wuffs_gif__decoder::decode_image_config
//  - while (true) {
//  -   call wuffs_gif__decoder::decode_frame_config
//  -   if that returned wuffs_base__suspension__end_of_data, break
//  -   call wuffs_gif__decoder::decode_frame
//  - }
//
// The first argument to each decode_foo method is the destination struct to
// store the decoded information.
//
// All of those calls are optional. For example, if decode_image_config is not
// called, then the first decode_frame_config call will implicitly parse and
// verify the image header, before parsing the first frame's header. Similarly,
// you can call only decode_frame N times, without calling decode_image_config
// or decode_frame_config, if you already know metadata like N and each frame's
// rectangle bounds by some other means (e.g. this is a first party, statically
// known image).
//
// Specifically, starting with an unknown (but re-windable) GIF image, if you
// want to just find N (i.e. count the number of frames), you can loop calling
// only the decode_frame_config method and avoid calling the more expensive
// decode_frame method. In terms of the underlying GIF image format, this will
// skip over the LZW-encoded pixel data, avoiding the LZW decompression.

wuffs_base__status SkWuffsGifCodec::decodeFrameConfig(wuffs_base__frame_config* fc) {
    while (1) {
        wuffs_base__status z = fDecoder->decode_frame_config(fc, fIOBuffer.reader());
        if ((z == wuffs_base__suspension__short_read) && fill_buffer(&fIOBuffer, fStream.get())) {
            continue;
        }
        return z;
    }
}

wuffs_base__status SkWuffsGifCodec::decodeFrame() {
    while (1) {
        // In general, Wuffs image decoders can require a work buffer. For the
        // Wuffs GIF decoder, the work buffer can always be empty.
        wuffs_base__slice_u8 work_buffer = ((wuffs_base__slice_u8){});

        wuffs_base__status z =
            fDecoder->decode_frame(&fPixelBuffer, 0, 0, fIOBuffer.reader(), work_buffer);
        if ((z == wuffs_base__suspension__short_read) && fill_buffer(&fIOBuffer, fStream.get())) {
            continue;
        }
        fSuspendedInDecodeFrame = wuffs_base__status__is_suspension(z);
        return z;
    }
}
