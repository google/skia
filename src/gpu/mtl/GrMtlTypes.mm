/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/mtl/GrMtlTypes.h"

#import <CoreFoundation/CoreFoundation.h>

GrCFResource::GrCFResource(const void* resource) {
    fCFObject = resource;
    if (fCFObject) {
        CFRetain(fCFObject);
    }
}

GrCFResource::GrCFResource(const GrCFResource& that) {
    fCFObject = that.fCFObject;
    if (fCFObject) {
        CFRetain(fCFObject);
    }
}

void GrCFResource::assign(const void* resource) {
    if (resource) {
        CFRetain(resource);
    }
    if (fCFObject) {
        CFRelease(fCFObject);
    }
    fCFObject = resource;
}

GrCFResource::~GrCFResource() {
    if (fCFObject) {
        CFRelease(fCFObject);
    }
    fCFObject = nullptr;
}
