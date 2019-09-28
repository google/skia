/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBuffer_DEFINED
#define GrBuffer_DEFINED

#include "include/gpu/GrTypes.h"

/** Base class for a GPU buffer object or a client side arrays. */
class GrBuffer {
public:
    GrBuffer(const GrBuffer&) = delete;
    GrBuffer& operator=(const GrBuffer&) = delete;

    virtual ~GrBuffer() = default;

    // Our subclasses derive from different ref counting base classes. In order to use base
    // class pointers with sk_sp we virtualize ref() and unref().
    virtual void ref() const = 0;
    virtual void unref() const = 0;

    /** Size of the buffer in bytes. */
    virtual size_t size() const = 0;

    /** Is this an instance of GrCpuBuffer? Otherwise, an instance of GrGpuBuffer. */
    virtual bool isCpuBuffer() const = 0;

protected:
    GrBuffer() = default;
};

#endif
