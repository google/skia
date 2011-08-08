#import "SkNSView.h"
#include "SkApplication.h"
#include "SkCanvas.h"
#include "SkCGUtils.h"
#include "SkEvent.h"

//#define FORCE_REDRAW
@implementation SkNSView
@synthesize fWind, fTitle, fOptionsDelegate, fGLContext;

- (id)initWithCoder:(NSCoder*)coder {
    if ((self = [super initWithCoder:coder])) {
        self = [self initWithMyDefaults];
    }
    return self;
}

- (id)initWithFrame:(NSRect)frameRect {
    if (self = [super initWithFrame:frameRect]) {
        self = [self initWithMyDefaults];
    }
    return self;
}

- (id)initWithMyDefaults {
    fRedrawRequestPending = false;
    fWind = new SampleWindow(self, NULL, NULL, NULL);
    application_init();
    fWind->resize(self.frame.size.width, self.frame.size.height, 
                  SkBitmap::kARGB_8888_Config);
    return self;
}

-(BOOL) isFlipped {
    return YES;
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

-(BOOL) inLiveResize {
    NSSize s = [self frame].size;
    if (fWind != NULL && fWind->width() != s.width && fWind->height() != s.height) {
        fWind->resize(s.width, s.height);
        [fGLContext update];
    }
    return [super inLiveResize];
}

- (void)dealloc {
    delete fWind;
    self.fGLContext = nil;
    self.fTitle = nil;
    application_term();
    [super dealloc];
}

///////////////////////////////////////////////////////////////////////////////

- (void)drawSkia {
    fRedrawRequestPending = false;
    SkCanvas canvas(fWind->getBitmap());
    fWind->draw(&canvas);
#ifdef FORCE_REDRAW
    fWind->inval(NULL);
#endif
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
    SkKey key = raw2key([event keyCode]);
    if (kNONE_SkKey != key)
        fWind->handleKey(key);
    else{
        unichar c = [[event characters] characterAtIndex:0];
        fWind->handleChar((SkUnichar)c);
    }
}

- (void)keyUp:(NSEvent *)event {
    SkKey key = raw2key([event keyCode]);
    if (kNONE_SkKey != key)
        fWind->handleKeyUp(key);
    else{
        unichar c = [[event characters] characterAtIndex:0];
        //fWind->handleChar((SkUnichar)c);
    }
}

- (void)mouseDown:(NSEvent *)event {
    NSPoint p = [event locationInWindow];
    if ([self mouse:p inRect:[self bounds]]) { //unecessary for a window with a single view
        NSPoint loc = [self convertPoint:p fromView:nil];
        fWind->handleClick(loc.x, loc.y, SkView::Click::kDown_State, self);
    }
}

- (void)mouseDragged:(NSEvent *)event {
    NSPoint p = [event locationInWindow];
    if ([self mouse:p inRect:[self bounds]]) {
        NSPoint loc = [self convertPoint:p fromView:nil];
        fWind->handleClick(loc.x, loc.y, SkView::Click::kMoved_State, self);
    }
}

- (void)mouseUp:(NSEvent *)event {
    NSPoint p = [event locationInWindow];
    if ([self mouse:p inRect:[self bounds]]) {
        NSPoint loc = [self convertPoint:p fromView:nil];
        fWind->handleClick(loc.x, loc.y, SkView::Click::kUp_State, self);
    }
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

///////////////////////////////////////////////////////////////////////////////
#include <OpenGL/OpenGL.h>

CGLContextObj createGLContext() {
    GLint major, minor;
    CGLContextObj ctx;
    
    CGLGetVersion(&major, &minor);
    SkDebugf("---- cgl version %d %d\n", major, minor);
    
    const CGLPixelFormatAttribute attributes[] = {
        kCGLPFAStencilSize, (CGLPixelFormatAttribute)8,
#if USE_MSAA
        kCGLPFASampleBuffers, 1,
        kCGLPFAMultisample,
        kCGLPFASamples, 8,
#endif
        kCGLPFAAccelerated,
        kCGLPFADoubleBuffer,
        (CGLPixelFormatAttribute)0
    };
    
    CGLPixelFormatObj format;
    GLint npix;
    CGLChoosePixelFormat(attributes, &format, &npix);
    SkDebugf("----- cgl format %p\n", format);
    
    CGLCreateContext(format, NULL, &ctx);

    SkDebugf("----- cgl context %p\n", ctx);
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
        glClear(GL_STENCIL_BUFFER_BIT);
    }
}
- (void)attachGL {
    if (nil == fGLContext)
        fGLContext = [[NSOpenGLContext alloc] initWithCGLContextObj:createGLContext()];
    
    [fGLContext makeCurrentContext];
    
    glViewport(0, 0, self.bounds.size.width, self.bounds.size.width);
    glClearColor(0, 0, 0, 0);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

- (void)detachGL {
    [fGLContext clearDrawable];
}

- (void)presentGL {
    [fGLContext flushBuffer];
}
@end