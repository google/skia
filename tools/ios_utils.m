/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#import <Foundation/Foundation.h>
#include "tools/ios_utils.h"
#include <unistd.h>

void cd_Documents() {
    @autoreleasepool {
        NSURL* dir = [[[NSFileManager defaultManager]
            URLsForDirectory:NSDocumentDirectory
                   inDomains:NSUserDomainMask] lastObject];
        chdir([dir.path UTF8String]);
    }
}
