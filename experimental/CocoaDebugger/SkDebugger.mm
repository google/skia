#import "SkDebugger.h"
@implementation SkDebugger
-(void) installSkViews {
    
    float width = [self frame].size.width;
    float height = [self frame].size.height;
    float commandListW = 200;
    float infoPanelH = 150.0;
    fCommand = new SkCommandListView;
    fCommand->setSize(commandListW, height);
    fCommand->setVisibleP(true);
    
    fInfo = new SkInfoPanelView;
    fInfo->setSize(width - commandListW, infoPanelH);
    fInfo->setVisibleP(true);
    
    fContent = new SkContentView(fCommand->getSinkID(), 
                                 fInfo->getSinkID());
    fContent->setSize(width - commandListW, height - infoPanelH);
    fContent->setVisibleP(true);
    
    [fInfoView addSkView:fInfo];
    [fCommandView addSkView:fCommand];
    [fContentView addSkView:fContent];
    
    fInfo->unref();
    fCommand->unref();
    fContent->unref();
}

- (void)loadFile:(NSString *)filename {
    fCommand->reinit();
    fContent->reinit([filename UTF8String]);
}

- (void)keyDown:(NSEvent *)event {
    // arrow keys have this mask
    if ([event modifierFlags] & NSNumericPadKeyMask) { 
        NSString *theArrow = [event charactersIgnoringModifiers];
        if ( [theArrow length] == 0 )
            return;            // reject dead keys
        if ( [theArrow length] == 1 ) {
            switch ([theArrow characterAtIndex:0]) {
                case NSLeftArrowFunctionKey:
                    fContent->goToAtom(fCommand->prevItem());
                    break;
                case NSRightArrowFunctionKey:
                    fContent->goToAtom(fCommand->nextItem());
                    break;
                case NSUpArrowFunctionKey:
                    fContent->goToAtom(fCommand->scrollUp());
                    break;
                case NSDownArrowFunctionKey:
                    fContent->goToAtom(fCommand->scrollDown());
                    break;
                default:
                    [super keyDown:event];
            }
            return;
        }
    }
    else {//normal keys
        switch ([[event characters] characterAtIndex:0]) {
            case 'c':
                fContent->toggleClip();
                break;
            case 'e':
                fCommand->toggleCentered();
                break;
            default:
                [super keyDown:event];
        }
        return;
    }
    
    [super keyDown:event];
}

- (void)mouseDown:(NSEvent *)event {
    if ([event clickCount] > 1) {
        [fContentView resetTransformations];
        [fContentView setNeedsDisplay:YES];
    }
    else {
        NSPoint p = [event locationInWindow];
        NSRect commandRect = [fCommandView convertRectToBase:[fCommandView bounds]];
        if ([fCommandView mouse:p inRect:commandRect]) {
            NSPoint mouseLocInView = [fCommandView convertPoint:p fromView:nil];
            fContent->goToAtom(fCommand->selectHighlight(mouseLocInView.y));
        }
    }
    [super mouseDown:event];
}

- (void)mouseDragged:(NSEvent *)event {
    NSPoint p = [event locationInWindow];
    NSRect contentRect = [fContentView convertRectToBase:[fContentView bounds]];
    NSRect commandRect = [fCommandView convertRectToBase:[fCommandView bounds]];
    if ([fContentView mouse:p inRect:contentRect]) {
        fContentView.fOffset =  NSMakePoint(fContentView.fOffset.x + [event deltaX], 
                                           fContentView.fOffset.y + [event deltaY]);
        fContentView.fCenter =  NSMakePoint(fContentView.fCenter.x - [event deltaX], 
                                           fContentView.fCenter.y - [event deltaY]);
        [fContentView setNeedsDisplay:YES];
    }
    [super mouseDragged:event];
}

- (void)magnifyWithEvent:(NSEvent *)event {
    if ([fContentView mouse:[event locationInWindow] 
                     inRect:[fContentView convertRectToBase:[fContentView bounds]]]) {
//        fContentView.fCenter = [fContentView convertPoint:[event locationInWindow]
//                                                fromView:nil];
        fContentView.fScale = fContentView.fScale * ([event magnification] + 1.0);
        [fContentView setNeedsDisplay:YES];
    }
    [super magnifyWithEvent:event];
}

- (void)rotateWithEvent:(NSEvent *)event {
    if ([fContentView mouse:[event locationInWindow] 
                     inRect:[fContentView convertRectToBase:[fContentView bounds]]]) {
//        fContentView.fCenter  = [fContentView convertPoint:[event locationInWindow]
//                                          fromView:nil];
        fContentView.fRotation = fContentView.fRotation - [event rotation];
        [fContentView setNeedsDisplay:YES];
    }
    [super rotateWithEvent:event];
}

- (void)scrollWheel:(NSEvent *)event {
    NSPoint p = [event locationInWindow];
    NSRect contentRect = [fContentView convertRectToBase:[fContentView bounds]];
    NSRect commandRect = [fCommandView convertRectToBase:[fCommandView bounds]];
    if ([fContentView mouse:p inRect:contentRect]) {
//        fContentView.fCenter = [fContentView convertPoint:[event locationInWindow]
//                                                fromView:nil];
        if ([event deltaY] > 0) {
            fContentView.fScale = fContentView.fScale * (1.05);
        }
        if ([event deltaY] < 0) {
            fContentView.fScale = fContentView.fScale * (0.95);
        }
        [fContentView setNeedsDisplay:YES];
    }
    if ([fCommandView mouse:p inRect:commandRect]) {
        if ([event deltaY] > 0) {
            fContent->goToAtom(fCommand->scrollUp());
        }
        if ([event deltaY] < 0) {
            fContent->goToAtom(fCommand->scrollDown());
        }
    }
    [super scrollWheel:event];
}
@end