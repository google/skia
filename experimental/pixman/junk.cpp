
extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <config.h>
#include "pixman-private.h"
#include "utils.h"
#include "gtk-utils.h"

}

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkGraphics.h"
#include "SkPaint.h"
#import "SkWindow.h"

bool DrawPixman(SkCanvas* canvas, int step, bool useOld);
SkCanvas* canvas;

extern "C" {

void*
pixbuf_from_argb32 (uint32_t *bits,
            int width,
            int height,
            int stride)
{
    SkBitmap* bitmap = new SkBitmap;
    bitmap->setConfig(SkBitmap::kARGB_8888_Config, width, height);
    bitmap->allocPixels();

    int p_stride = bitmap->rowBytes();
    uint32_t *p_bits = bitmap->getAddr32(0, 0);
    int i;

    for (i = 0; i < height; ++i)
    {
    uint32_t *src_row = &bits[i * (stride / 4)];
    uint32_t *dst_row = p_bits + i * (p_stride / 4);

    a8r8g8b8_to_rgba_np (dst_row, src_row, width);
    }
    return (void*) bitmap;
}


void show_image (pixman_image_t *image) {
    int width, height;
    pixman_format_code_t format;
    pixman_image_t *copy;

    width = pixman_image_get_width (image);
    height = pixman_image_get_height (image);


    format = pixman_image_get_format (image);

    /* Three cases:
     *
     *  - image is a8r8g8b8_sRGB: we will display without modification
     *    under the assumption that the monitor is sRGB
     *
     *  - image is a8r8g8b8: we will display without modification
     *    under the assumption that whoever created the image
     *    probably did it wrong by using sRGB inputs
     *
     *  - other: we will convert to a8r8g8b8 under the assumption that
     *    whoever created the image probably did it wrong.
     */
    switch (format)
    {
    case PIXMAN_a8r8g8b8_sRGB:
    case PIXMAN_a8r8g8b8:
    copy = pixman_image_ref (image);
    break;

    default:
    copy = pixman_image_create_bits (PIXMAN_a8r8g8b8,
                     width, height, NULL, -1);
    pixman_image_composite32 (PIXMAN_OP_SRC,
                  image, NULL, copy,
                  0, 0, 0, 0, 0, 0,
                  width, height);
    break;
    }

    SkBitmap* bitmap = (SkBitmap*) pixbuf_from_argb32 (pixman_image_get_data (copy),
                 width, height,
                 pixman_image_get_stride (copy));
    canvas->drawBitmap(*bitmap, 0, 0);
    delete bitmap;
}

}

bool DrawPixman(SkCanvas* c, int step, bool usePixman) {
    canvas = c;
    switch(step) {
        case 0:
            checkerboard_main(0, NULL);
            break;
        default:
            alpha_main(0, NULL);
            break;
    }
    return true;
}
