/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkWuffsGifCodec.h"

// TODO(nigeltao): use a swizzler instead of load_u32le and store_u32le.

static inline uint32_t load_u32le(uint8_t* p) {
  return ((uint32_t)(p[0]) << 0) | ((uint32_t)(p[1]) << 8) |
         ((uint32_t)(p[2]) << 16) | ((uint32_t)(p[3]) << 24);
}

static inline void store_u32le(uint8_t* p, uint32_t x) {
  p[0] = x >> 0;
  p[1] = x >> 8;
  p[2] = x >> 16;
  p[3] = x >> 24;
}

#define GIF87_STAMP "GIF87a"
#define GIF89_STAMP "GIF89a"
#define GIF_STAMP_LEN 6

bool SkWuffsGifCodec::IsGif(const void* buf, size_t bytesRead) {
  return (bytesRead >= GIF_STAMP_LEN) &&
         (memcmp(GIF87_STAMP, buf, GIF_STAMP_LEN) == 0 ||
          memcmp(GIF89_STAMP, buf, GIF_STAMP_LEN) == 0);
}

static bool fill_buffer(wuffs_base__io_buffer* b, SkStream* s) {
  // Compact the buffer, moving any written-but-unread bytes to the front.
  //
  // TODO(nigeltao): the Wuffs library should provide a function for this.
  size_t num_compacted = b->wi - b->ri;
  if (num_compacted) {
    memmove(b->ptr, b->ptr + b->ri, num_compacted);
  }
  b->ri = 0;
  b->wi = num_compacted;

  size_t num_read = s->read(b->ptr + b->wi, b->len - b->wi);
  b->wi += num_read;
  // TODO(nigeltao): false should be s->isAtEnd(), after changing Wuffs so that
  // "unexpected EOF" is a non-fatal error (a suspension) instead of a fatal
  // error. Or, change Wuffs to eliminate "unexpected EOF" as a concept.
  b->closed = false;
  return num_read > 0;
}

// TODO(nigeltao): figure out why printf(etc, sizeof *dummy) says 22880 in this
// file but the equivalent expression (the sizeof the wuffs_gif__decoder
// struct) says 22872 when compiling wuffs-v0.2.c. Is it some C/C++ compiler
// switch? Alignment?? Padding?? A missing "extern C" somewhere??
//
// The "sizeof_decoder" references throughout this file should really be
// "sizeof *decoder" or something similar.
static const size_t sizeof_decoder = 22872;
// wuffs_gif__decoder* dummy = nullptr;
// printf("got %d, want %d\n", (int)(sizeof *dummy), (int)(sizeof_decoder));

