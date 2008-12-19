#include <Carbon/Carbon.h>
#include "SkApplication.h"
#include "SkWindow.h"

int main(int argc, char* argv[])
{
    IBNibRef			nibRef;
    WindowRef			window;
    OSStatus			err = noErr;

    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr( err, CantGetNibRef );
    
    // Then create a window. "MainWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    err = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &window);
    require_noerr( err, CantCreateWindow );
    
    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);
    
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

