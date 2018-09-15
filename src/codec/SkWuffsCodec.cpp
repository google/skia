/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkWuffsCodec.h"

#include "SkSampler.h"

// TODO(nigeltao): use a swizzler instead of load_u32le and store_etc.

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

static inline void store_u32le_switched(uint8_t* p, uint32_t x) {
    // This could probably be optimized, but in any case, we should use a
    // swizzler.
    p[0] = x >> 16;
    p[1] = x >> 8;
    p[2] = x >> 0;
    p[3] = x >> 24;
}

static inline void store_565(uint8_t* p, uint32_t argb) {
    uint32_t r5 = 0x1F & (argb >> ((8 - 5) + 16));
    uint32_t g6 = 0x3F & (argb >> ((8 - 6) + 8));
    uint32_t b5 = 0x1F & (argb >> ((8 - 5) + 0));
    p[0] = (b5 << 0) | (g6 << 5);
    p[1] = (g6 >> 3) | (r5 << 3);
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
    }
    return SkCodecAnimation::DisposalMethod::kKeep;
}

SkWuffsFrame::SkWuffsFrame(wuffs_base__frame_config* fc)
    : INHERITED(fc->index()),
      fIOPosition(fc->io_position()),
      fReportedAlpha(wuffs_blend_to_skia_alpha(fc->blend())) {
    wuffs_base__rect_ie_u32 r = fc->bounds();
    setXYWH(r.min_incl_x, r.min_incl_y, r.width(), r.height());
    setDisposalMethod(wuffs_disposal_to_skia_disposal(fc->disposal()));
    setDuration(fc->duration() / WUFFS_BASE__FLICKS_PER_MILLISECOND);
    setBlend(wuffs_blend_to_skia_blend(fc->blend()));
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

void SkWuffsFrameHolder::init(SkWuffsCodec* codec, int width, int height) {
    fCodec = codec;
    // Initialize SkFrameHolder's (the superclass) fields.
    fScreenWidth = width;
    fScreenHeight = height;
}

const SkFrame* SkWuffsFrameHolder::onGetFrame(int i) const {
    return fCodec->frame(i);
};

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

bool SkWuffsCodec::Recognizes(const void* buf, size_t bytesRead) {
    constexpr const char* gif_ptr = "GIF8";
    constexpr size_t gif_len = 4;
    return (bytesRead >= gif_len) && (memcmp(buf, gif_ptr, gif_len) == 0);
}

std::unique_ptr<SkCodec> SkWuffsCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
                                                      SkCodec::Result* result) {
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

    uint8_t buffer[SK_WUFFS_CODEC_BUFFER_SIZE];
    wuffs_base__io_buffer iobuf = ((wuffs_base__io_buffer){});
    iobuf.data.ptr = buffer;
    iobuf.data.len = SK_WUFFS_CODEC_BUFFER_SIZE;
    while (true) {
        wuffs_base__status z = decoder->decode_image_config(&imgcfg, iobuf.reader());
        if (z == nullptr) {
            break;
        } else if (z != wuffs_base__suspension__short_read) {
            *result = SkCodec::kErrorInInput;
            return nullptr;
        } else if (!fill_buffer(&iobuf, stream.get())) {
            *result = SkCodec::kIncompleteInput;
            return nullptr;
        }
    }

    uint32_t width = imgcfg.pixcfg.width();
    uint32_t height = imgcfg.pixcfg.height();
    if ((width == 0) || (width > INT_MAX) || (height == 0) || (height > INT_MAX)) {
        *result = SkCodec::kInvalidInput;
        return nullptr;
    }

    uint64_t workbuf_len = imgcfg.workbuf_len().max_incl;
    std::unique_ptr<uint8_t[]> workbuf_ptr = nullptr;
    if (workbuf_len <= SIZE_MAX) {
        workbuf_ptr = std::make_unique<uint8_t[]>(workbuf_len);
    }
    if (!workbuf_ptr) {
        *result = SkCodec::kInternalError;
        return nullptr;
    }

    uint64_t pixbuf_len = imgcfg.pixcfg.pixbuf_len();
    std::unique_ptr<uint8_t[]> pixbuf_ptr = nullptr;
    if (pixbuf_len <= SIZE_MAX) {
        pixbuf_ptr = std::make_unique<uint8_t[]>(pixbuf_len);
    }
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

    // In Skia's API, the alpha we calculate here and return is only for the
    // first frame. For animated images, later frames (with
    // SkCodecAnimation::DisposalMethod::kRestoreBGColor) might lead to
    // transparent pixels, after composition, but that's not the concern here.
    //
    // Historically, Skia started off with an API that only supported reading
    // the first frame of an animated image.
    SkEncodedInfo::Alpha alpha = imgcfg.first_frame_is_opaque() ? SkEncodedInfo::kOpaque_Alpha
                                                                : SkEncodedInfo::kBinary_Alpha;

    SkEncodedInfo encodedInfo =
        SkEncodedInfo::MakeSRGB(width, height, SkEncodedInfo::kPalette_Color, alpha, 8);

    *result = kSuccess;
    return std::unique_ptr<SkCodec>(new SkWuffsCodec(
        std::move(encodedInfo), std::move(stream), std::move(decoder), std::move(workbuf_ptr),
        workbuf_len, std::move(pixbuf_ptr), imgcfg, pixbuf, iobuf));
}

