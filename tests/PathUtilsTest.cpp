/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkPathUtils.h"
#include "SkRandom.h"
#include "SkTime.h"

const int kNumIt = 100;

static void fill_random_bits(int chars, char* bits){
    SkRandom rand(SkTime::GetMSecs());

    for (int i = 0; i < chars; ++i){
        bits[i] = rand.nextU();
    }
}

static int get_bit(const char* buffer, int x) {
    int byte = x >> 3;
    int bit = x & 7;

    return buffer[byte] & (128 >> bit);
}

/* // useful for debugging errors
   #include <iostream>
static void print_bits( const char* bits, int w, int h) {

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x){
            bool bit = get_bit(&bits[y], x)!=0;
            std::cout << bit;
            }
        std::cout << std::endl;
    }
}

static void print_bmp( SkBitmap* bmp, int w, int h){

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

static void binary_to_skbitmap(const char* bin_bmp, SkBitmap* sk_bmp,
                         int w, int h, int rowBytes){
    //init the SkBitmap
    sk_bmp->setConfig(SkBitmap::kARGB_8888_Config, w, h);
    sk_bmp->allocPixels();

    for (int y = 0; y < h; ++y) { // for every row

        const char* curLine = &bin_bmp[y * rowBytes];
        for (int x = 0; x < w; ++x) {// for every pixel
            if (get_bit(curLine, x)) {
                *sk_bmp->getAddr32(x,y) = SK_ColorBLACK;
            }
            else {
                *sk_bmp->getAddr32(x,y) = SK_ColorWHITE;
            }
        }
    }
}

static bool test_bmp(skiatest::Reporter* reporter,
                      const SkBitmap* bmp1, const SkBitmap* bmp2,
                      int w, int h) {
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
    canvas.clear(SK_ColorWHITE);
    canvas.drawPath(*path, bmpPaint);

    // test bmp
    test_bmp(reporter, truth, &bmp, w, h);
}

static void test_path(skiatest::Reporter* reporter, const SkBitmap* truth,
                            const char* bin_bmp, int w, int h, int rowBytes){
    // make path
    SkPath path;
    SkPathUtils::BitsToPath_Path(&path, bin_bmp, w, h, rowBytes);

    //test for correctness
    test_path_eq(reporter, &path, truth, w, h);
}

static void test_region(skiatest::Reporter* reporter, const SkBitmap* truth,
                            const char* bin_bmp, int w, int h, int rowBytes){
    //generate bitmap
    SkPath path;
    SkPathUtils::BitsToPath_Region(&path, bin_bmp, w, h, rowBytes);

    //test for correctness
    test_path_eq(reporter, &path, truth, w, h);
}

DEF_TEST(PathUtils, reporter) {
    const int w[] = {4, 8, 12, 16};
    const int h = 8, rowBytes = 4;

    char bits[ h * rowBytes ];
    static char* binBmp = &bits[0];

    //loop to run randomized test lots of times
    for (int it = 0; it < kNumIt; ++it)
    {
        // generate a random binary bitmap
        fill_random_bits( h * rowBytes, binBmp); // generate random bitmap

        // for each bitmap width, use subset of binary bitmap
        for (unsigned int i = 0; i < SK_ARRAY_COUNT(w); ++i) {
            // generate truth bitmap
            SkBitmap bmpTruth;
            binary_to_skbitmap(binBmp, &bmpTruth, w[i], h, rowBytes);

            test_path(reporter, &bmpTruth, binBmp, w[i], h, rowBytes);
            test_region(reporter, &bmpTruth, binBmp, w[i], h, rowBytes);
        }
    }
}
