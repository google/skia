/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "sk_types_priv.h"

#include "SkImageInfo.h"
#include "SkSurfaceProps.h"
#include "SkPoint.h"
#include "SkRect.h"
#include "SkPoint3.h"
#include "SkSize.h"
#include "SkPaint.h"
#include "SkCodec.h"
#include "SkMask.h"
#include "SkCanvas.h"
#include "SkTime.h"
#include "SkDocument.h"
#include "SkHighContrastFilter.h"
#include "SkTextBlob.h"
#include "SkRSXform.h"

#if SK_SUPPORT_GPU
#include "GrTypes.h"
#include "gl/GrGLTypes.h"
#include "GrContextOptions.h"
#endif

#if __cplusplus >= 199711L

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define ASSERT_MSG(SK, C) "ABI changed, you must update the C structure for " TOSTRING(#SK) " to " TOSTRING(#C) "."

// custom mappings:
//  - sk_matrix_t
//  - sk_document_pdf_metadata_t

static_assert (sizeof (sk_ipoint_t) == sizeof (SkIPoint), ASSERT_MSG(SkIPoint, sk_ipoint_t));
static_assert (sizeof (sk_point_t) == sizeof (SkPoint), ASSERT_MSG(SkPoint, sk_point_t));
static_assert (sizeof (sk_irect_t) == sizeof (SkIRect), ASSERT_MSG(SkIRect, sk_irect_t));
static_assert (sizeof (sk_rect_t) == sizeof (SkRect), ASSERT_MSG(SkRect, sk_rect_t));
static_assert (sizeof (sk_isize_t) == sizeof (SkISize), ASSERT_MSG(SkISize, sk_isize_t));
static_assert (sizeof (sk_size_t) == sizeof (SkSize), ASSERT_MSG(SkSize, sk_size_t));
static_assert (sizeof (sk_point3_t) == sizeof (SkPoint3), ASSERT_MSG(SkPoint3, sk_point3_t));
static_assert (sizeof (sk_imageinfo_t) == sizeof (SkImageInfo), ASSERT_MSG(SkImageInfo, sk_imageinfo_t));
static_assert (sizeof (sk_fontmetrics_t) == sizeof (SkPaint::FontMetrics), ASSERT_MSG(SkPaint::FontMetrics, sk_fontmetrics_t));
static_assert (sizeof (sk_codec_options_t) == sizeof (SkCodec::Options), ASSERT_MSG(SkCodec::Options, sk_codec_options_t));
static_assert (sizeof (sk_mask_t) == sizeof (SkMask), ASSERT_MSG(SkMask, sk_mask_t));
static_assert (sizeof (sk_lattice_t) == sizeof (SkCanvas::Lattice), ASSERT_MSG(SkCanvas::Lattice, sk_lattice_t));
static_assert (sizeof (sk_time_datetime_t) == sizeof (SkTime::DateTime), ASSERT_MSG(SkTime::DateTime, sk_time_datetime_t));
static_assert (sizeof (sk_codec_frameinfo_t) == sizeof (SkCodec::FrameInfo), ASSERT_MSG(SkCodec::FrameInfo, sk_codec_frameinfo_t));
static_assert (sizeof (sk_colorspace_transfer_fn_t) == sizeof (SkColorSpaceTransferFn), ASSERT_MSG(SkColorSpaceTransferFn, sk_colorspace_transfer_fn_t));
static_assert (sizeof (sk_colorspaceprimaries_t) == sizeof (SkColorSpacePrimaries), ASSERT_MSG(SkColorSpacePrimaries, sk_colorspaceprimaries_t));
static_assert (sizeof (sk_highcontrastconfig_t) == sizeof (SkHighContrastConfig), ASSERT_MSG(SkHighContrastConfig, sk_highcontrastconfig_t));
static_assert (sizeof (sk_pngencoder_options_t) == sizeof (SkPngEncoder::Options), ASSERT_MSG(SkPngEncoder::Options, sk_pngencoder_options_t));
static_assert (sizeof (sk_jpegencoder_options_t) == sizeof (SkJpegEncoder::Options), ASSERT_MSG(SkJpegEncoder::Options, sk_jpegencoder_options_t));
static_assert (sizeof (sk_webpencoder_options_t) == sizeof (SkWebpEncoder::Options), ASSERT_MSG(SkWebpEncoder::Options, sk_webpencoder_options_t));
static_assert (sizeof (sk_textblob_builder_runbuffer_t) == sizeof (SkTextBlobBuilder::RunBuffer), ASSERT_MSG(SkTextBlobBuilder::RunBuffer, sk_textblob_builder_runbuffer_t));
static_assert (sizeof (sk_rsxform_t) == sizeof (SkRSXform), ASSERT_MSG(SkRSXform, sk_rsxform_t));
static_assert (sizeof (sk_color4f_t) == sizeof (SkColor4f), ASSERT_MSG(SkColor4f, sk_color4f_t));

#if SK_SUPPORT_GPU
static_assert (sizeof (gr_gl_framebufferinfo_t) == sizeof (GrGLFramebufferInfo), ASSERT_MSG(GrGLFramebufferInfo, gr_gl_framebufferinfo_t));
static_assert (sizeof (gr_gl_textureinfo_t) == sizeof (GrGLTextureInfo), ASSERT_MSG(GrGLTextureInfo, gr_gl_textureinfo_t));
#endif

#endif
