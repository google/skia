/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGpuFactory_DEFINED
#define GrGpuFactory_DEFINED

#include "GrTypes.h"

class GrGpu;
class GrContext;
struct GrContextOptions;

typedef GrGpu* (*CreateGpuProc)(GrBackendContext, const GrContextOptions& options, GrContext*);

class GrGpuFactoryRegistrar {
public:
    GrGpuFactoryRegistrar(int i, CreateGpuProc proc);
};

#endif
