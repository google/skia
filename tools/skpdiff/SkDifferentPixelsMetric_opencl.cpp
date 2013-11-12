/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"

#include "SkDifferentPixelsMetric.h"
#include "skpdiff_util.h"

static const char kDifferentPixelsKernelSource[] =
    "#pragma OPENCL_EXTENSION cl_khr_global_int32_base_atomics                  \n"
    "                                                                           \n"
    "const sampler_t gInSampler = CLK_NORMALIZED_COORDS_FALSE |                 \n"
    "                             CLK_ADDRESS_CLAMP_TO_EDGE   |                 \n"
    "                             CLK_FILTER_NEAREST;                           \n"
    "                                                                           \n"
    "__kernel void diff(read_only image2d_t baseline, read_only image2d_t test, \n"
    "                   __global int* result) {                                 \n"
    "    int2 coord = (int2)(get_global_id(0), get_global_id(1));               \n"
    "    uint4 baselinePixel = read_imageui(baseline, gInSampler, coord);       \n"
    "    uint4 testPixel = read_imageui(test, gInSampler, coord);               \n"
    "    if (baselinePixel.x != testPixel.x ||                                  \n"
    "        baselinePixel.y != testPixel.y ||                                  \n"
    "        baselinePixel.z != testPixel.z ||                                  \n"
    "        baselinePixel.w != testPixel.w) {                                  \n"
    "                                                                           \n"
    "        atomic_inc(result);                                                \n"
    "        // TODO: generate alpha mask                                       \n"
    "    }                                                                      \n"
    "}                                                                          \n";

const char* SkDifferentPixelsMetric::getName() const {
    return "different_pixels";
}

bool SkDifferentPixelsMetric::diff(SkBitmap* baseline, SkBitmap* test, bool computeMask,
                                   Result* result) const {
    double startTime = get_seconds();

    if (!fIsGood) {
        return false;
    }

    // If we never end up running the kernel, include some safe defaults in the result.
    result->poiCount = 0;

    // Ensure the images are comparable
    if (baseline->width() != test->width() || baseline->height() != test->height() ||
        baseline->width() <= 0 || baseline->height() <= 0 ||
        baseline->config() != test->config()) {
        return false;
    }

    cl_mem baselineImage;
    cl_mem testImage;
    cl_mem resultsBuffer;

    // Upload images to the CL device
    if (!this->makeImage2D(baseline, &baselineImage) || !this->makeImage2D(test, &testImage)) {
        SkDebugf("creation of openCL images failed");
        return false;
    }

    // A small hack that makes calculating percentage difference easier later on.
    result->result = 1.0 / ((double)baseline->width() * baseline->height());

    // Make a buffer to store results into. It must be initialized with pointers to memory.
    static const int kZero = 0;
    // We know OpenCL won't write to it because we use CL_MEM_COPY_HOST_PTR
    resultsBuffer = clCreateBuffer(fContext, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                   sizeof(int), (int*)&kZero, NULL);

    // Set all kernel arguments
    cl_int setArgErr = clSetKernelArg(fKernel, 0, sizeof(cl_mem), &baselineImage);
    setArgErr       |= clSetKernelArg(fKernel, 1, sizeof(cl_mem), &testImage);
    setArgErr       |= clSetKernelArg(fKernel, 2, sizeof(cl_mem), &resultsBuffer);
    if (CL_SUCCESS != setArgErr) {
        SkDebugf("Set arg failed: %s\n", cl_error_to_string(setArgErr));
        return false;
    }

    // Queue this diff on the CL device
    cl_event event;
    const size_t workSize[] = { baseline->width(), baseline->height() };
    cl_int enqueueErr;
    enqueueErr = clEnqueueNDRangeKernel(fCommandQueue, fKernel, 2, NULL, workSize,
                                        NULL, 0, NULL, &event);
    if (CL_SUCCESS != enqueueErr) {
        SkDebugf("Enqueue failed: %s\n", cl_error_to_string(enqueueErr));
        return false;
    }

    // This makes things totally synchronous. Actual queue is not ready yet
    clWaitForEvents(1, &event);

    // Immediate read back the results
    clEnqueueReadBuffer(fCommandQueue, resultsBuffer, CL_TRUE, 0,
                        sizeof(int), &result->poiCount, 0, NULL, NULL);
    result->result *= (double)result->poiCount;
    result->result = (1.0 - result->result);

    // Release all the buffers created
    clReleaseMemObject(resultsBuffer);
    clReleaseMemObject(baselineImage);
    clReleaseMemObject(testImage);

    result->timeElapsed = get_seconds() - startTime;
    return true;
}

bool SkDifferentPixelsMetric::onInit() {
    if (!this->loadKernelSource(kDifferentPixelsKernelSource, "diff", &fKernel)) {
        return false;
    }

    return true;
}
