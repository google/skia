/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCLImageDiffer_DEFINED
#define SkCLImageDiffer_DEFINED

#if SK_BUILD_FOR_MAC
#   include <OpenCL/cl.h>
#else
#   include <CL/cl.h>
#endif
#include "SkTDArray.h"

#include "SkImageDiffer.h"

class SkStream;

/**
 * An SkImageDiffer that requires initialization with an OpenCL device and context.
 */
class SkCLImageDiffer : public SkImageDiffer {
public:
    SkCLImageDiffer();

    virtual bool requiresOpenCL() const SK_OVERRIDE { return true; }

    /**
     * Initializes the OpenCL resources this differ needs to work
     * @param  device  An OpenCL device
     * @param  context An OpenCL context of the given device
     * @return         True on success, false otherwise
     */
    virtual bool init(cl_device_id device, cl_context context);

protected:
    /**
     * Called by init after fDevice, fContext, and fCommandQueue are successfully initialized
     * @return True on success, false otherwise
     */
    virtual bool onInit() = 0;

    /**
     * Loads an OpenCL kernel from the file with the given named entry point. This only works after
     * init is called.
     * @param  file   The file path of the kernel
     * @param  name   The name of the entry point of the desired kernel in the file
     * @param  kernel A pointer to return the loaded kernel into
     * @return        True on success, false otherwise
     */
    bool loadKernelFile(const char file[], const char name[], cl_kernel* kernel);

    /**
     * Loads an OpenCL kernel from the stream with the given named entry point. This only works
     * after init is called.
     * @param  stream  The stream that contains the kernel
     * @param  name    The name of the entry point of the desired kernel in the stream
     * @param  kernel  A pointer to return the loaded kernel into
     * @return         True on success, false otherwise
     */
    bool loadKernelStream(SkStream* stream, const char name[], cl_kernel* kernel);

    /**
     * Loads an OpenCL kernel from the source string with the given named entry point. This only
     * works after init is called.
     * @param  source  The string that contains the kernel
     * @param  name    The name of the entry point of the desired kernel in the source string
     * @param  kernel  A pointer to return the loaded kernel into
     * @return         True on success, false otherwise
     */
    bool loadKernelSource(const char source[], const char name[], cl_kernel* kernel);

    /**
     * Loads a read only copy of the given bitmap into device memory and returns the block of
     * memory. This only works after init is called.
     * @param  bitmap The bitmap to load into memory
     * @param  image  A pointer to return the allocated image to
     * @return        True on success, false otherwise
     */
    bool makeImage2D(SkBitmap* bitmap, cl_mem* image) const;

    cl_device_id     fDevice;
    cl_context       fContext;
    cl_command_queue fCommandQueue;

protected:
    bool fIsGood;

private:

    typedef SkImageDiffer INHERITED;
};

#endif
