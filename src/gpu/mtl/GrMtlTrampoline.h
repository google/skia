/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlTrampoline_DEFINED
#define GrMtlTrampoline_DEFINED

#include "GrTypes.h"

class GrContext;
class GrGpu;
struct GrContextOptions;

/*
 * This class is used to hold functions which trampoline from the Ganesh cpp code to the GrMtl
 * objective-c files.
 */
class GrMtlTrampoline {
public:
    static GrGpu* CreateGpu(GrContext* context,
                            const GrContextOptions& options,
                            void* device,
                            void* queue);
};

#endif

