/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkStream_NSData.h"

NSData* NSData_dataWithStream(SkStream* stream) {
    size_t length = stream->getLength();
    void* src = malloc(length);
    size_t bytes = stream->read(src, length);
    SkASSERT(bytes == length);
    return [NSData dataWithBytesNoCopy:src length:length freeWhenDone:YES];
}

NSData* NSData_dataFromResource(const char cname[], const char csuffix[]) {
    NSBundle* bundle = [NSBundle mainBundle];
    NSString* name = [NSString stringWithUTF8String:cname];
    NSString* suffix = [NSString stringWithUTF8String:csuffix];
    NSString* path = [bundle pathForResource:name ofType:suffix];
    return [NSData dataWithContentsOfMappedFile:path];
}

///////////////////////////////////////////////////////////////////////////////

SkStream_NSData::SkStream_NSData(NSData* data) {
    fNSData = data;
    [fNSData retain];

    this->setMemory([fNSData bytes], [fNSData length], false);
}

SkStream_NSData::~SkStream_NSData() {
    [fNSData release];
}

SkStream_NSData* SkStream_NSData::CreateFromResource(const char name[],
                                                     const char suffix[]) {
    NSData* data = NSData_dataFromResource(name, suffix);
    return SkNEW_ARGS(SkStream_NSData, (data));
}

