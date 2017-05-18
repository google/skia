/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#if SK_SUPPORT_GPU

class GrSurfaceContext;
class GrSurfaceProxy;

// Ensure that reading back from 'srcContext' as RGBA 8888 matches 'expectedPixelValues
void test_read_pixels(skiatest::Reporter*,
                      GrSurfaceContext* srcContext, uint32_t expectedPixelValues[],
                      const char* testName);

// See if trying to write RGBA 8888 pixels to 'dstContext' matches matches the
// expectation ('expectedToWork')
void test_write_pixels(skiatest::Reporter*,
                       GrSurfaceContext* srcContext, bool expectedToWork, const char* testName);

// Ensure that the pixels can be copied from 'proxy' to an RGBA 8888 destination (both
// texture-backed and rendertarget-backed).
void test_copy_from_surface(skiatest::Reporter*, GrContext*,
                            GrSurfaceProxy* proxy, uint32_t expectedPixelValues[],
                            bool onlyTestRTConfig, const char* testName);

// Ensure that RGBA 8888 pixels can be copied into 'dstContext'
void test_copy_to_surface(skiatest::Reporter*, GrResourceProvider*,
                          GrSurfaceContext* dstContext, const char* testName);
#endif
