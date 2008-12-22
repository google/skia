#include <Carbon/Carbon.h>
#include "SkCGUtils.h"
#include "SkCanvas.h"
#include "SkPaint.h"

extern "C" void SkiaDraw(CGContextRef cg, CGRect bounds);

static void sampleDraw(SkCanvas* canvas) {
    canvas->drawColor(0xFF0000FF);
    
    SkPaint paint;
    paint.setAntiAlias(true);

    canvas->drawCircle(SkIntToScalar(100), SkIntToScalar(100),
                       SkIntToScalar(90), paint);
}

static CGImageRef gImage;

void SkiaDraw(CGContextRef cg, CGRect bounds) {
    if (NULL == gImage) {
        SkBitmap bitmap;
        bitmap.setConfig(SkBitmap::kARGB_8888_Config, 640, 480);
        bitmap.allocPixels();
        
        SkCanvas canvas(bitmap);
        sampleDraw(&canvas);
        
        gImage = SkCreateCGImageRef(bitmap);
    }

	float components[] = { 1, 1, 1, 1 };

	CGColorSpaceRef colorspace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
	CGColorRef color = CGColorCreate(colorspace, components);

    CGContextSetFillColorWithColor(cg, color);
    CGColorRelease(color);
	CGColorSpaceRelease(colorspace);

    CGContextFillRect(cg, bounds);

    CGRect r = CGRectMake(0, 0, 640, 480);
    
    CGContextSaveGState(cg);
    CGContextTranslateCTM(cg, 0, r.size.height);
    CGContextScaleCTM(cg, 1, -1);
    
    CGContextDrawImage(cg, r, gImage);
    
    CGContextRestoreGState(cg);
}


