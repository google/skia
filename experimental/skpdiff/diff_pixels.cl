/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma OPENCL_EXTENSION cl_khr_global_int32_base_atomics

const sampler_t gInSampler = CLK_NORMALIZED_COORDS_FALSE |
                             CLK_ADDRESS_CLAMP_TO_EDGE   |
                             CLK_FILTER_NEAREST;

__kernel void diff(read_only image2d_t baseline, read_only image2d_t test,
                   __global int* result, __global int2* poi) {
    int2 coord = (int2)(get_global_id(0), get_global_id(1));
    uint4 baselinePixel = read_imageui(baseline, gInSampler, coord);
    uint4 testPixel = read_imageui(test, gInSampler, coord);
    int4 pixelCompare = baselinePixel == testPixel;
    if (baselinePixel.x != testPixel.x ||
        baselinePixel.y != testPixel.y ||
        baselinePixel.z != testPixel.z ||
        baselinePixel.w != testPixel.w) {

        int poiIndex = atomic_inc(result);
        poi[poiIndex] = coord;
    }
}