/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPathEffect.h"
#include "SkShadowPaintFilterCanvas.h"

#ifdef SK_EXPERIMENTAL_SHADOWING

SkShadowPaintFilterCanvas::SkShadowPaintFilterCanvas(SkCanvas *canvas)
        : SkPaintFilterCanvas(canvas) {
    fShadowParams.fShadowRadius = 0.0f;
    fShadowParams.fType = SkShadowParams::kNoBlur_ShadowType;
    fShadowParams.fBiasingConstant = 0.0f;
    fShadowParams.fMinVariance = 0.0f;
}

// TODO use a shader instead
bool SkShadowPaintFilterCanvas::onFilter(SkTCopyOnFirstWrite<SkPaint>* paint, Type type) const {
    if (*paint) {
        int z = this->getZ();
        SkASSERT(z <= 0xFF && z >= 0x00);

        SkPaint newPaint;
        newPaint.setPathEffect(sk_ref_sp<SkPathEffect>((*paint)->getPathEffect()));

        SkColor color = 0xFF000000; // init color to opaque black
        color |= z; // Put the index into the blue component

        if (fShadowParams.fType == SkShadowParams::kVariance_ShadowType) {
            int z2 = z * z;
            if (z2 > 255 * 256) {
                color |= 0xff00;
            } else {
                // Let's only store the more significant bits of z2 to save space.
                // In practice, this should barely impact shadow blur quality.
                color |= z2 & 0x0000ff00;
            }
        }
        newPaint.setColor(color);

        *paint->writable() = newPaint;
    }

    return true;
}

SkISize SkShadowPaintFilterCanvas::ComputeDepthMapSize(const SkLights::Light& light, int maxDepth,
                                                       int width, int height) {
    if (light.type() != SkLights::Light::kDirectional_LightType) {
        // Calculating the right depth map size for point lights is complex,
        // as it depends on the max depth, the max depth delta, the location
        // of the point light and the shapes, etc... If we take upper bounds
        // on those metrics, the shadow map will be pretty big in any case.
        // Thus, just using 4x the width and height seems to work for most scenes.
        return {width * 4, height * 4};
    }

    int dMapWidth = SkMin32(maxDepth * fabs(light.dir().fX) + width,
                            width * 2);
    int dMapHeight = SkMin32(maxDepth * fabs(light.dir().fY) + height,
                             height * 2);
    return SkISize::Make(dMapWidth, dMapHeight);
}

void SkShadowPaintFilterCanvas::setShadowParams(const SkShadowParams &params) {
    fShadowParams = params;
}

void SkShadowPaintFilterCanvas::onDrawPicture(const SkPicture *picture, const SkMatrix *matrix,
                                              const SkPaint *paint) {
    SkTCopyOnFirstWrite<SkPaint> filteredPaint(paint);
    if (this->onFilter(&filteredPaint, kPicture_Type)) {
        SkCanvas::onDrawPicture(picture, matrix, filteredPaint);
    }
}

void SkShadowPaintFilterCanvas::updateMatrix() {
    //  It is up to the user to set the 0th light in fLights to
    //  the light the want to render the depth map with.
    if (this->fLights->light(0).type() == SkLights::Light::kDirectional_LightType) {
        const SkVector3& lightDir = this->fLights->light(0).dir();
        SkScalar x = lightDir.fX * this->getZ();
        SkScalar y = lightDir.fY * this->getZ();

        this->translate(x, y);
    } else if (this->fLights->light(0).type() == SkLights::Light::kPoint_LightType) {
        SkISize size = this->getBaseLayerSize();

        SkPoint3 lightPos = this->fLights->light(0).pos();

        // shadow maps for point lights are 4x the size of the diffuse map, by experimentation
        // (see SPFCanvas::ComputeDepthMapSize())
        SkScalar diffuseHeight = size.fHeight / 4.0f;

        // move point light with canvas's CTM
        SkPoint lightPoint = SkPoint::Make(lightPos.fX, diffuseHeight - lightPos.fY);
        SkMatrix mat = this->getTotalMatrix();
        if (mat.invert(&mat)) {
            mat.mapPoints(&lightPoint, 1);
        }
        lightPoint.set(lightPoint.fX, diffuseHeight - lightPoint.fY);

        // center the shadow map
        // note: the 3/8 constant is specific to the 4.0 depth map size multiplier
        mat = this->getTotalMatrix();
        mat.postTranslate(size.width() * 0.375f, size.height() * 0.375f);
        this->setMatrix(mat);

        // project shapes onto canvas as shadows
        SkScalar scale = (lightPos.fZ) / (lightPos.fZ - this->getZ());
        this->scale(scale, scale);

        this->translate(-lightPoint.fX * this->getZ() /
                        ((lightPos.fZ - this->getZ()) * scale),
                        -(diffuseHeight - lightPoint.fY) * this->getZ() /
                        ((lightPos.fZ - this->getZ()) * scale));
    }
}

