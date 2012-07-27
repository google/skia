
#ifndef SkImagePriv_DEFINED
#define SkImagePriv_DEFINED

#include "SkBitmap.h"
#include "SkImage.h"

extern SkBitmap::Config SkImageInfoToBitmapConfig(const SkImage::Info&,
                                                  bool* isOpaque);

extern int SkImageBytesPerPixel(SkImage::ColorType);

extern bool SkBitmapToImageInfo(const SkBitmap&, SkImage::Info*);
extern SkImage* SkNewImageFromPixelRef(const SkImage::Info&, SkPixelRef*,
                                       size_t rowBytes);

/**
 *  Examines the bitmap to decide if it can share the existing pixelRef, or
 *  if it needs to make a deep-copy of the pixels
 */
extern SkImage* SkNewImageFromBitmap(const SkBitmap&);

static inline size_t SkImageMinRowBytes(const SkImage::Info& info) {
    return info.fWidth * SkImageBytesPerPixel(info.fColorType);
}

#endif
