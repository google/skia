/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/mtl/GrMtlTypes.h"

#import <CoreFoundation/CoreFoundation.h>

void GrCFResource::assign(const void* resource) {
    if (fCFObject) {
        CFRelease(fCFObject);
    }
    fCFObject = resource;
    if (fCFObject) {
        CFRetain(fCFObject);
    }
}

GrCFResource::~GrCFResource() {
    if (fCFObject) {
        CFRelease(fCFObject);
    }
    fCFObject = nullptr;
}
