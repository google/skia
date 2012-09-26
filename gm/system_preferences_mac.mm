/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "system_preferences.h"

#import <Cocoa/Cocoa.h>

void setSystemPreferences() {
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    // Set LCD font smoothing level for this application (does not affect other
    // applications). Based on resetDefaultsToConsistentValues() in
    // http://trac.webkit.org/browser/trunk/Tools/DumpRenderTree/mac/DumpRenderTree.mm
    enum {
        NoFontSmoothing     = 0,
        LightFontSmoothing  = 1,
        MediumFontSmoothing = 2,
        StrongFontSmoothing = 3,
    };
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    [defaults setInteger:MediumFontSmoothing forKey:@"AppleFontSmoothing"];

    [pool release];
}
