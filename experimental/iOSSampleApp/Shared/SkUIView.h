#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#include "SkMatrix.h"
#include "FlingState.h"
#include "SampleApp.h"
#include "SkiOSDeviceManager.h"
class SkOSWindow;
class SkEvent;
struct FPSState;
@interface SkUIView : UIView <UIAccelerometerDelegate> {
    BOOL fRedrawRequestPending;
    SkMatrix    fMatrix;

    float       fZoomAroundX, fZoomAroundY;
    bool        fZoomAround;

    struct {
        EAGLContext*    fContext;
        GLuint          fRenderbuffer;
        GLuint          fStencilbuffer;
        GLuint          fFramebuffer;
        GLint           fWidth;
        GLint           fHeight;
    } fGL;

    FPSState* fFPSState;
    NSString* fTitle;
    UINavigationItem* fTitleItem;
    SkOSWindow* fWind;
    
    SkiOSDeviceManager* fDevManager;
}

@property (nonatomic, assign) SkOSWindow *fWind;
@property (nonatomic, retain) UINavigationItem* fTitleItem;
@property (nonatomic, copy) NSString* fTitle;

- (void)setSkTitle:(const char*)title;
- (void)postInvalWithRect:(const SkIRect*)rectOrNil;
- (BOOL)onHandleEvent:(const SkEvent&)event;

@end

