// Copyright 2015 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
//  AnimDecoder implementation.
//

#ifdef HAVE_CONFIG_H
#include "../webp/config.h"
#endif

#include <assert.h>
#include <string.h>

#include "../utils/utils.h"
#include "../webp/decode.h"
#include "../webp/demux.h"

#define NUM_CHANNELS 4

struct WebPAnimDecoder {
  WebPDemuxer* demux_;             // Demuxer created from given WebP bitstream.
  WebPAnimInfo info_;              // Global info about the animation.
  uint8_t* curr_frame_;            // Current canvas (not disposed).
  uint8_t* prev_frame_disposed_;   // Previous canvas (properly disposed).
  int prev_frame_timestamp_;       // Previous frame timestamp (milliseconds).
  WebPIterator prev_iter_;         // Iterator object for previous frame.
  int prev_frame_was_keyframe_;    // True if previous frame was a keyframe.
  int next_frame_;                 // Index of the next frame to be decoded
                                   // (starting from 1).
};

WebPAnimDecoder* WebPAnimDecoderNewInternal(const WebPData* webp_data,
                                            int abi_version) {
  WebPAnimDecoder* dec = NULL;
  if (webp_data == NULL ||
      WEBP_ABI_IS_INCOMPATIBLE(abi_version, WEBP_DEMUX_ABI_VERSION)) {
    return NULL;
  }

  // Note: calloc() so that the pointer members are initialized to NULL.
  dec = (WebPAnimDecoder*)WebPSafeCalloc(1ULL, sizeof(*dec));
  if (dec == NULL) goto Error;

  dec->demux_ = WebPDemux(webp_data);
  if (dec->demux_ == NULL) goto Error;

  dec->info_.canvas_width = WebPDemuxGetI(dec->demux_, WEBP_FF_CANVAS_WIDTH);
  dec->info_.canvas_height = WebPDemuxGetI(dec->demux_, WEBP_FF_CANVAS_HEIGHT);
  dec->info_.loop_count = WebPDemuxGetI(dec->demux_, WEBP_FF_LOOP_COUNT);
  dec->info_.bgcolor = WebPDemuxGetI(dec->demux_, WEBP_FF_BACKGROUND_COLOR);
  dec->info_.frame_count = WebPDemuxGetI(dec->demux_, WEBP_FF_FRAME_COUNT);

  {
    const int canvas_bytes =
        dec->info_.canvas_width * NUM_CHANNELS * dec->info_.canvas_height;
    // Note: calloc() because we fill frame with zeroes as well.
    dec->curr_frame_ = WebPSafeCalloc(1ULL, canvas_bytes);
    if (dec->curr_frame_ == NULL) goto Error;
    dec->prev_frame_disposed_ = WebPSafeCalloc(1ULL, canvas_bytes);
    if (dec->prev_frame_disposed_ == NULL) goto Error;
  }

  WebPAnimDecoderReset(dec);

  return dec;

 Error:
  WebPAnimDecoderDelete(dec);
  return NULL;
}

int WebPAnimDecoderGetInfo(const WebPAnimDecoder* dec, WebPAnimInfo* info) {
  if (dec == NULL || info == NULL) return 0;
  *info = dec->info_;
  return 1;
}

// Returns true if the frame covers the full canvas.
static int IsFullFrame(int width, int height, int canvas_width,
                       int canvas_height) {
  return (width == canvas_width && height == canvas_height);
}

// Clear the canvas to transparent.
static void ZeroFillCanvas(uint8_t* rgba, uint32_t canvas_width,
                           uint32_t canvas_height) {
  memset(rgba, 0, canvas_width * NUM_CHANNELS * canvas_height);
}

// Clear given frame rectangle to transparent.
static void ZeroFillFrameRect(uint8_t* rgba, int rgba_stride, int x_offset,
                              int y_offset, int width, int height) {
  int j;
  assert(width * NUM_CHANNELS <= rgba_stride);
  rgba += y_offset * rgba_stride + x_offset * NUM_CHANNELS;
  for (j = 0; j < height; ++j) {
    memset(rgba, 0, width * NUM_CHANNELS);
    rgba += rgba_stride;
  }
}

// Copy width * height pixels from 'src' to 'dst'.
static void CopyCanvas(const uint8_t* src, uint8_t* dst,
                       uint32_t width, uint32_t height) {
  assert(src != NULL && dst != NULL);
  memcpy(dst, src, width * NUM_CHANNELS * height);
}

// Returns true if the current frame is a key-frame.
static int IsKeyFrame(const WebPIterator* const curr,
                      const WebPIterator* const prev,
                      int prev_frame_was_key_frame,
                      int canvas_width, int canvas_height) {
  if (curr->frame_num == 1) {
    return 1;
  } else if ((!curr->has_alpha || curr->blend_method == WEBP_MUX_NO_BLEND) &&
             IsFullFrame(curr->width, curr->height,
                         canvas_width, canvas_height)) {
    return 1;
  } else {
    return (prev->dispose_method == WEBP_MUX_DISPOSE_BACKGROUND) &&
           (IsFullFrame(prev->width, prev->height, canvas_width,
                        canvas_height) ||
            prev_frame_was_key_frame);
  }
}


