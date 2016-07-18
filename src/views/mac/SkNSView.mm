/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#import "SkNSView.h"
#include "SkCanvas.h"
#include "SkSurface.h"
#include "SkCGUtils.h"
#include "SkEvent.h"
static_assert(SK_SUPPORT_GPU, "not_implemented_for_non_gpu_build");
#include <OpenGL/gl.h>

//#define FORCE_REDRAW
// Can be dropped when we no longer support 10.6.
#define RETINA_API_AVAILABLE (defined(MAC_OS_X_VERSION_10_7) && \
                              MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7)
@implementation SkNSView
@synthesize fWind, fTitle, fOptionsDelegate, fGLContext;

- (id)initWithCoder:(NSCoder*)coder {
    if ((self = [super initWithCoder:coder])) {
        self = [self initWithDefaults];
        [self setUpWindow];
    }
    return self;
}

- (id)initWithFrame:(NSRect)frameRect {
    if ((self = [super initWithFrame:frameRect])) {
        self = [self initWithDefaults];
        [self setUpWindow];
    }
    return self;
}

- (id)initWithDefaults {
#if RETINA_API_AVAILABLE
    [self setWantsBestResolutionOpenGLSurface:YES];
#endif
    fRedrawRequestPending = false;
    fWind = NULL;
    return self;
}

- (void)setUpWindow {
    [[NSNotificationCenter defaultCenter] addObserver:self
                                          selector:@selector(backingPropertiesChanged:)
                                          name:@"NSWindowDidChangeBackingPropertiesNotification"
                                          object:[self window]];
    if (fWind) {
        fWind->setVisibleP(true);
        NSSize size = self.frame.size;
#if RETINA_API_AVAILABLE
        size = [self convertSizeToBacking:self.frame.size];
#endif
        fWind->resize((int) size.width, (int) size.height);
        [[self window] setAcceptsMouseMovedEvents:YES];
    }
}

