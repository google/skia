
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#import "SkNSWindow.h"
#import "SkDebuggerViews.h"
@interface SkDebugger : SkNSWindow {
    IBOutlet SkNSView* fCommandView;
    IBOutlet SkNSView* fContentView;
    IBOutlet SkNSView* fInfoView;
    
    SkCommandListView* fCommand;
    SkContentView* fContent;
    SkInfoPanelView* fInfo;
}

- (void)loadFile:(NSString *)filename;
@end