// Blend a single channel of 'src' over 'dst', given their alpha channel values.
static uint8_t BlendChannel(uint32_t src, uint8_t src_a, uint32_t dst,
                            uint8_t dst_a, uint32_t scale, int shift) {
  const uint8_t src_channel = (src >> shift) & 0xff;
  const uint8_t dst_channel = (dst >> shift) & 0xff;
  const uint32_t blend_unscaled = src_channel * src_a + dst_channel * dst_a;
  assert(blend_unscaled < (1ULL << 32) / scale);
  return (blend_unscaled * scale) >> 24;
}

// Blend 'src' over 'dst' assuming they are NOT pre-multiplied by alpha.
static uint32_t BlendPixel(uint32_t src, uint32_t dst) {
  const uint8_t src_a = (src >> 24) & 0xff;

  if (src_a == 0) {
    return dst;
  } else {
    const uint8_t dst_a = (dst >> 24) & 0xff;
    // This is the approximate integer arithmetic for the actual formula:
    // dst_factor_a = (dst_a * (255 - src_a)) / 255.
    const uint8_t dst_factor_a = (dst_a * (256 - src_a)) >> 8;
    const uint8_t blend_a = src_a + dst_factor_a;
    const uint32_t scale = (1UL << 24) / blend_a;

    const uint8_t blend_r =
        BlendChannel(src, src_a, dst, dst_factor_a, scale, 0);
    const uint8_t blend_g =
        BlendChannel(src, src_a, dst, dst_factor_a, scale, 8);
    const uint8_t blend_b =
        BlendChannel(src, src_a, dst, dst_factor_a, scale, 16);
    assert(src_a + dst_factor_a < 256);

    return (blend_r << 0) |
           (blend_g << 8) |
           (blend_b << 16) |
           ((uint32_t)blend_a << 24);
  }
}

// Returns two ranges (<left, width> pairs) at row 'canvas_y', that belong to
// 'src' but not 'dst'. A point range is empty if the corresponding width is 0.
static void FindBlendRangeAtRow(const WebPIterator* const src,
                                const WebPIterator* const dst, int canvas_y,
                                int* const left1, int* const width1,
                                int* const left2, int* const width2) {
  const int src_max_x = src->x_offset + src->width;
  const int dst_max_x = dst->x_offset + dst->width;
  const int dst_max_y = dst->y_offset + dst->height;
  assert(canvas_y >= src->y_offset && canvas_y < (src->y_offset + src->height));
  *left1 = -1;
  *width1 = 0;
  *left2 = -1;
  *width2 = 0;

  if (canvas_y < dst->y_offset || canvas_y >= dst_max_y ||
      src->x_offset >= dst_max_x || src_max_x <= dst->x_offset) {
    *left1 = src->x_offset;
    *width1 = src->width;
    return;
  }

  if (src->x_offset < dst->x_offset) {
    *left1 = src->x_offset;
    *width1 = dst->x_offset - src->x_offset;
  }

  if (src_max_x > dst_max_x) {
    *left2 = dst_max_x;
    *width2 = src_max_x - dst_max_x;
  }
}

// Blend 'num_pixels' in 'src' over 'dst'.
static void BlendPixelRow(uint32_t* const src, const uint32_t* const dst,
                          int num_pixels) {
  int i;
  for (i = 0; i < num_pixels; ++i) {
    uint32_t* const src_pixel_ptr = &src[i];
    const uint8_t src_alpha = (*src_pixel_ptr >> 24) & 0xff;
    if (src_alpha != 0xff) {
      const uint32_t dst_pixel = dst[i];
      *src_pixel_ptr = BlendPixel(*src_pixel_ptr, dst_pixel);
    }
  }
}

