/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#import "SkUIView.h"
#include "SkCanvas.h"
#include "SkCGUtils.h"
@implementation SkUIView

@synthesize fWind, fTitleItem, fOptionsDelegate;

- (id)initWithDefaults {
    fWind = NULL;
    return self;
}

- (id)initWithCoder:(NSCoder*)coder {
    if ((self = [super initWithCoder:coder])) {
        self = [self initWithDefaults];
        [self setUpWindow];
    }
    return self;
}

- (id)initWithFrame:(CGRect)frame {
    if (self = [super initWithFrame:frame]) {
        self = [self initWithDefaults];
        [self setUpWindow];
    }
    return self;
}

- (void)setUpWindow {
    if (NULL != fWind) {
        fWind->setVisibleP(true);
        fWind->resize(self.frame.size.width, self.frame.size.height);
    }
}

- (void)dealloc {
    delete fWind;
    [fTitleItem release];
    [super dealloc];
}

- (void)forceRedraw {
    [self drawInRaster];
}

- (void)drawInRaster {
    SkCanvas canvas(fWind->getBitmap());
    fWind->draw(&canvas);
    CGImageRef cgimage = SkCreateCGImageRef(fWind->getBitmap());
    self.layer.contents = (id)cgimage;
    CGImageRelease(cgimage);
}

//Gesture Handlers
- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
    for (UITouch *touch in touches) {
        CGPoint loc = [touch locationInView:self];
        fWind->handleClick(loc.x, loc.y, SkView::Click::kDown_State, touch);
    }
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
    for (UITouch *touch in touches) {
        CGPoint loc = [touch locationInView:self];
        fWind->handleClick(loc.x, loc.y, SkView::Click::kMoved_State, touch);
    }
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
    for (UITouch *touch in touches) {
        CGPoint loc = [touch locationInView:self];
        fWind->handleClick(loc.x, loc.y, SkView::Click::kUp_State, touch);
    }
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {
    for (UITouch *touch in touches) {
        CGPoint loc = [touch locationInView:self];
        fWind->handleClick(loc.x, loc.y, SkView::Click::kUp_State, touch);
    }
}

///////////////////////////////////////////////////////////////////////////////

- (void)setSkTitle:(const char *)title {
    if (fTitleItem) {
        fTitleItem.title = [NSString stringWithUTF8String:title];
    }
}

- (BOOL)onHandleEvent:(const SkEvent&)evt {
    return false;
}

- (void)getAttachmentInfo:(SkOSWindow::AttachmentInfo*)info {
    // we don't have a GL context.
    info->fSampleCount = 0;
    info->fStencilBits = 0;
}

#include "SkOSMenu.h"
- (void)onAddMenu:(const SkOSMenu*)menu {
    [self.fOptionsDelegate view:self didAddMenu:menu];
}
- (void)onUpdateMenu:(SkOSMenu*)menu {
    [self.fOptionsDelegate view:self didUpdateMenu:menu];
}

- (void)postInvalWithRect:(const SkIRect*)r {
    [self performSelector:@selector(drawInRaster) withObject:nil afterDelay:0];
    [self setNeedsDisplay];
}

@end
