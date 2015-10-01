/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGrPriv_DEFINED
#define SkGrPriv_DEFINED

#include "GrTypes.h"
#include "GrTextureAccess.h"
#include "SkImageInfo.h"
#include "SkXfermode.h"

class GrCaps;
class GrContext;
class GrFragmentProcessor;
class GrPaint;
class GrUniqueKey;
class SkPaint;
class SkMatrix;
struct SkIRect;

struct SkGrStretch {
    enum Type {
        kNone_Type,
        kBilerp_Type,
        kNearest_Type
    } fType;
    int fWidth;
    int fHeight;
};

GrTextureParams GrImageUsageToTextureParams(SkImageUsageType);

/**
 *  Our key includes the offset, width, and height so that bitmaps created by extractSubset()
 *  are unique.
 *
 *  The imageID is in the shared namespace (see SkNextID::ImageID()
 *      - SkBitmap/SkPixelRef
 *      - SkImage
 *      - SkImageGenerator
 *
 *  Note: width/height must fit in 16bits for this impl.
 */
void GrMakeKeyFromImageID(GrUniqueKey* key, uint32_t imageID, const SkIRect& imageBounds,
                          const GrCaps&, SkImageUsageType);

/**
 *  Given an "unstretched" key, and a stretch rec, produce a stretched key.
 */
bool GrMakeStretchedKey(const GrUniqueKey& origKey, const SkGrStretch&, GrUniqueKey* stretchedKey);

/** Converts an SkPaint to a GrPaint for a given GrContext. The matrix is required in order
    to convert the SkShader (if any) on the SkPaint. The primitive itself has no color. */
bool SkPaintToGrPaint(GrContext*,
                      const SkPaint& skPaint,
                      const SkMatrix& viewM,
                      GrPaint* grPaint);

/** Same as above but ignores the SkShader (if any) on skPaint. */
bool SkPaintToGrPaintNoShader(GrContext* context,
                              const SkPaint& skPaint,
                              GrPaint* grPaint);

/** Replaces the SkShader (if any) on skPaint with the passed in GrFragmentProcessor. The processor
    should expect an unpremul input color and produce a premultiplied output color. There is
    no primitive color. */
bool SkPaintToGrPaintReplaceShader(GrContext*,
                                   const SkPaint& skPaint,
                                   const GrFragmentProcessor* shaderFP,
                                   GrPaint* grPaint);

/** Blends the SkPaint's shader (or color if no shader) with the color which specified via a
    GrBatch's GrPrimitiveProcesssor. Currently there is a bool param to indicate whether the
    primitive color is the dst or src color to the blend in order to work around differences between
    drawVertices and drawAtlas. */
bool SkPaintToGrPaintWithXfermode(GrContext* context,
                                  const SkPaint& skPaint,
                                  const SkMatrix& viewM,
                                  SkXfermode::Mode primColorMode,
                                  bool primitiveIsSrc,
                                  GrPaint* grPaint);

/** This is used when there is a primitive color, but the shader should be ignored. Currently,
    the expectation is that the primitive color will be premultiplied, though it really should be
    unpremultiplied so that interpolation is done in unpremul space. The paint's alpha will be
    applied to the primitive color after interpolation. */
inline bool SkPaintToGrPaintWithPrimitiveColor(GrContext* context, const SkPaint& skPaint,
                                               GrPaint* grPaint) {
    return SkPaintToGrPaintWithXfermode(context, skPaint, SkMatrix::I(), SkXfermode::kDst_Mode,
                                        false, grPaint);
}

bool GrTextureUsageSupported(const GrCaps&, int width, int height, SkImageUsageType);

#endif
