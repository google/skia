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

std::unique_ptr<GrMtlBinaryArchive> GrMtlBinaryArchive::Make(id<MTLDevice> device) {
    NSArray *paths = [[NSFileManager defaultManager] URLsForDirectory:NSCachesDirectory
                                                            inDomains:NSUserDomainMask];
    NSURL* cachePath = [paths objectAtIndex:0];

    MTLBinaryArchiveDescriptor* desc = [MTLBinaryArchiveDescriptor new];
    desc.url = [cachePath URLByAppendingPathComponent:@"binaryArchiveCache"]; // try to load
    NSError* error;
    id<MTLBinaryArchive> archive = [device newBinaryArchiveWithDescriptor:desc error:&error];
    if (!archive) {
        desc.url = nil; // create new
        NSError* error;
        id<MTLBinaryArchive> archive = [device newBinaryArchiveWithDescriptor:desc error:&error];
        if (!archive) {
            SkDebugf("Error creating MTLBinaryArchive:\n%s\n",
                     error.debugDescription.UTF8String);
            return nil;
        }
    }

    return std::unique_ptr<GrMtlBinaryArchive>(new GrMtlBinaryArchive(archive));
}

bool GrMtlBinaryArchive::store() {
    NSArray *paths = [[NSFileManager defaultManager] URLsForDirectory:NSCachesDirectory
                                                            inDomains:NSUserDomainMask];
    NSURL* cachePath = [paths objectAtIndex:0];
    NSError* error;
    NSURL* fileURL = [cachePath URLByAppendingPathComponent:@"binaryArchiveCache"];
    [fBinaryArchive serializeToURL:fileURL error:&error];
    if (error) {
        SkDebugf("Error storing MTLBinaryArchive:\n%s\n",
                 error.debugDescription.UTF8String);
        return false;
    }

    return true;
}