std::unique_ptr<SkCodec> SkWuffsGifCodec::MakeFromStream(
    std::unique_ptr<SkStream> stream,
    Result* result) {
  wuffs_gif__decoder* decoder = nullptr;

  // TODO(nigeltao): C++ style new/delete instead of malloc/free?
  decoder = static_cast<wuffs_gif__decoder*>(malloc(sizeof_decoder));
  if (!decoder) {
    *result = SkCodec::kInternalError;
    return nullptr;
  }
  // TODO(nigeltao): C++ style RAII for free'ing decoder if we don't return
  // success.

  memset(decoder, 0, sizeof_decoder);
  wuffs_gif__decoder__check_wuffs_version(decoder, sizeof_decoder,
                                          WUFFS_VERSION);

  wuffs_base__image_config imgcfg = ((wuffs_base__image_config){});

  uint8_t buffer[SK_WUFFS_GIF_CODEC_BUFFER_SIZE];
  wuffs_base__io_buffer iobuf = ((wuffs_base__io_buffer){});
  iobuf.ptr = buffer;
  iobuf.len = SK_WUFFS_GIF_CODEC_BUFFER_SIZE;
  while (1) {
    wuffs_base__status z = wuffs_gif__decoder__decode_config(
        decoder, &imgcfg, wuffs_base__io_buffer__reader(&iobuf));
    if (z == WUFFS_BASE__STATUS_OK) {
      break;
    } else if (z == WUFFS_BASE__SUSPENSION_END_OF_DATA) {
      // TODO(nigeltao): is it an error if the GIF image has no frames??
      break;
    } else if (z == WUFFS_BASE__SUSPENSION_SHORT_READ) {
      if (fill_buffer(&iobuf, stream.get())) {
        continue;
      }
      *result = SkCodec::kIncompleteInput;
    } else {
      // TODO(nigeltao): translate z (the Wuffs error code) to a Skia error
      // code. When debugging, a human-readable name for the z number is:
      //
      // printf("wuffs gif status: %s\n", wuffs_gif__status__string(z));
      *result = SkCodec::kInternalError;
    }
    free(decoder);
    return nullptr;
  }

  if (!wuffs_base__image_config__is_valid(&imgcfg)) {
    *result = SkCodec::kInternalError;
    free(decoder);
    return nullptr;
  }

  size_t pixbuf_len = wuffs_base__image_config__pixbuf_size(&imgcfg);
  uint8_t* pixbuf_ptr = static_cast<uint8_t*>(malloc(pixbuf_len));
  if (!pixbuf_ptr) {
    *result = SkCodec::kInternalError;
    free(decoder);
    return nullptr;
  }
  wuffs_base__image_buffer imgbuf = ((wuffs_base__image_buffer){});
  wuffs_base__status z =
      wuffs_base__image_buffer__set_from_slice(&imgbuf, imgcfg,
                                               ((wuffs_base__slice_u8){
                                                   .ptr = pixbuf_ptr,
                                                   .len = pixbuf_len,
                                               }));
  if (z != WUFFS_BASE__STATUS_OK) {
    *result = SkCodec::kInternalError;
    free(pixbuf_ptr);
    free(decoder);
    return nullptr;
  }

  bool ffio = wuffs_base__image_config__first_frame_is_opaque(&imgcfg);

  const auto alpha =
      ffio ? SkEncodedInfo::kOpaque_Alpha : SkEncodedInfo::kBinary_Alpha;

  const auto encodedInfo =
      SkEncodedInfo::Make(SkEncodedInfo::kPalette_Color, alpha, 8);

  const auto alphaType = ffio ? kOpaque_SkAlphaType : kUnpremul_SkAlphaType;

  const auto imageInfo = SkImageInfo::Make(
      wuffs_base__image_config__width(&imgcfg),
      wuffs_base__image_config__height(&imgcfg),
      // TODO(nigeltao): check that kBGRA_8888_SkColorType is the correct Skia
      // constant (should it be kN32_SkColorType??). Wuffs uses (B, G, R, A) in
      // memory order, regardless of endianness.
      //
      // When manually testing this code on linux / x86_64,
      // kBGRA_8888_SkColorType and kN32_SkColorType happen to be the same, but
      // that isn't true on all platform / arches.
      kBGRA_8888_SkColorType, alphaType, SkColorSpace::MakeSRGB());

  *result = kSuccess;
  return std::unique_ptr<SkCodec>(new SkWuffsGifCodec(
      encodedInfo, imageInfo, std::move(stream), decoder, imgbuf, iobuf));
}

