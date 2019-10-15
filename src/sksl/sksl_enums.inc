R"(/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


/*************************************************************************************************/
/* This file is used from both C++ and SkSL, so we need to stick to syntax compatible with both. */
/*************************************************************************************************/

/**
 * We have coverage effects that clip rendering to the edge of some geometric primitive.
 * This enum specifies how that clipping is performed. Not all factories that take a
 * GrProcessorEdgeType will succeed with all values and it is up to the caller to check for
 * a NULL return.
 */
enum class GrClipEdgeType {
    kFillBW,
    kFillAA,
    kInverseFillBW,
    kInverseFillAA,
    kHairlineAA,

    kLast = kHairlineAA
};

enum class PMConversion {
    kToPremul        = 0,
    kToUnpremul      = 1,
    kPMConversionCnt = 2
};

enum class GrColorType {
    kUnknown,
    kAlpha_8,
    kBGR_565,
    kABGR_4444,  // This name differs from SkColorType. kARGB_4444_SkColorType is misnamed.
    kRGBA_8888,
    kRGBA_8888_SRGB,
    kRGB_888x,
    kRG_88,
    kBGRA_8888,
    kRGBA_1010102,
    kGray_8,
    kAlpha_F16,
    kRGBA_F16,
    kRGBA_F16_Clamped,
    kRGBA_F32,

    kAlpha_16,
    kRG_1616,
    kRG_F16,
    kRGBA_16161616,

    // Unusual formats that come up after reading back in cases where we are reassigning the meaning
    // of a texture format's channels to use for a particular color format but have to read back the
    // data to a full RGBA quadruple. (e.g. using a R8 texture format as A8 color type but the API
    // only supports reading to RGBA8.) None of these have SkColorType equivalents.
    kAlpha_8xxx,
    kAlpha_F32xxx,
    kGray_8xxx,

    kLast = kGray_8xxx
};

)"
