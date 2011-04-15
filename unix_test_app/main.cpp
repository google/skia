#include "X11/Xlib.h"
#include "X11/keysym.h"

#include "SkApplication.h"
#include "SkEvent.h"
#include "SkWindow.h"
#include "SkTypes.h"

//#include <signal.h>
//#include <sys/time.h>

SkOSWindow* gWindow;

#if 0
static void catch_alarm(int sig)
{
    SkDebugf("caught alarm; calling ServiceQueueTimer\n");
    SkEvent::ServiceQueueTimer();
}
#endif

int main(){
//    signal(SIGALRM, catch_alarm);

    gWindow = create_sk_window(NULL);
    // Start normal Skia sequence
    application_init();

    gWindow->loop();

    application_term();
    return 0;
}

// SkEvent handlers

void SkEvent::SignalNonEmptyQueue()
{
    if (gWindow)
        gWindow->post_linuxevent();
    else
        while (SkEvent::ProcessEvent());
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