void SkShadowPaintFilterCanvas::onDrawPaint(const SkPaint &paint) {
    this->save();
    this->updateMatrix();
    this->INHERITED::onDrawPaint(paint);
    this->restore();
}

void SkShadowPaintFilterCanvas::onDrawPoints(PointMode mode, size_t count, const SkPoint pts[],
                                             const SkPaint &paint) {
    this->save();
    this->updateMatrix();
    this->INHERITED::onDrawPoints(mode, count, pts, paint);
    this->restore();
}

void SkShadowPaintFilterCanvas::onDrawRect(const SkRect &rect, const SkPaint &paint) {
    this->save();
    this->updateMatrix();
    this->INHERITED::onDrawRect(rect, paint);
    this->restore();
}

void SkShadowPaintFilterCanvas::onDrawRRect(const SkRRect &rrect, const SkPaint &paint) {
    this->save();
    this->updateMatrix();
    this->INHERITED::onDrawRRect(rrect, paint);
    this->restore();
}

void SkShadowPaintFilterCanvas::onDrawDRRect(const SkRRect &outer, const SkRRect &inner,
                  const SkPaint &paint) {
    this->save();
    this->updateMatrix();
    this->INHERITED::onDrawDRRect(outer, inner, paint);
    this->restore();
}

void SkShadowPaintFilterCanvas::onDrawOval(const SkRect &rect, const SkPaint &paint) {
    this->save();
    this->updateMatrix();
    this->INHERITED::onDrawOval(rect, paint);
    this->restore();
}

void SkShadowPaintFilterCanvas::onDrawArc(const SkRect &rect, SkScalar startAngle,
                                          SkScalar sweepAngle, bool useCenter,
                                          const SkPaint &paint) {
    this->save();
    this->updateMatrix();
    this->INHERITED::onDrawArc(rect, startAngle, sweepAngle, useCenter, paint);
    this->restore();
}

void SkShadowPaintFilterCanvas::onDrawPath(const SkPath &path, const SkPaint &paint) {
    this->save();
    this->updateMatrix();
    this->INHERITED::onDrawPath(path, paint);
    this->restore();
}

void SkShadowPaintFilterCanvas::onDrawBitmap(const SkBitmap &bm, SkScalar left, SkScalar top,
                                             const SkPaint *paint) {
    this->save();
    this->updateMatrix();
    this->INHERITED::onDrawBitmap(bm, left, top, paint);
    this->restore();
}

void SkShadowPaintFilterCanvas::onDrawBitmapRect(const SkBitmap &bm, const SkRect *src,
                                                 const SkRect &dst, const SkPaint *paint,
                                                 SrcRectConstraint constraint) {
    this->save();
    this->updateMatrix();
    this->INHERITED::onDrawBitmapRect(bm, src, dst, paint, constraint);
    this->restore();
}

void SkShadowPaintFilterCanvas::onDrawBitmapNine(const SkBitmap &bm, const SkIRect &center,
                                                 const SkRect &dst, const SkPaint *paint) {
    this->save();
    this->updateMatrix();
    this->INHERITED::onDrawBitmapNine(bm, center, dst, paint);
    this->restore();
}

