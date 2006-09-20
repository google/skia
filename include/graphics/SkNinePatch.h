#ifndef SkNinePatch_DEFINED
#define SkNinePatch_DEFINED

#include "SkRect.h"

class SkBitmap;
class SkCanvas;
class SkPaint;

class SkNinePatch {
public:
    static void Draw(SkCanvas* canvas, const SkRect& dst,
                     const SkBitmap& bitmap, const SkRect16& margin,
                     const SkPaint* paint = NULL);

    static void Draw(SkCanvas* canvas, const SkRect& dst,
                     const SkBitmap& bitmap, int cx, int cy,
                     const SkPaint* paint = NULL);
};

#endif
