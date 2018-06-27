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
#include "SkEncodedInfo.h"
#include "SkHighContrastFilter.h"

#if SK_SUPPORT_GPU
#include "GrTypes.h"
#include "GrContextOptions.h"
#endif

#if __cplusplus >= 199711L

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define ASSERT_MSG(SK, C) "ABI changed, you must update the C structure for " TOSTRING(#SK) " to " TOSTRING(#C) "."

// custom mappings:
//  - sk_imageinfo_t
//  - sk_surfaceprops_t
//  - sk_matrix_t
//  - sk_document_pdf_metadata_t

static_assert (sizeof (sk_ipoint_t) == sizeof (SkIPoint), ASSERT_MSG(SkIPoint, sk_ipoint_t));
static_assert (sizeof (sk_point_t) == sizeof (SkPoint), ASSERT_MSG(SkPoint, sk_point_t));
static_assert (sizeof (sk_irect_t) == sizeof (SkIRect), ASSERT_MSG(SkIRect, sk_irect_t));
static_assert (sizeof (sk_rect_t) == sizeof (SkRect), ASSERT_MSG(SkRect, sk_rect_t));
static_assert (sizeof (sk_isize_t) == sizeof (SkISize), ASSERT_MSG(SkISize, sk_isize_t));
static_assert (sizeof (sk_size_t) == sizeof (SkSize), ASSERT_MSG(SkSize, sk_size_t));
static_assert (sizeof (sk_point3_t) == sizeof (SkPoint3), ASSERT_MSG(SkPoint3, sk_point3_t));
static_assert (sizeof (sk_fontmetrics_t) == sizeof (SkPaint::FontMetrics), ASSERT_MSG(SkPaint::FontMetrics, sk_fontmetrics_t));
static_assert (sizeof (sk_codec_options_t) == sizeof (SkCodec::Options), ASSERT_MSG(SkCodec::Options, sk_codec_options_t));
static_assert (sizeof (sk_mask_t) == sizeof (SkMask), ASSERT_MSG(SkMask, sk_mask_t));
static_assert (sizeof (sk_lattice_t) == sizeof (SkCanvas::Lattice), ASSERT_MSG(SkCanvas::Lattice, sk_lattice_t));
static_assert (sizeof (sk_time_datetime_t) == sizeof (SkTime::DateTime), ASSERT_MSG(SkTime::DateTime, sk_time_datetime_t));
static_assert (sizeof (sk_encodedinfo_t) == sizeof (SkEncodedInfo), ASSERT_MSG(SkEncodedInfo, sk_encodedinfo_t));
static_assert (sizeof (sk_codec_frameinfo_t) == sizeof (SkCodec::FrameInfo), ASSERT_MSG(SkCodec::FrameInfo, sk_codec_frameinfo_t));
static_assert (sizeof (sk_colorspace_transfer_fn_t) == sizeof (SkColorSpaceTransferFn), ASSERT_MSG(SkColorSpaceTransferFn, sk_colorspace_transfer_fn_t));
static_assert (sizeof (sk_colorspaceprimaries_t) == sizeof (SkColorSpacePrimaries), ASSERT_MSG(SkColorSpacePrimaries, sk_colorspaceprimaries_t));
static_assert (sizeof (sk_highcontrastconfig_t) == sizeof (SkHighContrastConfig), ASSERT_MSG(SkHighContrastConfig, sk_highcontrastconfig_t));
static_assert (sizeof (sk_pngencoder_options_t) == sizeof (SkPngEncoder::Options), ASSERT_MSG(SkPngEncoder::Options, sk_pngencoder_options_t));
static_assert (sizeof (sk_jpegencoder_options_t) == sizeof (SkJpegEncoder::Options), ASSERT_MSG(SkJpegEncoder::Options, sk_jpegencoder_options_t));
static_assert (sizeof (sk_webpencoder_options_t) == sizeof (SkWebpEncoder::Options), ASSERT_MSG(SkWebpEncoder::Options, sk_webpencoder_options_t));

#if SK_SUPPORT_GPU
static_assert (sizeof (gr_backend_rendertarget_desc_t) == sizeof (GrBackendRenderTargetDesc), ASSERT_MSG(GrBackendRenderTargetDesc, gr_backend_rendertarget_desc_t));
static_assert (sizeof (gr_backend_texture_desc_t) == sizeof (GrBackendTextureDesc), ASSERT_MSG(GrBackendTextureDesc, gr_backend_texture_desc_t));
static_assert (sizeof (gr_context_options_t) == sizeof (GrContextOptions), ASSERT_MSG(GrContextOptions, gr_context_options_t));
#endif

#endif
