#import <UIKit/UIKit.h>
#include "SkApplication.h"

extern bool set_cmd_line_args(int argc, char *argv[], const char* dir);

int main(int argc, char *argv[]) {
    signal(SIGPIPE, SIG_IGN);
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    application_init();
    // Identify the documents directory
    NSArray *dirPaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *docsDir = [dirPaths objectAtIndex:0];
    const char *d = [docsDir UTF8String];

    bool ranCommand = set_cmd_line_args(argc, argv, d);
    int retVal = ranCommand ? 0 : UIApplicationMain(argc, argv, nil, nil);
    application_term();
    [pool release];
    return retVal;
}