SkWuffsCodec::SkWuffsCodec(SkEncodedInfo&& encodedInfo,
                           std::unique_ptr<SkStream> stream,
                           std::unique_ptr<wuffs_gif__decoder> dec,
                           std::unique_ptr<uint8_t[]> workbuf_ptr,
                           size_t workbuf_len,
                           std::unique_ptr<uint8_t[]> pixbuf_ptr,
                           wuffs_base__image_config imgcfg,
                           wuffs_base__pixel_buffer pixbuf,
                           wuffs_base__io_buffer iobuf)
    : INHERITED(std::move(encodedInfo),
                skcms_PixelFormat_RGBA_8888,
                // Pass a nullptr SkStream to the SkCodec constructor. We
                // manage the stream ourselves, as the default SkCodec behavior
                // is too trigger-happy on rewinding the stream.
                nullptr),
      fStream(std::move(stream)),
      fDecoder(std::move(dec)),
      fWorkbufPtr(std::move(workbuf_ptr)),
      fWorkbufLen(workbuf_len),
      fPixbufPtr(std::move(pixbuf_ptr)),
      fFirstFrameIOPosition(imgcfg.first_frame_io_position()),
      fNumLoops(imgcfg.num_loops()),
      fFrameConfig((wuffs_base__frame_config){}),
      fPixelBuffer(pixbuf),
      fIOBuffer((wuffs_base__io_buffer){}),
      fIncrDecColorType(kUnknown_SkColorType),
      fIncrDecDst(nullptr),
      fIncrDecHaveFrameConfig(false),
      fIncrDecRowBytes(0),
      fFramesComplete(false),
      fNeedResetDecoder(false),
      fPreviousBestNumFullyReceived(0) {
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

SkEncodedImageFormat SkWuffsCodec::onGetEncodedFormat() const {
    return SkEncodedImageFormat::kGIF;
}

SkCodec::Result SkWuffsCodec::onGetPixels(const SkImageInfo& dstInfo,
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

const SkFrameHolder* SkWuffsCodec::getFrameHolder() const {
    return &fFrameHolder;
}

SkCodec::Result SkWuffsCodec::onStartIncrementalDecode(const SkImageInfo& dstInfo,
                                                       void* dst,
                                                       size_t rowBytes,
                                                       const SkCodec::Options& options) {
    if (options.fSubset) {
        return SkCodec::kUnimplemented;
    }
    SkCodec::Result result = seekFrame(options.fFrameIndex);
    if (result != SkCodec::kSuccess) {
        return result;
    }

    SkSampler::Fill(dstInfo, dst, rowBytes, options.fZeroInitialized);

    fIncrDecColorType = dstInfo.colorType();
    fIncrDecDst = static_cast<uint8_t*>(dst);
    fIncrDecHaveFrameConfig = false;
    fIncrDecRowBytes = rowBytes;

    return SkCodec::kSuccess;
}

SkCodec::Result SkWuffsCodec::onIncrementalDecode(int* rowsDecoded) {
    if (!fIncrDecDst) {
        return SkCodec::kInternalError;
    }

    if (!fIncrDecHaveFrameConfig) {
        wuffs_base__status z = decodeFrameConfig();
        if (z == nullptr) {
            // No-op.
        } else if (z == wuffs_base__suspension__short_read) {
            return SkCodec::kIncompleteInput;
        } else {
            return SkCodec::kErrorInInput;
        }
        fIncrDecHaveFrameConfig = true;
    }

    SkCodec::Result result = SkCodec::kSuccess;
    wuffs_base__status z = decodeFrame();
    if (z == nullptr) {
        // No-op.
    } else if (z == wuffs_base__suspension__short_read) {
        result = SkCodec::kIncompleteInput;
    } else {
        return SkCodec::kErrorInInput;
    }

    // TODO(nigeltao): use a swizzler, once I figure out how it works. For
    // now, a C style load/store loop gets the job done.
    wuffs_base__rect_ie_u32 r = fFrameConfig.bounds();
    wuffs_base__table_u8 pixels = fPixelBuffer.plane(0);
    wuffs_base__slice_u8 palette = fPixelBuffer.palette();
    SkASSERT(palette.len == 4 * 256);
    switch (fIncrDecColorType) {
        case kRGB_565_SkColorType:
            for (uint32_t y = r.min_incl_y; y < r.max_excl_y; y++) {
                uint8_t* d = fIncrDecDst + (y * fIncrDecRowBytes) + (r.min_incl_x * 2);
                uint8_t* s = pixels.ptr + (y * pixels.stride) + (r.min_incl_x * 1);
                for (uint32_t x = r.min_incl_x; x < r.max_excl_x; x++) {
                    uint8_t index = *s++;
                    uint32_t argb = load_u32le(palette.ptr + 4 * static_cast<size_t>(index));
                    store_565(d, argb);
                    d += 2;
                }
            }
            break;
        case kBGRA_8888_SkColorType:
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
            break;
        case kRGBA_8888_SkColorType:
            for (uint32_t y = r.min_incl_y; y < r.max_excl_y; y++) {
                uint8_t* d = fIncrDecDst + (y * fIncrDecRowBytes) + (r.min_incl_x * 4);
                uint8_t* s = pixels.ptr + (y * pixels.stride) + (r.min_incl_x * 1);
                for (uint32_t x = r.min_incl_x; x < r.max_excl_x; x++) {
                    uint8_t index = *s++;
                    uint32_t argb = load_u32le(palette.ptr + 4 * static_cast<size_t>(index));
                    store_u32le_switched(d, argb);
                    d += 4;
                }
            }
            break;
        default:
            return SkCodec::kUnimplemented;
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
    if (rowsDecoded) {
        *rowsDecoded = static_cast<int>(fPixelBuffer.pixcfg.height());
    }

    if (result == SkCodec::kSuccess) {
        fIncrDecColorType = kUnknown_SkColorType;
        fIncrDecDst = nullptr;
        fIncrDecHaveFrameConfig = false;
        fIncrDecRowBytes = 0;
    }
    return result;
}

int SkWuffsCodec::onGetFrameCount() {
    if (!fFramesComplete) {
        readFrames();
    }
    return fFrames.size();
}

bool SkWuffsCodec::onGetFrameInfo(int i, SkCodec::FrameInfo* frameInfo) const {
    const SkWuffsFrame* f = frame(i);
    if (!f) {
        return false;
    }
    if (frameInfo) {
        *frameInfo = f->frameInfo(static_cast<uint64_t>(i) < numFullyReceived());
    }
    return true;
}

int SkWuffsCodec::onGetRepetitionCount() {
    // Convert from Wuffs's loop count to Skia's repeat count. Wuffs' uint32_t
    // number is how many times to play the loop. Skia's int number is how many
    // times to play the loop *after the first play*. Wuffs and Skia use 0 and
    // kRepetitionCountInfinite respectively to mean loop forever.
    if (fNumLoops == 0) {
        return SkCodec::kRepetitionCountInfinite;
    }
    uint32_t n = fNumLoops - 1;
    return n < INT_MAX ? n : INT_MAX;
}

const SkWuffsFrame* SkWuffsCodec::frame(int i) const {
    if ((0 <= i) && (static_cast<size_t>(i) < fFrames.size())) {
        return &fFrames[i];
    }
    return nullptr;
}

void SkWuffsCodec::readFrames() {
    size_t n = fFrames.size();
    int i = n ? n - 1 : 0;
    if (seekFrame(i) != SkCodec::kSuccess) {
        return;
    }

    // Iterate through the frames, converting from Wuffs'
    // wuffs_base__frame_config type to Skia's SkWuffsFrame type.
    for (; i < INT_MAX; i++) {
        wuffs_base__status z = decodeFrameConfig();
        if (z == nullptr) {
            // No-op.
        } else if (z == wuffs_base__warning__end_of_data) {
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
    // Update fPreviousBestNumFullyReceived as we might seek backwards.
    uint64_t current = fDecoder->num_decoded_frames();
    if (fPreviousBestNumFullyReceived < current) {
        fPreviousBestNumFullyReceived = current;
    }

    if (fNeedResetDecoder) {
        SkCodec::Result res = resetDecoder();
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
    if (fDecoder->restart_frame(frameIndex, fIOBuffer.reader_io_position()) != nullptr) {
        return SkCodec::kInternalError;
    }
    return SkCodec::kSuccess;
}

uint64_t SkWuffsCodec::numFullyReceived() const {
    uint64_t current = fDecoder->num_decoded_frames();
    return fPreviousBestNumFullyReceived > current ? fPreviousBestNumFullyReceived : current;
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

SkCodec::Result SkWuffsCodec::resetDecoder() {
    if (!fStream->rewind()) {
        return SkCodec::kInternalError;
    }
    fIOBuffer.meta = ((wuffs_base__io_buffer_meta){});

    // Reset fDecoder, and parse the image_config once more.
    memset(fDecoder.get(), 0, sizeof wuffs_gif__decoder{});
    if (fDecoder->check_wuffs_version(sizeof wuffs_gif__decoder{}, WUFFS_VERSION) != nullptr) {
        return SkCodec::kInternalError;
    }
    while (true) {
        wuffs_base__status z = fDecoder->decode_image_config(nullptr, fIOBuffer.reader());
        if (z == nullptr) {
            break;
        } else if ((z == wuffs_base__suspension__short_read) &&
                   fill_buffer(&fIOBuffer, fStream.get())) {
            continue;
        }
        return SkCodec::kInternalError;
    }

    fNeedResetDecoder = false;
    return SkCodec::kSuccess;
}

wuffs_base__status SkWuffsCodec::decodeFrameConfig() {
    while (true) {
        wuffs_base__status z = fDecoder->decode_frame_config(&fFrameConfig, fIOBuffer.reader());
        if ((z == wuffs_base__suspension__short_read) && fill_buffer(&fIOBuffer, fStream.get())) {
            continue;
        }
        fNeedResetDecoder = !wuffs_base__status__is_complete(z);
        return z;
    }
}

wuffs_base__status SkWuffsCodec::decodeFrame() {
    while (true) {
        wuffs_base__status z = fDecoder->decode_frame(&fPixelBuffer, fIOBuffer.reader(),
                                                      ((wuffs_base__slice_u8){
                                                          .ptr = fWorkbufPtr.get(),
                                                          .len = fWorkbufLen,
                                                      }),
                                                      NULL);
        if ((z == wuffs_base__suspension__short_read) && fill_buffer(&fIOBuffer, fStream.get())) {
            continue;
        }
        fNeedResetDecoder = !wuffs_base__status__is_complete(z);
        return z;
    }
}
