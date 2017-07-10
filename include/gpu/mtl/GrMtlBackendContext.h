/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlBackendContext_h
#define GrMtlBackendContext_h

#include "SkRefCnt.h"

// The GrMtlBackendContext contains the MTLDevice and MTLCommandQueue that we will use for all
// operations. These objects are passed in as intptr_t's and will be cast to their respective
// objects in the Ganesh backend. This allows for this header to be used in cpp files. If a client
// wants to override the default metal functions calls, they should create their own objects which
// implements all the MTLDevice (and other metal object) functions and pass that object is at the
// device here.
struct GrMtlBackendContext : public SkRefCnt {
    intptr_t fDevice; // id <MTLDevice>
    intptr_t fQueue;  // id <MTLCommandQueue>
    
    GrMtlBackendContext() : fDevice(0), fQueue(0) {}
    
    static GrMtlBackendContext* CreateDefault();
};

#endif
