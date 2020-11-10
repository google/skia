/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlBinaryArchive_DEFINED
#define GrMtlBinaryArchive_DEFINED

#import <Metal/Metal.h>

#include <memory>

// A wrapper for a MTLBinaryArchive object with serialization support.
class GrMtlBinaryArchive {
public:
    static std::unique_ptr<GrMtlBinaryArchive> Make(id<MTLDevice> device);

    ~GrMtlBinaryArchive() { this->store(); fBinaryArchive = nil; }

    bool store();

    id<MTLBinaryArchive> archive() { return fBinaryArchive; }

private:
    GrMtlBinaryArchive(id<MTLBinaryArchive> binaryArchive)
        : fBinaryArchive(binaryArchive) {}

    id<MTLBinaryArchive> fBinaryArchive = nil;
};

#endif
