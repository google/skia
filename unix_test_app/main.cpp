#include "X11/Xlib.h"
#include "X11/keysym.h"

#include "SkApplication.h"
#include "SkKey.h"
#include "SkView.h"
#include "SkWindow.h"
#include "XkeysToSkKeys.h"
#include "keysym2ucs.h"
#include "SkTypes.h"
//#include <signal.h>
//#include <sys/time.h>

// Globals for access to the window
Display* dsp = 0;
Window win;

const int WIDTH = 1000;
const int HEIGHT = 1000;

// Put an event in the X queue to fire an SkEvent.
static void post_linuxevent()
{
    if (!dsp) return;
    long event_mask = NoEventMask;
    XClientMessageEvent event;
    event.type = ClientMessage;
    Atom myAtom;
    event.message_type = myAtom;
    event.format = 32;
    event.data.l[0] = 0;
    XSendEvent(dsp, win, false, 0, (XEvent*) &event);
}

#if 0
static void catch_alarm(int sig)
{
    SkDebugf("caught alarm; calling ServiceQueueTimer\n");
    SkEvent::ServiceQueueTimer();
}
#endif

int main(){
    dsp = XOpenDisplay(NULL);
    if(!dsp) {
        return 1;
    }

//    signal(SIGALRM, catch_alarm);

    win = XCreateSimpleWindow(dsp, DefaultRootWindow(dsp), 0, 0, WIDTH, HEIGHT, 0, 0, 0);
    XMapWindow(dsp, win);

    long eventMask = StructureNotifyMask;
    XSelectInput(dsp, win, eventMask);

    // Wait until screen is ready.
    XEvent evt;
    do {
        XNextEvent(dsp, &evt);
    } while(evt.type != MapNotify);

    GC gc = XCreateGC(dsp, win, 0, NULL);
    // Start normal Skia sequence
    application_init();

    SkOSWindow* window = create_sk_window(NULL);
    window->setUnixWindow(dsp, win, DefaultScreen(dsp), gc);
    window->resize(WIDTH, HEIGHT);


    // Determine which events to listen for.
    eventMask = StructureNotifyMask|ButtonPressMask|ButtonReleaseMask
            |ExposureMask|Button1MotionMask|KeyPressMask|KeyReleaseMask;
    XSelectInput(dsp, win, eventMask);
 
    bool loop = true;
    while (loop) {
        XNextEvent(dsp, &evt);
        switch (evt.type) {
            case Expose:
                if (evt.xexpose.count == 0)
                    window->inval(NULL);
                break;
            case ConfigureNotify:
                window->resize(evt.xconfigure.width, evt.xconfigure.height);
                break;
            case ButtonPress:
                if (evt.xbutton.button == Button1)
                    window->handleClick(evt.xbutton.x, evt.xbutton.y, SkView::Click::kDown_State);
                break;
            case ButtonRelease:
                if (evt.xbutton.button == Button1)
                    window->handleClick(evt.xbutton.x, evt.xbutton.y, SkView::Click::kUp_State);
                break;
            case MotionNotify:
                // 'If' statement is unnecessary, since we are only masking for button 1
                if (evt.xbutton.button == Button1)
                    window->handleClick(evt.xmotion.x, evt.xmotion.y, SkView::Click::kMoved_State);
                break;
            case KeyPress:
            {
                KeySym keysym = XKeycodeToKeysym(dsp, evt.xkey.keycode, 0);
                //SkDebugf("pressed key %i!\n\tKeySym:%i\n", evt.xkey.keycode, XKeycodeToKeysym(dsp, evt.xkey.keycode, 0));
                if (keysym == XK_Escape) {
                    loop = false;
                    break;
                }
                window->handleKey(XKeyToSkKey(keysym));
                long uni = keysym2ucs(keysym);
                if (uni != -1) {
                    window->handleChar((SkUnichar) uni);
                }
                break;
            }
            case KeyRelease:
                //SkDebugf("released key %i\n", evt.xkey.keycode);
                window->handleKeyUp(XKeyToSkKey(XKeycodeToKeysym(dsp, evt.xkey.keycode, 0)));
                break;
            case ClientMessage:
                if (SkEvent::ProcessEvent()) {
                    post_linuxevent();
                }
                break;
            default:
                // Do nothing for other events
                break;
        }
    }

    XFreeGC(dsp, gc);
    XDestroyWindow(dsp, win);
    XCloseDisplay(dsp);

    application_term();
    return 0;
}

// SkEvent handlers

void SkEvent::SignalNonEmptyQueue()
{
    post_linuxevent();
}

void SkEvent::SignalQueueTimer(SkMSec delay)
{
#if 0
    itimerval newTimer;
    newTimer.it_interval.tv_sec = 0;
    newTimer.it_interval.tv_usec = 0;
    newTimer.it_value.tv_sec = 0;
    newTimer.it_value.tv_usec = delay * 1000;
    int success = setitimer(ITIMER_REAL, NULL, &newTimer);
    SkDebugf("SignalQueueTimer(%i)\nreturnval = %i\n", delay, success);
#endif
}
