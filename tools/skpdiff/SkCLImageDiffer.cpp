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

bool SkCLImageDiffer::makeImage2D(SkBitmap* bitmap, cl_mem* image) const {
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
