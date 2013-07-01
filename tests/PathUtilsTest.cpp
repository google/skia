
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkPathUtils.h"
#include "SkRandom.h"
#include "SkTime.h"

#define NUM_IT 1000
#define ON  0xFF000000 // black pixel
#define OFF 0x00000000 // transparent pixel    

class SkBitmap;

//this function is redefined for sample, test, and bench. is there anywhere
// I can put it to avoid code duplcation?           
static void fillRandomBits( int chars, char* bits ){
    SkTime time;
    SkMWCRandom rand = SkMWCRandom( time.GetMSecs() );

    for (int i = 0; i < chars; ++i){
        bits[i] = rand.nextU();
    }
}

//also defined within PathUtils.cpp, but not in scope here. Anyway to call it
// without re-defining it?
static int getBit( const char* buffer, int x ) {
    int byte = x >> 3;
    int bit = x & 7;

    return buffer[byte] & (1 << bit);
}

static void bin2SkBitmap(const char* bin_bmp, SkBitmap* sk_bmp,
                               int h, int w, int stride){
    //init the SkBitmap
    sk_bmp->setConfig(SkBitmap::kARGB_8888_Config, w, h);
    sk_bmp->allocPixels();

    for (int y = 0; y < h; ++y) { // for every row

        const char* curLine = &bin_bmp[y * stride];
        for (int x = 0; x < w; ++x) {// for every pixel
            if (getBit(curLine, x)) {
                *sk_bmp->getAddr32(x,y) = ON;
            }
            else {
                *sk_bmp->getAddr32(x,y) = OFF;
            }
        }
    }
}

static bool test_bmp(skiatest::Reporter* reporter,
                      const SkBitmap* bmp1, const SkBitmap* bmp2,
                      int h, int w) {
    for (int y = 0; y < h; ++y) { // loop through all pixels
        for (int x = 0; x < w; ++x) {
            REPORTER_ASSERT( reporter, *bmp1->getAddr32(x,y) == *bmp1->getAddr32(x,y) );
        }
    }
    return true;
}

static void test_path_eq(skiatest::Reporter* reporter, const SkPath* path,
                      const SkBitmap* truth, int h, int w){
    // make paint
    SkPaint bmpPaint;
    bmpPaint.setAntiAlias(true);  // Black paint for bitmap
    bmpPaint.setStyle(SkPaint::kFill_Style);
    bmpPaint.setColor(SK_ColorBLACK);
    
    // make bmp
    SkBitmap bmp;
    bmp.setConfig(SkBitmap::kARGB_8888_Config, w, h);
    bmp.allocPixels();
    SkCanvas(bmp).drawPath(*path, bmpPaint);

    // test bmp
    test_bmp(reporter, &bmp, truth, h, w);
}

static void test_path(skiatest::Reporter* reporter, const SkBitmap* truth,
                            const char* bin_bmp, int h, int w, int stride){
    // make path
    SkPath path;
    SkPathUtils::BitsToPath_Path(&path, bin_bmp, h, w, stride);
    
    //test for correctness
    test_path_eq(reporter, &path, truth, h, w);
}

static void test_region(skiatest::Reporter* reporter, const SkBitmap* truth,
                            const char* bin_bmp, int h, int w, int stride){
    //generate bitmap
    SkPath path;
    SkPathUtils::BitsToPath_Region(&path, bin_bmp, h, w, stride);
    
    //test for correctness
    test_path_eq(reporter, &path, truth, h, w);
}

#define W_tests 4

static void TestPathUtils(skiatest::Reporter* reporter) {
    const int w[W_tests] = {4, 8, 12, 16};
    const int h = 8, stride = 4;

    char bits[ h * stride ];
    static char* bin_bmp = &bits[0];

    //loop to run randomized test lots of times
    for (int it = 0; it < NUM_IT; ++it)
    {
        // generate a random binary bitmap
        fillRandomBits( h * stride, bin_bmp); // generate random bitmap    
    
        // for each bitmap width, use subset of binary bitmap
        for (int i = 0; i < W_tests; ++i) {
            // generate truth bitmap
            SkBitmap bmpTruth;
            bin2SkBitmap(bin_bmp, &bmpTruth, h, w[i], stride);

            test_path(reporter, &bmpTruth, bin_bmp, h, w[i], stride);
            test_region(reporter, &bmpTruth, bin_bmp, h, w[i], stride);
        }
    }
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("PathUtils", PathUtils, TestPathUtils)
