/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrDawnResourceProvider_DEFINED
#define GrDawnResourceProvider_DEFINED

#include "GrResourceHandle.h"

class GrDawnGpu;

class GrDawnResourceProvider {
public:
    GrDawnResourceProvider(GrDawnGpu* gpu);
    ~GrDawnResourceProvider();

    void init();

private:
//    GrDawnGpu* fGpu;
};

#endif
