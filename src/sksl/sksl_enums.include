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

)"