-(BOOL) isFlipped {
    return YES;
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (float)scaleFactor {
    NSWindow *window = [self window];
#if RETINA_API_AVAILABLE
    if (window) {
        return [window backingScaleFactor];
    }
    return [[NSScreen mainScreen] backingScaleFactor];
#else
    if (window) {
        return [window userSpaceScaleFactor];
    }
    return [[NSScreen mainScreen] userSpaceScaleFactor];
#endif
}

- (void)backingPropertiesChanged:(NSNotification *)notification {
    CGFloat oldBackingScaleFactor = (CGFloat)[
        [notification.userInfo objectForKey:@"NSBackingPropertyOldScaleFactorKey"] doubleValue
    ];
    CGFloat newBackingScaleFactor = [self scaleFactor];
    if (oldBackingScaleFactor == newBackingScaleFactor) {
        return;
    }
    
    // TODO: need a better way to force a refresh (that works).
    // [fGLContext update] does not appear to update if the point size has not changed,
    // even if the backing size has changed.
    [self setFrameSize:NSMakeSize(self.frame.size.width + 1, self.frame.size.height + 1)];
}

- (void)resizeSkView:(NSSize)newSize {
#if RETINA_API_AVAILABLE
    newSize = [self convertSizeToBacking:newSize];
#endif
    if (fWind && (fWind->width()  != newSize.width || fWind->height() != newSize.height)) {
        fWind->resize((int) newSize.width, (int) newSize.height);
        if (fGLContext) {
            glClear(GL_STENCIL_BUFFER_BIT);
            [fGLContext update];
        }
    }
}

- (void) setFrameSize:(NSSize)newSize {
    [super setFrameSize:newSize];
    [self resizeSkView:newSize];
}

- (void)dealloc {
    [self freeNativeWind];
    self.fGLContext = nil;
    self.fTitle = nil;
    [super dealloc];
}

- (void)freeNativeWind {
    delete fWind;
    fWind = nil;
}

////////////////////////////////////////////////////////////////////////////////

- (void)drawSkia {
    fRedrawRequestPending = false;
    if (fWind) {
        SkAutoTUnref<SkSurface> surface(fWind->createSurface());
        fWind->draw(surface->getCanvas());
#ifdef FORCE_REDRAW
        fWind->inval(NULL);
#endif
    }
}

- (void)setSkTitle:(const char *)title {
    self.fTitle = [NSString stringWithUTF8String:title];
    [[self window] setTitle:self.fTitle];
}

- (BOOL)onHandleEvent:(const SkEvent&)evt {
    return false;
}

#include "SkOSMenu.h"
- (void)onAddMenu:(const SkOSMenu*)menu {
    [self.fOptionsDelegate view:self didAddMenu:menu];
}

- (void)onUpdateMenu:(const SkOSMenu*)menu {
    [self.fOptionsDelegate view:self didUpdateMenu:menu];
}

- (void)postInvalWithRect:(const SkIRect*)r {
    if (!fRedrawRequestPending) {
        fRedrawRequestPending = true;
        [self setNeedsDisplay:YES];
        [self performSelector:@selector(drawSkia) withObject:nil afterDelay:0];
    }
}
///////////////////////////////////////////////////////////////////////////////

#include "SkKey.h"
enum {
	SK_MacReturnKey		= 36,
	SK_MacDeleteKey		= 51,
	SK_MacEndKey		= 119,
	SK_MacLeftKey		= 123,
	SK_MacRightKey		= 124,
	SK_MacDownKey		= 125,
	SK_MacUpKey			= 126,
    SK_Mac0Key          = 0x52,
    SK_Mac1Key          = 0x53,
    SK_Mac2Key          = 0x54,
    SK_Mac3Key          = 0x55,
    SK_Mac4Key          = 0x56,
    SK_Mac5Key          = 0x57,
    SK_Mac6Key          = 0x58,
    SK_Mac7Key          = 0x59,
    SK_Mac8Key          = 0x5b,
    SK_Mac9Key          = 0x5c
};

static SkKey raw2key(UInt32 raw)
{
	static const struct {
		UInt32  fRaw;
		SkKey   fKey;
	} gKeys[] = {
		{ SK_MacUpKey,		kUp_SkKey		},
		{ SK_MacDownKey,	kDown_SkKey		},
		{ SK_MacLeftKey,	kLeft_SkKey		},
		{ SK_MacRightKey,   kRight_SkKey	},
		{ SK_MacReturnKey,  kOK_SkKey		},
		{ SK_MacDeleteKey,  kBack_SkKey		},
		{ SK_MacEndKey,		kEnd_SkKey		},
        { SK_Mac0Key,       k0_SkKey        },
        { SK_Mac1Key,       k1_SkKey        },
        { SK_Mac2Key,       k2_SkKey        },
        { SK_Mac3Key,       k3_SkKey        },
        { SK_Mac4Key,       k4_SkKey        },
        { SK_Mac5Key,       k5_SkKey        },
        { SK_Mac6Key,       k6_SkKey        },
        { SK_Mac7Key,       k7_SkKey        },
        { SK_Mac8Key,       k8_SkKey        },
        { SK_Mac9Key,       k9_SkKey        }
	};
    
	for (unsigned i = 0; i < SK_ARRAY_COUNT(gKeys); i++)
		if (gKeys[i].fRaw == raw)
			return gKeys[i].fKey;
	return kNONE_SkKey;
}

- (void)keyDown:(NSEvent *)event {
    if (NULL == fWind)
        return;
    
    SkKey key = raw2key([event keyCode]);
    if (kNONE_SkKey != key)
        fWind->handleKey(key);
    else{
        unichar c = [[event characters] characterAtIndex:0];
        fWind->handleChar((SkUnichar)c);
    }
}

- (void)keyUp:(NSEvent *)event {
    if (NULL == fWind)
        return;
    
    SkKey key = raw2key([event keyCode]);
    if (kNONE_SkKey != key)
        fWind->handleKeyUp(key);
 // else
 //     unichar c = [[event characters] characterAtIndex:0];
}

static const struct {
    unsigned    fNSModifierMask;
    unsigned    fSkModifierMask;
} gModifierMasks[] = {
    { NSAlphaShiftKeyMask,  kShift_SkModifierKey },
    { NSShiftKeyMask,       kShift_SkModifierKey },
    { NSControlKeyMask,     kControl_SkModifierKey },
    { NSAlternateKeyMask,   kOption_SkModifierKey },
    { NSCommandKeyMask,     kCommand_SkModifierKey },
};

static unsigned convertNSModifiersToSk(NSUInteger nsModi) {
    unsigned skModi = 0;
    for (size_t i = 0; i < SK_ARRAY_COUNT(gModifierMasks); ++i) {
        if (nsModi & gModifierMasks[i].fNSModifierMask) {
            skModi |= gModifierMasks[i].fSkModifierMask;
        }
    }
    return skModi;
}

- (void)mouseDown:(NSEvent *)event {
    NSPoint p = [event locationInWindow];
    unsigned modi = convertNSModifiersToSk([event modifierFlags]);

    if ([self mouse:p inRect:[self bounds]] && fWind) {
        NSPoint loc = [self convertPoint:p fromView:nil];
#if RETINA_API_AVAILABLE
        loc = [self convertPointToBacking:loc]; //y-up
        loc.y = -loc.y;
#endif
        fWind->handleClick((int) loc.x, (int) loc.y,
                           SkView::Click::kDown_State, self, modi);
    }
}

- (void)mouseDragged:(NSEvent *)event {
    NSPoint p = [event locationInWindow];
    unsigned modi = convertNSModifiersToSk([event modifierFlags]);

    if ([self mouse:p inRect:[self bounds]] && fWind) {
        NSPoint loc = [self convertPoint:p fromView:nil];
#if RETINA_API_AVAILABLE
        loc = [self convertPointToBacking:loc]; //y-up
        loc.y = -loc.y;
#endif
        fWind->handleClick((int) loc.x, (int) loc.y,
                           SkView::Click::kMoved_State, self, modi);
    }
}

- (void)mouseMoved:(NSEvent *)event {
    NSPoint p = [event locationInWindow];
    unsigned modi = convertNSModifiersToSk([event modifierFlags]);
    
    if ([self mouse:p inRect:[self bounds]] && fWind) {
        NSPoint loc = [self convertPoint:p fromView:nil];
#if RETINA_API_AVAILABLE
        loc = [self convertPointToBacking:loc]; //y-up
        loc.y = -loc.y;
#endif
        fWind->handleClick((int) loc.x, (int) loc.y,
                           SkView::Click::kMoved_State, self, modi);
    }
}

- (void)mouseUp:(NSEvent *)event {
    NSPoint p = [event locationInWindow];
    unsigned modi = convertNSModifiersToSk([event modifierFlags]);
    
    if ([self mouse:p inRect:[self bounds]] && fWind) {
        NSPoint loc = [self convertPoint:p fromView:nil];
#if RETINA_API_AVAILABLE
        loc = [self convertPointToBacking:loc]; //y-up
        loc.y = -loc.y;
#endif
        fWind->handleClick((int) loc.x, (int) loc.y,
                           SkView::Click::kUp_State, self, modi);
    }
}

///////////////////////////////////////////////////////////////////////////////
#include <OpenGL/OpenGL.h>

static CGLContextObj createGLContext(int msaaSampleCount) {
    GLint major, minor;
    CGLGetVersion(&major, &minor);
    
    static const CGLPixelFormatAttribute attributes[] = {
        kCGLPFAStencilSize, (CGLPixelFormatAttribute) 8,
        kCGLPFAAccelerated,
        kCGLPFADoubleBuffer,
        kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute) kCGLOGLPVersion_3_2_Core,
        (CGLPixelFormatAttribute)0
    };
    
    CGLPixelFormatObj format;
    GLint npix = 0;
    if (msaaSampleCount > 0) {
        static const int kAttributeCount = SK_ARRAY_COUNT(attributes);
        CGLPixelFormatAttribute msaaAttributes[kAttributeCount + 5];
        memcpy(msaaAttributes, attributes, sizeof(attributes));
        SkASSERT(0 == msaaAttributes[kAttributeCount - 1]);
        msaaAttributes[kAttributeCount - 1] = kCGLPFASampleBuffers;
        msaaAttributes[kAttributeCount + 0] = (CGLPixelFormatAttribute)1;
        msaaAttributes[kAttributeCount + 1] = kCGLPFAMultisample;
        msaaAttributes[kAttributeCount + 2] = kCGLPFASamples;
        msaaAttributes[kAttributeCount + 3] =
                                    (CGLPixelFormatAttribute)msaaSampleCount;
        msaaAttributes[kAttributeCount + 4] = (CGLPixelFormatAttribute)0;
        CGLChoosePixelFormat(msaaAttributes, &format, &npix);
    }
    if (!npix) {
        CGLChoosePixelFormat(attributes, &format, &npix);
    }
    CGLContextObj ctx;
    CGLCreateContext(format, NULL, &ctx);
    CGLDestroyPixelFormat(format);
    
    static const GLint interval = 1;
    CGLSetParameter(ctx, kCGLCPSwapInterval, &interval);
    CGLSetCurrentContext(ctx);
    return ctx;
}

