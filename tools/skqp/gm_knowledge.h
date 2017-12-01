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

#include <stdbool.h>
#include <stdint.h>

/**
A structure representing an image.  pix should either be nullptr (representing
a missing image) or point to a block of memory width*height in size.

Each pixel is an un-pre-multiplied RGBA color:
    void set_color(GMKB_ImageData* data, int x, int y,
                   unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
        data->pix[x + data->width * y] = (r << 0) | (g << 8) | (b << 16) | (a << 24);
    }
 */
typedef struct {
    const uint32_t* pix;
    int width;
    int height;
} GMKB_ImageData;

enum GMKB_Error {
    GMKB_Error_none = 0,      /**< No error. */
    GMKB_Error_bad_input = 1, /**< Error with the given image data. */
    GMKB_Error_bad_data = 2,  /**< Error with the given gmkb data directory. */
};

/**
Check if the given test image matches the expected results.

@param data                   the image
@param gm_name                the name of the rendering test that produced the image
@param backend                the name of the backend (optional)
@param gmkb_directory_path    GM KnowledgeBase data file location
@param report_directory_path  (optional) locatation to write report to.
@param error_out              (optional) error return code.

@return 0 if the test passes, otherwise a positive number representing how
         badly it failed.  Return FLT_MAX on error.
 */

float GMKB_Check(GMKB_ImageData image,
                 const char* name,
                 const char* backend,
                 const char* gmkb_directory_path,
                 const char* report_directory_path,
                 GMKB_Error* error_out);

/**
Check to see if the given test has expected results.

@param gm_name  the name of a rendering test.
@param gmkb_directory_path    GM KnowledgeBase data file location

@return true of expected results are known for the given test.
*/
bool GMKB_IsGoodGM(const char* name, const char* gmkb_directory_path);

#ifdef __cplusplus
}
#endif

#endif  // gm_knowledge_DEFINED
