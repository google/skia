/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#import "HelloWorldNSView.h"

#include "SkApplication.h"
#include <crt_externs.h>  

@implementation HelloWorldNSView

- (id)initWithDefaults {
    if ((self = [super initWithDefaults])) {
        fWind = create_sk_window(self, *_NSGetArgc(), *_NSGetArgv());
    }
    return self;
}

- (void)dealloc {
    delete fWind;
    [super dealloc];
}

@end
