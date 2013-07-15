
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

#define NUM_IT 100
#define ON  0xFF000000 // black pixel
#define OFF 0xFFFFFFFF // white pixel

class SkBitmap;

static void fillRandomBits( int chars, char* bits ){
    SkMWCRandom rand(SkTime::GetMSecs());

    for (int i = 0; i < chars; ++i){
        bits[i] = rand.nextU();
    }
}

static int getBit( const char* buffer, int x ) {
    int byte = x >> 3;
    int bit = x & 7;

    return buffer[byte] & (128 >> bit);
}
/* // useful for debugging errors
   #include <iostream>
static void printBits( const char* bits, int w, int h) {

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x){
            bool bit = getBit(&bits[y], x)!=0;
            std::cout << bit;
            }
        std::cout << std::endl;
    }
}

static void printBmp( SkBitmap* bmp, int w, int h){

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int d = *bmp->getAddr32(x,y);
            if (d == -1)
                std::cout << 0;
            else
                std::cout << 1;
            }
        std::cout << std::endl;
    }
    }
*/

static void bin2SkBitmap(const char* bin_bmp, SkBitmap* sk_bmp,
                         int h, int w, int rowBytes){
    //init the SkBitmap
    sk_bmp->setConfig(SkBitmap::kARGB_8888_Config, w, h);
    sk_bmp->allocPixels();

    for (int y = 0; y < h; ++y) { // for every row

        const char* curLine = &bin_bmp[y * rowBytes];
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
            REPORTER_ASSERT( reporter, *bmp1->getAddr32(x,y) == *bmp2->getAddr32(x,y) );
        }
    }
    return true;
}

static void test_path_eq(skiatest::Reporter* reporter, const SkPath* path,
                      const SkBitmap* truth, int w, int h){
    // make paint
    SkPaint bmpPaint;
    bmpPaint.setAntiAlias(true);  // Black paint for bitmap
    bmpPaint.setStyle(SkPaint::kFill_Style);
    bmpPaint.setColor(SK_ColorBLACK);

    // make bmp
    SkBitmap bmp;
    bmp.setConfig(SkBitmap::kARGB_8888_Config, w, h);
    bmp.allocPixels();
    SkCanvas canvas(bmp);
    canvas.clear(0xFFFFFFFF);
    canvas.drawPath(*path, bmpPaint);

    // test bmp
    test_bmp(reporter, truth, &bmp, h, w);
}

static void test_path(skiatest::Reporter* reporter, const SkBitmap* truth,
                            const char* bin_bmp, int w, int h, int stride){
    // make path
    SkPath path;
    SkPathUtils::BitsToPath_Path(&path, bin_bmp, w, h, stride);

    //test for correctness
    test_path_eq(reporter, &path, truth, w, h);
}

static void test_region(skiatest::Reporter* reporter, const SkBitmap* truth,
                            const char* bin_bmp, int w, int h, int stride){
    //generate bitmap
    SkPath path;
    SkPathUtils::BitsToPath_Region(&path, bin_bmp, w, h, stride);

    //test for correctness
    test_path_eq(reporter, &path, truth, w, h);
}

static void TestPathUtils(skiatest::Reporter* reporter) {
    const int w[4] = {4, 8, 12, 16};
//    const int w[1] = {8};
    const int h = 8, rowBytes = 4;

    char bits[ h * rowBytes ];
    static char* bin_bmp = &bits[0];

    //loop to run randomized test lots of times
    for (int it = 0; it < NUM_IT; ++it)
    {
        // generate a random binary bitmap
        fillRandomBits( h * rowBytes, bin_bmp); // generate random bitmap

        // for each bitmap width, use subset of binary bitmap
        for (unsigned int i = 0; i < SK_ARRAY_COUNT(w); ++i) {
            // generate truth bitmap
            SkBitmap bmpTruth;
            bin2SkBitmap(bin_bmp, &bmpTruth, h, w[i], rowBytes);

            test_path(reporter, &bmpTruth, bin_bmp, w[i], h, rowBytes);
            test_region(reporter, &bmpTruth, bin_bmp, w[i], h, rowBytes);
        }
    }
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("PathUtils", PathUtils, TestPathUtils)
