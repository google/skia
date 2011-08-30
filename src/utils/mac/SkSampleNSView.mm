#import "SkSampleNSView.h"
#include "SampleApp.h"
@implementation SkSampleNSView

- (id)initWithDefaults {
    if (self = [super initWithDefaults]) {
        fWind = new SampleWindow(self, NULL, NULL, NULL);
    }
    return self;
}

- (void)dealloc {
    delete fWind;
    [super dealloc];
}

- (void)swipeWithEvent:(NSEvent *)event {
    CGFloat x = [event deltaX];
    if (x < 0)
        ((SampleWindow*)fWind)->previousSample();
    else if (x > 0)
        ((SampleWindow*)fWind)->nextSample();
    else
        ((SampleWindow*)fWind)->showOverview();
}

@end
