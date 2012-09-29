
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#import "SkSampleNSView.h"
#include "SampleApp.h"
#include <crt_externs.h>
@implementation SkSampleNSView

- (id)initWithDefaults {
    if ((self = [super initWithDefaults])) {
        fWind = new SampleWindow(self, *_NSGetArgc(), *_NSGetArgv(), NULL);
    }
    return self;
}

- (void)dealloc {
    delete fWind;
    [super dealloc];
}

- (void)swipeWithEvent:(NSEvent *)event {
    CGFloat x = [event deltaX];
    if (x < 0)
        ((SampleWindow*)fWind)->previousSample();
    else if (x > 0)
        ((SampleWindow*)fWind)->nextSample();
    else
        ((SampleWindow*)fWind)->showOverview();
}

@end
