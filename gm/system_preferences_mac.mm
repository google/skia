/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#import <Cocoa/Cocoa.h>

void setSystemPreferences() {
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    // Set LCD font smoothing level for this application (does not affect other
    // applications). Based on resetDefaultsToConsistentValues() in
    // http://trac.webkit.org/browser/trunk/Tools/DumpRenderTree/mac/DumpRenderTree.mm
    static const int NoFontSmoothing     = 0;
    static const int LightFontSmoothing  = 1;
    static const int MediumFontSmoothing = 2;
    static const int StrongFontSmoothing = 3;
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    [defaults setInteger:MediumFontSmoothing forKey:@"AppleFontSmoothing"];

    [pool release];
}