void SkShadowPaintFilterCanvas::onDrawImage(const SkImage *image, SkScalar left,
                                            SkScalar top, const SkPaint *paint) {
    this->save();
    this->updateMatrix();
    this->INHERITED::onDrawImage(image, left, top, paint);
    this->restore();
}

void SkShadowPaintFilterCanvas::onDrawImageRect(const SkImage *image, const SkRect *src,
                                                const SkRect &dst, const SkPaint *paint,
                                                SrcRectConstraint constraint) {
    this->save();
    this->updateMatrix();
    this->INHERITED::onDrawImageRect(image, src, dst, paint, constraint);
    this->restore();
}

void SkShadowPaintFilterCanvas::onDrawImageNine(const SkImage *image, const SkIRect &center,
                                                const SkRect &dst, const SkPaint *paint) {
    this->save();
    this->updateMatrix();
    this->INHERITED::onDrawImageNine(image, center, dst, paint);
    this->restore();
}


void SkShadowPaintFilterCanvas::onDrawVertices(VertexMode vmode, int vertexCount,
                                               const SkPoint vertices[], const SkPoint texs[],
                                               const SkColor colors[], SkXfermode *xmode,
                                               const uint16_t indices[], int indexCount,
                                               const SkPaint &paint) {
    this->save();
    this->updateMatrix();
    this->INHERITED::onDrawVertices(vmode, vertexCount, vertices, texs, colors,
                                    xmode, indices, indexCount, paint);
    this->restore();
}

void SkShadowPaintFilterCanvas::onDrawPatch(const SkPoint cubics[], const SkColor colors[],
                                            const SkPoint texCoords[], SkXfermode *xmode,
                                            const SkPaint &paint) {
    this->save();
    this->updateMatrix();
    this->INHERITED::onDrawPatch(cubics, colors, texCoords, xmode, paint);
    this->restore();
}

void SkShadowPaintFilterCanvas::onDrawText(const void *text, size_t byteLength, SkScalar x,
                                           SkScalar y, const SkPaint &paint) {
    this->save();
    this->updateMatrix();
    this->INHERITED::onDrawText(text, byteLength, x, y, paint);
    this->restore();
}

void SkShadowPaintFilterCanvas::onDrawPosText(const void *text, size_t byteLength,
                                              const SkPoint pos[], const SkPaint &paint) {
    this->save();
    this->updateMatrix();
    this->INHERITED::onDrawPosText(text, byteLength, pos, paint);
    this->restore();
}

void SkShadowPaintFilterCanvas::onDrawPosTextH(const void *text, size_t byteLength,
                                               const SkScalar xpos[],
                                               SkScalar constY, const SkPaint &paint) {
    this->save();
    this->updateMatrix();
    this->INHERITED::onDrawPosTextH(text, byteLength, xpos, constY, paint);
    this->restore();
}

void SkShadowPaintFilterCanvas::onDrawTextOnPath(const void *text, size_t byteLength,
                                                 const SkPath &path, const SkMatrix *matrix,
                                                 const SkPaint &paint) {
    this->save();
    this->updateMatrix();
    this->INHERITED::onDrawTextOnPath(text, byteLength, path, matrix, paint);
    this->restore();
}

void SkShadowPaintFilterCanvas::onDrawTextRSXform(const void *text, size_t byteLength,
                                                  const SkRSXform xform[], const SkRect *cull,
                                                  const SkPaint &paint) {
    this->save();
    this->updateMatrix();
    this->INHERITED::onDrawTextRSXform(text, byteLength, xform, cull, paint);
    this->restore();
}

void SkShadowPaintFilterCanvas::onDrawTextBlob(const SkTextBlob *blob, SkScalar x, SkScalar y,
                                               const SkPaint &paint) {
    this->save();
    this->updateMatrix();
    this->INHERITED::onDrawTextBlob(blob, x, y, paint);
    this->restore();
}

#endif
