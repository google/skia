/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef gm_knowledge_DEFINED
#define gm_knowledge_DEFINED

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/**
A structure representing an image.  pix should either be nullptr (representing
a missing image) or point to a block of memory width*height in size.

Each pixel is an un-pre-multiplied RGBA color:
    void set_color(GMK_ImageData* data, int x, int y,
                   unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
        data->pix[x + data->width * y] = (r << 0) | (g << 8) | (b << 16) | (a << 24);
    }
 */
typedef struct {
    const uint32_t* pix;
    int width;
    int height;
} GMK_ImageData;

/**
Check if the given test image matches the expected results.

@param data     the image
@param gm_name  the name of the rendering test that produced the image

@return 0 if the test passes, otherwise a positive number representing how
         badly it failed.
 */
float GMK_Check(GMK_ImageData data, const char* gm_name);

/**
Check to see if the given test has expected results.

@param gm_name  the name of a rendering test.

@return true of expected results are known for the given test.
*/
bool GMK_IsGoodGM(const char* gm_name);

#ifdef __cplusplus
}
#endif

#endif  // gm_knowledge_DEFINED
