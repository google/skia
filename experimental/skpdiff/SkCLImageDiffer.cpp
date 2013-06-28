
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cstring>

#include "SkBitmap.h"
#include "SkStream.h"

#include "SkCLImageDiffer.h"
#include "skpdiff_util.h"

SkCLImageDiffer::SkCLImageDiffer() {
    fIsGood = false;
}


bool SkCLImageDiffer::init(cl_device_id device, cl_context context) {
    fContext = context;
    fDevice = device;

    cl_int queueErr;
    fCommandQueue = clCreateCommandQueue(fContext, fDevice, 0, &queueErr);
    if (CL_SUCCESS != queueErr) {
        SkDebugf("Command queue creation failed: %s\n", cl_error_to_string(queueErr));
        fIsGood = false;
        return false;
    }

    fIsGood = this->onInit();
    return fIsGood;
}

bool SkCLImageDiffer::loadKernelFile(const char file[], const char name[], cl_kernel* kernel) {
    // Open the kernel source file
    SkFILEStream sourceStream(file);
    if (!sourceStream.isValid()) {
        SkDebugf("Failed to open kernel source file");
        return false;
    }

    return loadKernelStream(&sourceStream, name, kernel);
}

bool SkCLImageDiffer::loadKernelStream(SkStream* stream, const char name[], cl_kernel* kernel) {
    // Read the kernel source into memory
    SkString sourceString;
    sourceString.resize(stream->getLength());
    size_t bytesRead = stream->read(sourceString.writable_str(), sourceString.size());
    if (bytesRead != sourceString.size()) {
        SkDebugf("Failed to read kernel source file");
        return false;
    }

    return loadKernelSource(sourceString.c_str(), name, kernel);
}

bool SkCLImageDiffer::loadKernelSource(const char source[], const char name[], cl_kernel* kernel) {
    // Build the kernel source
    size_t sourceLen = strlen(source);
    cl_program program = clCreateProgramWithSource(fContext, 1, &source, &sourceLen, NULL);
    cl_int programErr = clBuildProgram(program, 1, &fDevice, "", NULL, NULL);
    if (CL_SUCCESS != programErr) {
        SkDebugf("Program creation failed: %s\n", cl_error_to_string(programErr));

        // Attempt to get information about why the build failed
        char buildLog[4096];
        clGetProgramBuildInfo(program, fDevice, CL_PROGRAM_BUILD_LOG, sizeof(buildLog),
                              buildLog, NULL);
        SkDebugf("Build log: %s\n", buildLog);

        return false;
    }

    cl_int kernelErr;
    *kernel = clCreateKernel(program, name, &kernelErr);
    if (CL_SUCCESS != kernelErr) {
        SkDebugf("Kernel creation failed: %s\n", cl_error_to_string(kernelErr));
        return false;
    }

    return true;
}

bool SkCLImageDiffer::makeImage2D(SkBitmap* bitmap, cl_mem* image) {
    cl_int imageErr;
    cl_image_format bitmapFormat;
    switch (bitmap->config()) {
        case SkBitmap::kA8_Config:
            bitmapFormat.image_channel_order = CL_A;
            bitmapFormat.image_channel_data_type = CL_UNSIGNED_INT8;
            break;
        case SkBitmap::kRGB_565_Config:
            bitmapFormat.image_channel_order = CL_RGB;
            bitmapFormat.image_channel_data_type = CL_UNORM_SHORT_565;
            break;
        case SkBitmap::kARGB_8888_Config:
            bitmapFormat.image_channel_order = CL_RGBA;
            bitmapFormat.image_channel_data_type = CL_UNSIGNED_INT8;
            break;
        default:
            SkDebugf("Image format is unsupported\n");
            return false;
    }

    // Upload the bitmap data to OpenCL
    bitmap->lockPixels();
    *image = clCreateImage2D(fContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                             &bitmapFormat, bitmap->width(), bitmap->height(),
                             bitmap->rowBytes(), bitmap->getPixels(),
                             &imageErr);
    bitmap->unlockPixels();

    if (CL_SUCCESS != imageErr) {
        SkDebugf("Input image creation failed: %s\n", cl_error_to_string(imageErr));
        return false;
    }

    return true;
}


////////////////////////////////////////////////////////////////

struct SkDifferentPixelsImageDiffer::QueuedDiff {
    bool finished;
    double result;
    int numDiffPixels;
    SkIPoint* poi;
    cl_mem baseline;
    cl_mem test;
    cl_mem resultsBuffer;
    cl_mem poiBuffer;
};

const char* SkDifferentPixelsImageDiffer::getName() {
    return "different_pixels";
}

int SkDifferentPixelsImageDiffer::queueDiff(SkBitmap * baseline, SkBitmap * test) {
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
                    baseline->width() <= 0 || baseline->height() <= 0) {
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

    diff->poi = SkNEW_ARRAY(SkIPoint, diff->numDiffPixels);
    clEnqueueReadBuffer(fCommandQueue, diff->poiBuffer, CL_TRUE, 0,
                        sizeof(SkIPoint) * diff->numDiffPixels, diff->poi, 0, NULL, NULL);

    // Release all the buffers created
    clReleaseMemObject(diff->poiBuffer);
    clReleaseMemObject(diff->resultsBuffer);
    clReleaseMemObject(diff->baseline);
    clReleaseMemObject(diff->test);

    SkDebugf("Time: %f\n", (get_seconds() - startTime));

    return diffID;
}

void SkDifferentPixelsImageDiffer::deleteDiff(int id) {
    QueuedDiff* diff = &fQueuedDiffs[id];
    if (NULL != diff->poi) {
        SkDELETE_ARRAY(diff->poi);
        diff->poi = NULL;
    }
}

bool SkDifferentPixelsImageDiffer::isFinished(int id) {
    return fQueuedDiffs[id].finished;
}

double SkDifferentPixelsImageDiffer::getResult(int id) {
    return fQueuedDiffs[id].result;
}

int SkDifferentPixelsImageDiffer::getPointsOfInterestCount(int id) {
    return fQueuedDiffs[id].numDiffPixels;
}

SkIPoint* SkDifferentPixelsImageDiffer::getPointsOfInterest(int id) {
    return fQueuedDiffs[id].poi;
}

bool SkDifferentPixelsImageDiffer::onInit() {
    if (!loadKernelFile("experimental/skpdiff/diff_pixels.cl", "diff", &fKernel)) {
        return false;
    }

    return true;
}