int WebPAnimDecoderGetNext(WebPAnimDecoder* dec,
                           uint8_t** rgba_ptr, int* timestamp_ptr) {
  WebPIterator iter;
  uint32_t width;
  uint32_t height;
  int is_key_frame;
  int timestamp;

  if (dec == NULL || rgba_ptr == NULL || timestamp_ptr == NULL) return 0;
  if (!WebPAnimDecoderHasMoreFrames(dec)) return 0;

  width = dec->info_.canvas_width;
  height = dec->info_.canvas_height;

  // Get compressed frame.
  if (!WebPDemuxGetFrame(dec->demux_, dec->next_frame_, &iter)) {
    return 0;
  }
  timestamp = dec->prev_frame_timestamp_ + iter.duration;

  // Initialize.
  is_key_frame = IsKeyFrame(&iter, &dec->prev_iter_,
                            dec->prev_frame_was_keyframe_, width, height);
  if (is_key_frame) {
    ZeroFillCanvas(dec->curr_frame_, width, height);
  } else {
    CopyCanvas(dec->prev_frame_disposed_, dec->curr_frame_, width, height);
  }

  // Decode.
  {
    const uint8_t* input = iter.fragment.bytes;
    const size_t input_size = iter.fragment.size;
    const size_t output_offset =
        (iter.y_offset * width + iter.x_offset) * NUM_CHANNELS;
    uint8_t* output = dec->curr_frame_ + output_offset;
    const int output_stride = NUM_CHANNELS * width;
    const size_t output_size = output_stride * iter.height;

    if (WebPDecodeRGBAInto(input, input_size, output, output_size,
                           output_stride) == NULL) {
      goto Error;
    }
  }

  // During the decoding of current frame, we may have set some pixels to be
  // transparent (i.e. alpha < 255). However, the value of each of these
  // pixels should have been determined by blending it against the value of
  // that pixel in the previous frame if blending method of is WEBP_MUX_BLEND.
  if (iter.frame_num > 1 && iter.blend_method == WEBP_MUX_BLEND &&
      !is_key_frame) {
    if (dec->prev_iter_.dispose_method == WEBP_MUX_DISPOSE_NONE) {
      int y;
      // Blend transparent pixels with pixels in previous canvas.
      for (y = 0; y < iter.height; ++y) {
        const size_t offset =
            (iter.y_offset + y) * width + iter.x_offset;
        BlendPixelRow((uint32_t*)dec->curr_frame_ + offset,
                      (uint32_t*)dec->prev_frame_disposed_ + offset,
                      iter.width);
      }
    } else {
      int y;
      assert(dec->prev_iter_.dispose_method == WEBP_MUX_DISPOSE_BACKGROUND);
      // We need to blend a transparent pixel with its value just after
      // initialization. That is, blend it with:
      // * Fully transparent pixel if it belongs to prevRect <-- No-op.
      // * The pixel in the previous canvas otherwise <-- Need alpha-blending.
      for (y = 0; y < iter.height; ++y) {
        const int canvas_y = iter.y_offset + y;
        int left1, width1, left2, width2;
        FindBlendRangeAtRow(&iter, &dec->prev_iter_, canvas_y, &left1, &width1,
                            &left2, &width2);
        if (width1 > 0) {
          const size_t offset1 = canvas_y * width + left1;
          BlendPixelRow((uint32_t*)dec->curr_frame_ + offset1,
                        (uint32_t*)dec->prev_frame_disposed_ + offset1, width1);
        }
        if (width2 > 0) {
          const size_t offset2 = canvas_y * width + left2;
          BlendPixelRow((uint32_t*)dec->curr_frame_ + offset2,
                        (uint32_t*)dec->prev_frame_disposed_ + offset2, width2);
        }
      }
    }
  }

  // Update info of the previous frame and dispose it for the next iteration.
  dec->prev_frame_timestamp_ = timestamp;
  dec->prev_iter_ = iter;
  dec->prev_frame_was_keyframe_ = is_key_frame;
  CopyCanvas(dec->curr_frame_, dec->prev_frame_disposed_, width, height);
  if (dec->prev_iter_.dispose_method == WEBP_MUX_DISPOSE_BACKGROUND) {
    ZeroFillFrameRect(dec->prev_frame_disposed_, width * NUM_CHANNELS,
                      dec->prev_iter_.x_offset, dec->prev_iter_.y_offset,
                      dec->prev_iter_.width, dec->prev_iter_.height);
  }
  ++dec->next_frame_;

  // All OK, fill in the values.
  *rgba_ptr = dec->curr_frame_;
  *timestamp_ptr = timestamp;
  return 1;

 Error:
  WebPDemuxReleaseIterator(&iter);
  return 0;
}

int WebPAnimDecoderHasMoreFrames(const WebPAnimDecoder* dec) {
  if (dec == NULL) return 0;
  return (dec->next_frame_ <= (int)dec->info_.frame_count);
}

void WebPAnimDecoderReset(WebPAnimDecoder* dec) {
  if (dec != NULL) {
    dec->prev_frame_timestamp_ = 0;
    memset(&dec->prev_iter_, 0, sizeof(dec->prev_iter_));
    dec->prev_frame_was_keyframe_ = 0;
    dec->next_frame_ = 1;
  }
}

void WebPAnimDecoderDelete(WebPAnimDecoder* dec) {
  if (dec != NULL) {
    WebPDemuxDelete(dec->demux_);
    WebPSafeFree(dec->curr_frame_);
    WebPSafeFree(dec->prev_frame_disposed_);
    WebPSafeFree(dec);
  }
}
