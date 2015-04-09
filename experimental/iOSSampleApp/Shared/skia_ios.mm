/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#import <UIKit/UIKit.h>
#include "SkApplication.h"

int main(int argc, char *argv[]) {
    signal(SIGPIPE, SIG_IGN);
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    application_init();

    // Identify the documents directory
    NSArray *dirPaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *docsDir = [dirPaths objectAtIndex:0];
    NSString *resourceDir = [docsDir stringByAppendingString:@"/resources"];
    const char *d = [resourceDir UTF8String];

    // change to the dcouments directory. To allow the 'writePath' flag to use relative paths. 
    NSFileManager *filemgr = [NSFileManager defaultManager];
    int retVal = 99; 
    if ([filemgr changeCurrentDirectoryPath: docsDir] == YES)
    {
        IOS_launch_type launchType = set_cmd_line_args(argc, argv, d);
        retVal = launchType == kApplication__iOSLaunchType
                ? UIApplicationMain(argc, argv, nil, nil) : (int) launchType;
    }
    application_term();
    [filemgr release];
    [pool release];
    return retVal;
}
