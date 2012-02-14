
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTypes.h"

#ifndef GrGLByteColorConversion_DEFINED
#define GrGLByteColorConversion_DEFINED

class GrGLContextInfo;

enum GrGLByteColorConversion {
    kUnknown_GrGLByteColorConversion,
    kHigh_GrGLByteColorConversion,
    kRound_GrGLByteColorConversion,
    kLow_GrGLByteColorConversion
};

/**
 * When a fragment shader outputs to a RGBA8888 pixel format the floating point
 * result has to be converted to bytes. We would expect that the conversion of
 * floating-point component f to be something like round(f * 255). However,
 * experience shows that is common for the conversion to be biased towards
 * larger than expected byte values, particularly in the low part of the range.
 *
 * This function determines experimentally whether that is true for the current
 * OpenGL context accessible via ctxInfo.
 */
GrGLByteColorConversion
    GrGLFloatToByteColorConversion(const GrGLContextInfo& ctxInfo);

#endif
