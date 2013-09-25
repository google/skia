/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#import <CoreGraphics/CoreGraphics.h>
#include <CoreGraphics/CGColorSpace.h>
#import <UIKit/UIKit.h>

#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkMovie.h"
#include "SkStream_NSData.h"

class SkImageDecoder_iOS : public SkImageDecoder {
protected:
    virtual bool onDecode(SkStream* stream, SkBitmap* bm, Mode);
};

#define BITMAP_INFO (kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast)

bool SkImageDecoder_iOS::onDecode(SkStream* stream, SkBitmap* bm, Mode mode) {

    NSData* data = NSData_dataWithStream(stream);

    UIImage* uimage = [UIImage imageWithData:data];
    
    const int width = uimage.size.width;
    const int height = uimage.size.height;
    bm->setConfig(SkBitmap::kARGB_8888_Config, width, height);
    if (SkImageDecoder::kDecodeBounds_Mode == mode) {
        return true;
    }
    
    if (!this->allocPixelRef(bm, NULL)) {
        return false;
    }
    
    bm->lockPixels();
    bm->eraseColor(0);
    
    CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
    CGContextRef cg = CGBitmapContextCreate(bm->getPixels(), width, height,
                                            8, bm->rowBytes(), cs, BITMAP_INFO);
    CGContextDrawImage(cg, CGRectMake(0, 0, width, height), uimage.CGImage);
    CGContextRelease(cg);
    CGColorSpaceRelease(cs);
    
    bm->unlockPixels();
    return true;
}

/////////////////////////////////////////////////////////////////////////

SkImageDecoder* SkImageDecoder::Factory(SkStreamRewindable* stream) {
    return new SkImageDecoder_iOS;
}

SkMovie* SkMovie::DecodeStream(SkStreamRewindable* stream) {
    return NULL;
}

SkImageEncoder* SkImageEncoder::Create(Type t) {
    return NULL;
}

