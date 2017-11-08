/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSharedEnums_DEFINED
#define GrSharedEnums_DEFINED

/*************************************************************************************************/
/* This file is used from both C++ and SkSL, so we need to stick to syntax compatible with both. */
/*************************************************************************************************/

/**
 * We have coverage effects that clip rendering to the edge of some geometric primitive.
 * This enum specifies how that clipping is performed. Not all factories that take a
 * GrProcessorEdgeType will succeed with all values and it is up to the caller to check for
 * a NULL return.
 */
enum GrPrimitiveEdgeType {
    kFillBW_GrProcessorEdgeType,
    kFillAA_GrProcessorEdgeType,
    kInverseFillBW_GrProcessorEdgeType,
    kInverseFillAA_GrProcessorEdgeType,
    kHairlineAA_GrProcessorEdgeType,

    kLast_GrProcessorEdgeType = kHairlineAA_GrProcessorEdgeType
};

enum PMConversion {
    kToPremul_PMConversion   = 0,
    kToUnpremul_PMConversion = 1,
    kPMConversionCnt         = 2
};

#endif
