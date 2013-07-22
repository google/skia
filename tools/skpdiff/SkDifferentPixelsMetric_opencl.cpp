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
    "                   __global int* result, __global int2* poi) {             \n"
    "    int2 coord = (int2)(get_global_id(0), get_global_id(1));               \n"
    "    uint4 baselinePixel = read_imageui(baseline, gInSampler, coord);       \n"
    "    uint4 testPixel = read_imageui(test, gInSampler, coord);               \n"
    "    if (baselinePixel.x != testPixel.x ||                                  \n"
    "        baselinePixel.y != testPixel.y ||                                  \n"
    "        baselinePixel.z != testPixel.z ||                                  \n"
    "        baselinePixel.w != testPixel.w) {                                  \n"
    "                                                                           \n"
    "        int poiIndex = atomic_inc(result);                                 \n"
    "        poi[poiIndex] = coord;                                             \n"
    "    }                                                                      \n"
    "}                                                                          \n";

struct SkDifferentPixelsMetric::QueuedDiff {
    bool finished;
    double result;
    int numDiffPixels;
    SkIPoint* poi;
    cl_mem baseline;
    cl_mem test;
    cl_mem resultsBuffer;
    cl_mem poiBuffer;
};

const char* SkDifferentPixelsMetric::getName() {
    return "different_pixels";
}

int SkDifferentPixelsMetric::queueDiff(SkBitmap* baseline, SkBitmap* test) {
    int diffID = fQueuedDiffs.count();
    double startTime = get_seconds();
    QueuedDiff* diff = fQueuedDiffs.push();

    // If we never end up running the kernel, include some safe defaults in the result.
    diff->finished = false;
    diff->result = -1.0;
    diff->numDiffPixels = 0;
    diff->poi = NULL;

    // Ensure the images are comparable
    if (baseline->width() != test->width() || baseline->height() != test->height() ||
        baseline->width() <= 0 || baseline->height() <= 0 ||
        baseline->config() != test->config()) {
        diff->finished = true;
        return diffID;
    }

    // Upload images to the CL device
    if (!this->makeImage2D(baseline, &diff->baseline) || !this->makeImage2D(test, &diff->test)) {
        diff->finished = true;
        fIsGood = false;
        return -1;
    }

    // A small hack that makes calculating percentage difference easier later on.
    diff->result = 1.0 / ((double)baseline->width() * baseline->height());

    // Make a buffer to store results into. It must be initialized with pointers to memory.
    static const int kZero = 0;
    // We know OpenCL won't write to it because we use CL_MEM_COPY_HOST_PTR
    diff->resultsBuffer = clCreateBuffer(fContext, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                         sizeof(int), (int*)&kZero, NULL);

    diff->poiBuffer = clCreateBuffer(fContext, CL_MEM_WRITE_ONLY,
                                     sizeof(int) * 2 * baseline->width() * baseline->height(),
                                     NULL, NULL);

    // Set all kernel arguments
    cl_int setArgErr = clSetKernelArg(fKernel, 0, sizeof(cl_mem), &diff->baseline);
    setArgErr       |= clSetKernelArg(fKernel, 1, sizeof(cl_mem), &diff->test);
    setArgErr       |= clSetKernelArg(fKernel, 2, sizeof(cl_mem), &diff->resultsBuffer);
    setArgErr       |= clSetKernelArg(fKernel, 3, sizeof(cl_mem), &diff->poiBuffer);
    if (CL_SUCCESS != setArgErr) {
        SkDebugf("Set arg failed: %s\n", cl_error_to_string(setArgErr));
        fIsGood = false;
        return -1;
    }

    // Queue this diff on the CL device
    cl_event event;
    const size_t workSize[] = { baseline->width(), baseline->height() };
    cl_int enqueueErr;
    enqueueErr = clEnqueueNDRangeKernel(fCommandQueue, fKernel, 2, NULL, workSize,
                                        NULL, 0, NULL, &event);
    if (CL_SUCCESS != enqueueErr) {
        SkDebugf("Enqueue failed: %s\n", cl_error_to_string(enqueueErr));
        fIsGood = false;
        return -1;
    }

    // This makes things totally synchronous. Actual queue is not ready yet
    clWaitForEvents(1, &event);
    diff->finished = true;

    // Immediate read back the results
    clEnqueueReadBuffer(fCommandQueue, diff->resultsBuffer, CL_TRUE, 0,
                        sizeof(int), &diff->numDiffPixels, 0, NULL, NULL);
    diff->result *= (double)diff->numDiffPixels;
    diff->result = (1.0 - diff->result);

    // Reading a buffer of size zero can cause issues on some (Mac) OpenCL platforms.
    if (diff->numDiffPixels > 0) {
        diff->poi = SkNEW_ARRAY(SkIPoint, diff->numDiffPixels);
        clEnqueueReadBuffer(fCommandQueue, diff->poiBuffer, CL_TRUE, 0,
                        sizeof(SkIPoint) * diff->numDiffPixels, diff->poi, 0, NULL, NULL);
    }

    // Release all the buffers created
    clReleaseMemObject(diff->poiBuffer);
    clReleaseMemObject(diff->resultsBuffer);
    clReleaseMemObject(diff->baseline);
    clReleaseMemObject(diff->test);

    SkDebugf("Time: %f\n", (get_seconds() - startTime));

    return diffID;
}

void SkDifferentPixelsMetric::deleteDiff(int id) {
    QueuedDiff* diff = &fQueuedDiffs[id];
    if (NULL != diff->poi) {
        SkDELETE_ARRAY(diff->poi);
        diff->poi = NULL;
    }
}

bool SkDifferentPixelsMetric::isFinished(int id) {
    return fQueuedDiffs[id].finished;
}

double SkDifferentPixelsMetric::getResult(int id) {
    return fQueuedDiffs[id].result;
}

int SkDifferentPixelsMetric::getPointsOfInterestCount(int id) {
    return fQueuedDiffs[id].numDiffPixels;
}

SkIPoint* SkDifferentPixelsMetric::getPointsOfInterest(int id) {
    return fQueuedDiffs[id].poi;
}

bool SkDifferentPixelsMetric::onInit() {
    if (!this->loadKernelSource(kDifferentPixelsKernelSource, "diff", &fKernel)) {
        return false;
    }

    return true;
}
