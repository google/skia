/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlBinaryArchive.h"

#include "include/gpu/GrTypes.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

static NSURL* cache_URL() {
    NSArray *paths = [[NSFileManager defaultManager] URLsForDirectory:NSCachesDirectory
                                                            inDomains:NSUserDomainMask];
    NSURL* cachePath = [paths objectAtIndex:0];
    return [cachePath URLByAppendingPathComponent:@"binaryArchive.metallib"];
}

std::unique_ptr<GrMtlBinaryArchive> GrMtlBinaryArchive::Make(id<MTLDevice> device) {
    MTLBinaryArchiveDescriptor* desc = [MTLBinaryArchiveDescriptor new];
    desc.url = cache_URL(); // try to load
    NSError* error;
    id<MTLBinaryArchive> archive = [device newBinaryArchiveWithDescriptor:desc error:&error];
    if (!archive) {
        desc.url = nil; // create new
        NSError* error;
        archive = [device newBinaryArchiveWithDescriptor:desc error:&error];
        if (!archive) {
            SkDebugf("Error creating MTLBinaryArchive:\n%s\n",
                     error.debugDescription.UTF8String);
            return nil;
        }
    }

    return std::unique_ptr<GrMtlBinaryArchive>(new GrMtlBinaryArchive(archive));
}

bool GrMtlBinaryArchive::store() {
    NSError* error;
    [fBinaryArchive serializeToURL:cache_URL() error:&error];
    if (error) {
        SkDebugf("Error storing MTLBinaryArchive:\n%s\n",
                 error.debugDescription.UTF8String);
        return false;
    }

    return true;
}

