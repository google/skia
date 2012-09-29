#import <UIKit/UIKit.h>
#include "SkApplication.h"
int main(int argc, char *argv[]) {
    signal(SIGPIPE, SIG_IGN);
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    application_init();
    int retVal = UIApplicationMain(argc, argv, nil, nil);
    application_term();
    [pool release];
    return retVal;
}
