#import <UIKit/UIKit.h>
#include "SkApplication.h"

extern void save_args(int argc, char *argv[]);

int main(int argc, char *argv[]) {
    signal(SIGPIPE, SIG_IGN);
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    application_init();
    save_args(argc, argv);
    int retVal = UIApplicationMain(argc, argv, nil, nil);
    application_term();
    [pool release];
    return retVal;
}
