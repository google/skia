#include "X11/Xlib.h"
#include "X11/keysym.h"

#include "SkApplication.h"
#include "SkEvent.h"
#include "SkWindow.h"
#include "SkTypes.h"

#include <signal.h>
#include <sys/time.h>

SkOSWindow* gWindow;

static void catch_alarm(int sig)
{
    SkEvent::ServiceQueueTimer();
}

int main(int argc, char** argv){
    signal(SIGALRM, catch_alarm);

    gWindow = create_sk_window(NULL, argc, argv);
    // Start normal Skia sequence
    application_init();

    gWindow->loop();

    delete gWindow;
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
    itimerval newTimer;
    newTimer.it_interval.tv_sec = 0;
    newTimer.it_interval.tv_usec = 0;
    newTimer.it_value.tv_sec = 0;
    newTimer.it_value.tv_usec = delay * 1000;

    setitimer(ITIMER_REAL, &newTimer, NULL);
}
