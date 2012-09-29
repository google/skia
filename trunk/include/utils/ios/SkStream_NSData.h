
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkStream_NSData_DEFINED
#define SkStream_NSData_DEFINED

#import <UIKit/UIKit.h>
#include "SkStream.h"

/** Returns an NSData with a copy of the stream's data. The caller must call
    retain if it intends to keep the data object beyond the current stack-frame
    (i.e. internally we're calling [NSData dataWithBytes...]
 */
NSData* NSData_dataWithStream(SkStream* stream);

/** Returns an NSData from the named resource (from main bundle).
    The caller must call retain if it intends to keep the data object beyond
    the current stack-frame
    (i.e. internally we're calling [NSData dataWithContentsOfMappedFile...]
 */
NSData* NSData_dataFromResource(const char name[], const char suffix[]);

/** Wrap a stream around NSData.
 */
class SkStream_NSData : public SkMemoryStream {
public:
            SkStream_NSData(NSData* data);
    virtual ~SkStream_NSData();

    static SkStream_NSData* CreateFromResource(const char name[],
                                               const char suffix[]);

private:
    NSData* fNSData;
};

#endif
