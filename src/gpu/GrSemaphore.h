/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSemaphore_DEFINED
#define GrSemaphore_DEFINED

#include "SkRefCnt.h"

class GrGpu;

class GrSemaphore : public SkRefCnt {
protected:
    explicit GrSemaphore(const GrGpu* gpu) : fGpu(gpu) {}

    const GrGpu* fGpu;
};

#endif
