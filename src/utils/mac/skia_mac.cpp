#include <Carbon/Carbon.h>
#include "SkApplication.h"
#include "SkWindow.h"

int main(int argc, char* argv[])
{
    WindowRef			window;
    OSStatus			err = noErr;

    Rect bounds = {100, 100, 500, 500};
    WindowAttributes attrs = kWindowStandardHandlerAttribute | 
                             kWindowLiveResizeAttribute |
                             kWindowInWindowMenuAttribute | 
                             kWindowCompositingAttribute |
                             kWindowAsyncDragAttribute | 
                             kWindowFullZoomAttribute | 
                             kWindowFrameworkScaledAttribute;
                             //kWindowDoesNotCycleAttribute;
    CreateNewWindow(kDocumentWindowClass, attrs, &bounds, &window);

    MenuRef menu;
    CreateNewMenu(0, 0, &menu);

    // if we get here, we can start our normal Skia sequence
    {
        application_init();
        (void)create_sk_window(window);
        SizeWindow(window, 640, 480, false);
    }
    
    // The window was created hidden so show it.
    ShowWindow( window );
    
    // Call the event loop
    RunApplicationEventLoop();

	application_term();

CantCreateWindow:
CantGetNibRef:
	return err;
}

