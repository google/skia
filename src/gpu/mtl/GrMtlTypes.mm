/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/mtl/GrMtlTypes.h"

#import <CoreFoundation/CoreFoundation.h>

int globalRetainCnt = 0;

GrCFResource::GrCFResource(const void* resource) {
    fCFObject = resource;
    if (fCFObject) {
        ++globalRetainCnt;
        CFRetain(fCFObject);
        SkDebugf("Global retain: %d\n", globalRetainCnt);
    }
}

GrCFResource::GrCFResource(const GrCFResource& that) {
    fCFObject = that.fCFObject;
    if (fCFObject) {
        ++globalRetainCnt;
        CFRetain(fCFObject);
        SkDebugf("Global retain: %d\n", globalRetainCnt);
    }
}

void GrCFResource::assign(const void* resource) {
    SkASSERT(resource != fCFObject);
    if (fCFObject) {
        --globalRetainCnt;
        CFRelease(fCFObject);
        SkDebugf("Global retain: %d\n", globalRetainCnt);
    }
    fCFObject = resource;
    if (fCFObject) {
        ++globalRetainCnt;
        CFRetain(fCFObject);
        SkDebugf("Global retain: %d\n", globalRetainCnt);
    }
}

GrCFResource::~GrCFResource() {
    if (fCFObject) {
        --globalRetainCnt;
        CFRelease(fCFObject);
        SkDebugf("Global retain: %d\n", globalRetainCnt);
    }
    fCFObject = nullptr;
}

const void* GrCFResource::retain() {
    if (fCFObject) {
        ++globalRetainCnt;
        CFRetain(fCFObject);
        SkDebugf("Global retain: %d\n", globalRetainCnt);
    }
    return fCFObject;
}
