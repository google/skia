#include "Resources.h"
#include "gm.h"

DEF_SIMPLE_GM(small_image, canvas, 8, 8) {
    SkBitmap bitmap;
    if (GetResourceAsBitmap("randPixels.png", &bitmap)) {
        canvas->drawBitmap(bitmap, 0.0f, 0.0f);
    } else {
        SkDebugf("\nCould not decode resource.\n");
    }
}
