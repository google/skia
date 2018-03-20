/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrNXTResourceProvider_DEFINED
#define GrNXTResourceProvider_DEFINED

#include "GrResourceHandle.h"

class GrNXTGpu;

class GrNXTResourceProvider {
public:
    GrNXTResourceProvider(GrNXTGpu* gpu);
    ~GrNXTResourceProvider();

    void init();

private:
//    GrNXTGpu* fGpu;
};

#endif