SkWuffsGifCodec::SkWuffsGifCodec(const SkEncodedInfo& encodedInfo,
                                 const SkImageInfo& imageInfo,
                                 std::unique_ptr<SkStream> stream,
                                 wuffs_gif__decoder* dec,
                                 wuffs_base__image_buffer imgbuf,
                                 wuffs_base__io_buffer iobuf)
    : INHERITED(encodedInfo,
                imageInfo,
                // TODO(nigeltao): is kRGBA_8888_ColorFormat correct?? For
                // example, is there a kBGRA_etc constant??
                SkColorSpaceXform::kRGBA_8888_ColorFormat,
                std::move(stream)),
      fDecoder(dec),
      fImageBuffer(imgbuf),
      fIOBuffer((wuffs_base__io_buffer){}),
      fIncrDecDst(nullptr),
      fIncrDecRowBytes(0) {
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

SkWuffsGifCodec::~SkWuffsGifCodec() {
  if (fDecoder) {
    free(fDecoder);
  }
  uint8_t* pixbuf = wuffs_base__image_buffer__plane(&fImageBuffer, 0).ptr;
  if (pixbuf) {
    free(pixbuf);
  }
}

SkEncodedImageFormat SkWuffsGifCodec::onGetEncodedFormat() const {
  return SkEncodedImageFormat::kGIF;
}

bool SkWuffsGifCodec::onRewind() {
  // Reset fIOBuffer, dropping any read-but-as-yet-unprocessed bytes.
  fIOBuffer.wi = 0;
  fIOBuffer.ri = 0;
  fIOBuffer.closed = false;

  // Reset fDecoder.
  memset(fDecoder, 0, sizeof_decoder);
  wuffs_gif__decoder__check_wuffs_version(fDecoder, sizeof_decoder,
                                          WUFFS_VERSION);

  // Read the GIF header again, to be ready to call
  // wuffs_gif__decoder__decode_frame during onGetPixels.
  //
  // TODO(nigeltao): change Wuffs so that this isn't necessary? This would let
  // us go straight from a freshly reset wuffs_gif__decoder to calling
  // wuffs_gif__decoder__decode_frame without calling
  // wuffs_gif__decoder__decode_config.
  wuffs_base__image_config imgcfg = ((wuffs_base__image_config){});
  while (1) {
    wuffs_base__status z = wuffs_gif__decoder__decode_config(
        fDecoder, &imgcfg, wuffs_base__io_buffer__reader(&fIOBuffer));
    if (z == WUFFS_BASE__STATUS_OK) {
      break;
    } else if (z == WUFFS_BASE__SUSPENSION_END_OF_DATA) {
      // TODO(nigeltao): is it an error if the GIF image has no frames??
      break;
    } else if (z == WUFFS_BASE__SUSPENSION_SHORT_READ) {
      if (fill_buffer(&fIOBuffer, stream())) {
        continue;
      }
    }
    return false;
  }
  return true;
}

SkCodec::Result SkWuffsGifCodec::onGetPixels(const SkImageInfo& dstInfo,
                                             void* dst,
                                             size_t rowBytes,
                                             const Options& options,
                                             int* rowsDecoded) {
  SkCodec::Result result =
      onStartIncrementalDecode(dstInfo, dst, rowBytes, options);
  if (result != kSuccess) {
    return result;
  }
  return onIncrementalDecode(rowsDecoded);
}

SkCodec::Result SkWuffsGifCodec::onStartIncrementalDecode(
    const SkImageInfo& dstInfo,
    void* dst,
    size_t rowBytes,
    const SkCodec::Options& options) {
  if (options.fSubset) {
    return kUnimplemented;
  }

  // TODO(nigeltao): again, is kN32_SkColorType more correct??
  if (dstInfo.colorType() != kBGRA_8888_SkColorType) {
    return kUnimplemented;
  }

  fIncrDecDst = static_cast<uint8_t*>(dst);
  fIncrDecRowBytes = rowBytes;

  return kSuccess;
}

SkCodec::Result SkWuffsGifCodec::onIncrementalDecode(int* rowsDecoded) {
  if (!fIncrDecDst) {
    return kInternalError;
  }

  // In general, Wuffs image decoders can require a work buffer. For the Wuffs
  // GIF decoder, the work buffer can always be empty.
  wuffs_base__slice_u8 work_buffer = ((wuffs_base__slice_u8){});

  SkCodec::Result result = SkCodec::kSuccess;
  while (1) {
    wuffs_base__status z = wuffs_gif__decoder__decode_frame(
        fDecoder, &fImageBuffer, wuffs_base__io_buffer__reader(&fIOBuffer),
        work_buffer);
    if (z == WUFFS_BASE__STATUS_OK) {
      break;
    } else if (z == WUFFS_BASE__SUSPENSION_END_OF_DATA) {
      // TODO(nigeltao): there were no more frames to decode. We've reached the
      // end of the animation (other than looping back to the start). How do we
      // report this in Skia's API?
      return SkCodec::kIncompleteInput;
    } else if (z == WUFFS_BASE__SUSPENSION_SHORT_READ) {
      if (fill_buffer(&fIOBuffer, stream())) {
        continue;
      }
      // The input is truncated. We set result to kIncompleteInput, but we
      // don't return it straight away. We first decode as much as we can.
      //
      // TODO(nigeltao): do we need to zero-initialize the dst buffer outside
      // of the pixels we successfully decoded?? Does that depend on the
      // options.fZeroInitialized value passed to onStartIncrementalDecode?
      result = SkCodec::kIncompleteInput;
      break;
    }
    // TODO(nigeltao): translate z (the Wuffs error code) to a Skia error code.
    return SkCodec::kInternalError;
  }

  wuffs_base__slice_u8 palette =
      wuffs_base__image_buffer__palette(&fImageBuffer);
  SkASSERT(palette.len == 4 * 256);

  wuffs_base__table_u8 pixels =
      wuffs_base__image_buffer__plane(&fImageBuffer, 0);

  // TODO(nigeltao): wuffs_base__image_buffer__bounds, i.e. s/config/buffer/.
  wuffs_base__rect_ie_u32 r = wuffs_base__image_config__bounds(
      wuffs_base__image_buffer__image_config(&fImageBuffer));
  uint32_t width = wuffs_base__rect_ie_u32__width(r);
  uint32_t height = wuffs_base__rect_ie_u32__height(r);

  // TODO(nigeltao): should this be the *frame* rectangle, not the *image*
  // rectangle? For multi-framed images (i.e. animations), the two concepts can
  // differ. Should the loop over y/x below start at at the frame's top-left or
  // the image's top-left? The image's top-left is always (0, 0). The answer
  // depends on the semantics of the dst pointer argument given to
  // onStartIncrementalDecode.
  //
  // In terms of Wuffs, the frame rectangle is readily available, via
  // wuffs_base__image_buffer__dirty_rect. For example:
  if (0) {
    printf("wuffs image rect: (%d, %d) - (%d, %d)\n", ((int)(r.min_incl_x)),
           ((int)(r.min_incl_y)), ((int)(r.max_excl_x)), ((int)(r.max_excl_y)));

    r = wuffs_base__image_buffer__dirty_rect(&fImageBuffer);
    width = wuffs_base__rect_ie_u32__width(r);
    height = wuffs_base__rect_ie_u32__height(r);

    printf("wuffs frame rect: (%d, %d) - (%d, %d)\n", ((int)(r.min_incl_x)),
           ((int)(r.min_incl_y)), ((int)(r.max_excl_x)), ((int)(r.max_excl_y)));
  }

  // TODO(nigeltao): use a swizzler, once I figure out how it works. For now, a
  // C style load/store loop gets the job done.
  //
  // I'm guessing that a swizzler gets us e.g. kRGB_565_SkColorType support.
  uint8_t* dst = fIncrDecDst;
  // TODO(nigeltao): is "y = 0" and "x = 0" correct? See above.
  for (uint32_t y = 0; y < height; y++) {
    uint8_t* d = dst;
    uint8_t* s = pixels.ptr + y * pixels.stride;
    for (uint32_t x = 0; x < width; x++) {
      uint8_t index = *s++;
      store_u32le(d, load_u32le(palette.ptr + 4 * static_cast<size_t>(index)));
      d += 4;
    }
    dst = SkTAddOffset<uint8_t>(dst, fIncrDecRowBytes);
  }

  if (rowsDecoded) {
    if (height > INT_MAX) {
      // TODO(nigeltao): deal with the overflow in a better way than capping it
      // at INT_MAX? For GIF, the max height is 65535, which won't overflow.
      // But if we generalize this Wuffs SkCodec implementation to handle other
      // file formats (e.g. use the Wuffs PNG implementation), we might have to
      // care about overflow on a 4 billion pixel high image, which the PNG
      // file format allows.
      //
      // There might be other overflow concerns if the *rowsDecoded value
      // should be incremented each time a row is touched, rather than each
      // time it is finished. For interlaced PNGs, a row can be decoded to
      // multiple intermediate, low-resolution values before the final pass.
      *rowsDecoded = INT_MAX;
    } else {
      *rowsDecoded = static_cast<int>(height);
    }
  }

  fIncrDecDst = nullptr;
  fIncrDecRowBytes = 0;
  return result;
}

int SkWuffsGifCodec::onGetFrameCount() {
  // TODO(nigeltao): implement.
  //
  // TODO(nigeltao): also, wuffs_gif__decoder__frame_count returns a uint64_t.
  // Deal with potential uint64_t to int overflow.
  return 1;
}

bool SkWuffsGifCodec::onGetFrameInfo(int i,
                                     SkCodec::FrameInfo* frameInfo) const {
  if (i < 0) {
    return false;
  }

  uint64_t fc = wuffs_gif__decoder__frame_count(fDecoder);
  if ((fc > 0) && (static_cast<uint64_t>(i) < fc - 1)) {
    // TODO(nigeltao): we need to rewind the stream to the start (i.e. call
    // onRewind), and step forward again to get to frame i.
    //
    // But how is this (reading from the stream) supposed to work if this
    // method is const???
  }

  if (frameInfo) {
    // The Wuffs C API doesn't use "const" as often as Skia does.
    wuffs_base__image_buffer* ib =
        const_cast<wuffs_base__image_buffer*>(&fImageBuffer);

    frameInfo->fDuration = wuffs_base__image_buffer__duration(ib) /
                           WUFFS_BASE__FLICKS_PER_MILLISECOND;

    frameInfo->fRequiredFrame = 0;                  // TODO(nigeltao): ???
    frameInfo->fFullyReceived = true;               // TODO(nigeltao): ???
    frameInfo->fAlphaType = kUnpremul_SkAlphaType;  // TODO(nigeltao): ???

    switch (wuffs_base__image_buffer__disposal(ib)) {
      case WUFFS_BASE__ANIMATION_DISPOSAL__RESTORE_BACKGROUND:
        frameInfo->fDisposalMethod =
            SkCodecAnimation::DisposalMethod::kRestoreBGColor;
        break;
      case WUFFS_BASE__ANIMATION_DISPOSAL__RESTORE_PREVIOUS:
        frameInfo->fDisposalMethod =
            SkCodecAnimation::DisposalMethod::kRestorePrevious;
        break;
      default:
        frameInfo->fDisposalMethod = SkCodecAnimation::DisposalMethod::kKeep;
        break;
    }
  }

  return true;
}

int SkWuffsGifCodec::onGetRepetitionCount() {
  // TODO(nigeltao): deal with potential uint32_t to int overflow. Again, this
  // isn't a problem for GIF per se, but it will be a problem if we copy/paste
  // this code for APNG.
  return wuffs_base__image_config__num_loops(
      wuffs_base__image_buffer__image_config(&fImageBuffer));
}
