/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#import <UIKit/UIKit.h>
#include "ios_util.h"
#include <unistd.h>

void cd_Documents() {
    @autoreleasepool {
        NSURL* dir = [[[NSFileManager defaultManager]
            URLsForDirectory:NSDocumentDirectory
                   inDomains:NSUserDomainMask] lastObject];
        chdir([dir.path UTF8String]);
    }
}
