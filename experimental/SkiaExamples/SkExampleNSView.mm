//
//  SkExampleNSView.m
//  SkiaExamples
//
//  Created by Sergio González on 6/11/13.
//
//

#import "SkExampleNSView.h"

#include "SkApplication.h"
#include <crt_externs.h>  

@implementation SkExampleNSView

- (id)initWithDefaults {
    if ((self = [super initWithDefaults])) {
        fWind = create_sk_window(self, *_NSGetArgc(), *_NSGetArgv());
    }
    return self;
}

- (void)dealloc {
    delete fWind;
    [super dealloc];
}

@end
