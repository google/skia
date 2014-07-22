#import <UIKit/UIKit.h>
#include "SkApplication.h"

int main(int argc, char *argv[]) {
    signal(SIGPIPE, SIG_IGN);
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    application_init();
    // Identify the documents directory
    NSArray *dirPaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *docsDir = [dirPaths objectAtIndex:0];
    const char *d = [docsDir UTF8String];
    IOS_launch_type launchType = set_cmd_line_args(argc, argv, d);
    int retVal = launchType == kApplication__iOSLaunchType
            ? UIApplicationMain(argc, argv, nil, nil) : (int) launchType;
    application_term();
    [pool release];
    return retVal;
}
