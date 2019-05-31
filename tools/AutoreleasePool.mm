/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "tools/AutoreleasePool.h"

#import <Foundation/NSAutoreleasePool.h>

AutoreleasePool::AutoreleasePool() {
    fPool = (void*)[[NSAutoreleasePool alloc] init];
}

AutoreleasePool::~AutoreleasePool() {
    [(NSAutoreleasePool*)fPool release];
    fPool = nullptr;
}

void AutoreleasePool::drain() {
    [(NSAutoreleasePool*)fPool drain];
    fPool = (void*)[[NSAutoreleasePool alloc] init];
}