- (void)viewDidMoveToWindow {
    [super viewDidMoveToWindow];
    
    //Attaching view to fGLContext requires that the view to be part of a window,
    //and that the NSWindow instance must have a CoreGraphics counterpart (or 
    //it must NOT be deferred or should have been on screen at least once)
    if ([fGLContext view] != self && nil != self.window) {
        [fGLContext setView:self];
    }
}
- (bool)attach:(SkOSWindow::SkBackEndTypes)attachType
        withMSAASampleCount:(int) sampleCount
        andGetInfo:(SkOSWindow::AttachmentInfo*) info {
    if (nil == fGLContext) {
        CGLContextObj ctx = createGLContext(sampleCount);
        SkASSERT(ctx);
        fGLContext = [[NSOpenGLContext alloc] initWithCGLContextObj:ctx];
        CGLReleaseContext(ctx);
        if (NULL == fGLContext) {
            return false;
        }
        [fGLContext setView:self];
    }

    [fGLContext makeCurrentContext];
    CGLPixelFormatObj format = CGLGetPixelFormat((CGLContextObj)[fGLContext CGLContextObj]);
    CGLDescribePixelFormat(format, 0, kCGLPFASamples, &info->fSampleCount);
    CGLDescribePixelFormat(format, 0, kCGLPFAStencilSize, &info->fStencilBits);
    NSSize size = self.bounds.size;
#if RETINA_API_AVAILABLE
    size = [self convertSizeToBacking:size];
#endif
    glViewport(0, 0, (int) size.width, (int) size.height);
    glClearColor(0, 0, 0, 0);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    return true;
}

- (void)detach {
    [fGLContext release];
    fGLContext = nil;
}

- (void)present {
    if (nil != fGLContext) {
        [fGLContext flushBuffer];
    }
}

- (void)setVSync:(bool)enable {
    if (fGLContext) {
        GLint interval = enable ? 1 : 0;
        CGLContextObj ctx = (CGLContextObj)[fGLContext CGLContextObj];
        CGLSetParameter(ctx, kCGLCPSwapInterval, &interval);
    }
}
@end
