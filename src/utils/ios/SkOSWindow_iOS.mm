#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/gl.h>
#include "SkTypes.h"
#include "SkWindow.h"
#include "SkCanvas.h"
#include "SkOSMenu.h"
#include "SkTime.h"
#include "SkGraphics.h"

#define kINVAL_UIVIEW_EventType "inval-uiview"
#include "SkIOSNotifier.h"
#import "SkUIView_shell.h"

SkOSWindow::SkOSWindow(void* hWnd) : fHWND(hWnd) {
    fInvalEventIsPending = false;
    fNotifier = [[SkIOSNotifier alloc] init];
}
SkOSWindow::~SkOSWindow() {
    [(SkIOSNotifier*)fNotifier release];
}

void SkOSWindow::onHandleInval(const SkIRect& r) {
    if (!fInvalEventIsPending) {
        fInvalEventIsPending = true;
        (new SkEvent(kINVAL_UIVIEW_EventType))->post(this->getSinkID());
    }
}

bool SkOSWindow::onEvent(const SkEvent& evt) {
    if (evt.isType(kINVAL_UIVIEW_EventType)) {
        fInvalEventIsPending = false;
        const SkIRect& r = this->getDirtyBounds();
        [(SkUIView_shell*)fHWND postInvalWithRect:&r];
        return true;
    }
    if ([(SkUIView_shell*)fHWND onHandleEvent:evt]) {
        return true;
    }
    return INHERITED::onEvent(evt);
}

void SkOSWindow::onSetTitle(const char title[]) {
    [(SkUIView_shell*)fHWND setSkTitle:title];
}

void SkOSWindow::onAddMenu(const SkOSMenu* sk_menu)
{
    
}

///////////////////////////////////////////////////////////////////////////////////////
/*
 #if 1
 static void NonEmptyCallback(CFRunLoopTimerRef timer, void*) {
 //    printf("------- event queue depth = %d\n", SkEvent::CountEventsOnQueue());
 
 if (!SkEvent::ProcessEvent()) {
 CFRunLoopTimerInvalidate(timer);
 }
 }
 
 void SkEvent::SignalNonEmptyQueue() {
 double tinyDelay = 1.0 / 60;
 CFRunLoopTimerRef timer;
 
 timer = CFRunLoopTimerCreate(NULL,
 CACurrentMediaTime() + tinyDelay,
 tinyDelay,
 0,
 0,
 NonEmptyCallback,
 NULL);
 CFRunLoopAddTimer(CFRunLoopGetCurrent(),
 timer,
 kCFRunLoopCommonModes);
 CFRelease(timer);
 }
 #elif 1
 #if 0
 #define NONE_EMPTY_CODE(code)   code
 #else
 #define NONE_EMPTY_CODE(code)
 #endif
 static CFRunLoopSourceRef gNonEmptySource;
 static CFRunLoopRef gNoneEmptyRunLoop;
 static bool gAlreadySignaled;
 
 static void signal_nonempty() {
 if (!gAlreadySignaled) {
 NONE_EMPTY_CODE(printf("--- before resignal\n");)
 gAlreadySignaled = true;
 CFRunLoopSourceSignal(gNonEmptySource);
 CFRunLoopWakeUp(gNoneEmptyRunLoop);
 NONE_EMPTY_CODE(printf("--- after resignal\n");)
 }
 }
 
 static void NonEmptySourceCallback(void*) {
 gAlreadySignaled = false;
 NONE_EMPTY_CODE(printf("---- service NonEmptySourceCallback %d\n", SkEvent::CountEventsOnQueue());)
 if (SkEvent::ProcessEvent()) {
 signal_nonempty();
 }
 NONE_EMPTY_CODE(printf("----- after service\n");)
 }
 
 void SkEvent::SignalNonEmptyQueue() {
 if (NULL == gNonEmptySource) {
 gNoneEmptyRunLoop = CFRunLoopGetMain();
 
 CFIndex order = 0;  // should this be lower, to not start UIEvents?
 CFRunLoopSourceContext context;
 sk_bzero(&context, sizeof(context));
 // give it a "unique" info, for the default Hash function
 context.info = (void*)NonEmptySourceCallback;
 // our perform callback
 context.perform = NonEmptySourceCallback;
 gNonEmptySource = CFRunLoopSourceCreate(NULL, order, &context);
 
 CFRunLoopAddSource(gNoneEmptyRunLoop,
 gNonEmptySource,
 kCFRunLoopCommonModes);
 }
 signal_nonempty();
 }
 #elif 1
 @interface NonEmptyHandler : NSObject {}
 - (void)signalNonEmptyQ;
 @end
 
 @implementation NonEmptyHandler
 
 - (void)callProccessEvent {
 //    printf("----- callProcessEvent\n");
 if (SkEvent::ProcessEvent()) {
 [self signalNonEmptyQ];
 }
 }
 
 - (void)signalNonEmptyQ {
 [self performSelectorOnMainThread:@selector(callProccessEvent) withObject:nil waitUntilDone:NO];
 }
 
 void SkEvent::SignalNonEmptyQueue() {
 static id gNonEmptyQueueObject;
 if (nil == gNonEmptyQueueObject) {
 gNonEmptyQueueObject = [[NonEmptyHandler alloc] init];
 }
 [gNonEmptyQueueObject signalNonEmptyQ];
 }
 
 @end
 
 #endif
 
 ///////////////////////////////////////////////////////////////////////////////
 
 static CFRunLoopTimerRef gTimer;
 
 static void TimerCallback(CFRunLoopTimerRef timer, void* info) {
 gTimer = NULL;
 SkEvent::ServiceQueueTimer();
 }
 
 void SkEvent::SignalQueueTimer(SkMSec delay)
 {
 //We always release the timer right after we've added it to our RunLoop,
 //thus we don't worry about freeing it later: if it fires our callback,
 //it gets automatically freed, as it does if we call invalidate()
 
 if (gTimer) {
 // our callback wasn't called, so invalidate it
 CFRunLoopTimerInvalidate(gTimer);
 gTimer = NULL;
 }
 
 if (delay) {
 gTimer = CFRunLoopTimerCreate(NULL,
 CACurrentMediaTime() + delay/1000.0,
 //                                      CFAbsoluteTimeGetCurrent() + delay/1000.0,
 0,
 0,
 0,
 TimerCallback,
 NULL);
 CFRunLoopAddTimer(CFRunLoopGetCurrent(),
 gTimer,
 kCFRunLoopCommonModes);
 CFRelease(gTimer);
 }
 }
 */
///////////////////////////////////////////////////////////////////////////////////////

bool SkOSWindow::attachGL()
{
    bool success = true;
    return success;
}

void SkOSWindow::detachGL() {
}

void SkOSWindow::presentGL() {
    glFlush();
}

