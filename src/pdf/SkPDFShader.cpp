/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkPDFShader.h"

#include "SkData.h"
#include "SkPDFDocument.h"
#include "SkPDFDevice.h"
#include "SkPDFDocumentPriv.h"
#include "SkPDFFormXObject.h"
#include "SkPDFGradientShader.h"
#include "SkPDFGraphicState.h"
#include "SkPDFResourceDict.h"
#include "SkPDFUtils.h"
#include "SkScalar.h"
#include "SkStream.h"
#include "SkSurface.h"
#include "SkTemplates.h"


static void draw_image_matrix(SkCanvas* canvas, const SkImage* img,
                              const SkMatrix& matrix, const SkPaint& paint) {
    SkAutoCanvasRestore acr(canvas, true);
    canvas->concat(matrix);
    canvas->drawImage(img, 0, 0, &paint);
}

static void draw_bitmap_matrix(SkCanvas* canvas, const SkBitmap& bm,
                               const SkMatrix& matrix, const SkPaint& paint) {
    SkAutoCanvasRestore acr(canvas, true);
    canvas->concat(matrix);
    canvas->drawBitmap(bm, 0, 0, &paint);
}

static SkPDFIndirectReference make_image_shader(SkPDFDocument* doc,
                                                const SkPDFImageShaderKey& key,
                                                SkImage* image) {
    SkASSERT(image);

    // The image shader pattern cell will be drawn into a separate device
    // in pattern cell space (no scaling on the bitmap, though there may be
    // translations so that all content is in the device, coordinates > 0).

    // Map clip bounds to shader space to ensure the device is large enough
    // to handle fake clamping.
    SkMatrix finalMatrix = key.fCanvasTransform;
    finalMatrix.preConcat(key.fShaderTransform);
    SkRect deviceBounds = SkRect::Make(key.fBBox);
    if (!SkPDFUtils::InverseTransformBBox(finalMatrix, &deviceBounds)) {
        return SkPDFIndirectReference();
    }

    SkRect bitmapBounds = SkRect::Make(image->bounds());

    // For tiling modes, the bounds should be extended to include the bitmap,
    // otherwise the bitmap gets clipped out and the shader is empty and awful.
    // For clamp modes, we're only interested in the clip region, whether
    // or not the main bitmap is in it.
    SkShader::TileMode tileModes[2];
    tileModes[0] = key.fImageTileModes[0];
    tileModes[1] = key.fImageTileModes[1];
    if (tileModes[0] != SkShader::kClamp_TileMode ||
            tileModes[1] != SkShader::kClamp_TileMode) {
        deviceBounds.join(bitmapBounds);
    }

    SkISize patternDeviceSize = {SkScalarCeilToInt(deviceBounds.width()),
                                 SkScalarCeilToInt(deviceBounds.height())};
    auto patternDevice = sk_make_sp<SkPDFDevice>(patternDeviceSize, doc);
    SkCanvas canvas(patternDevice);

    SkRect patternBBox = SkRect::Make(image->bounds());

    // Translate the canvas so that the bitmap origin is at (0, 0).
    canvas.translate(-deviceBounds.left(), -deviceBounds.top());
    patternBBox.offset(-deviceBounds.left(), -deviceBounds.top());
    // Undo the translation in the final matrix
    finalMatrix.preTranslate(deviceBounds.left(), deviceBounds.top());

    // If the bitmap is out of bounds (i.e. clamp mode where we only see the
    // stretched sides), canvas will clip this out and the extraneous data
    // won't be saved to the PDF.
    canvas.drawImage(image, 0, 0);

    SkScalar width = SkIntToScalar(image->width());
    SkScalar height = SkIntToScalar(image->height());

    SkPaint paint;
    paint.setColor(key.fPaintColor);
    // Tiling is implied.  First we handle mirroring.
    if (tileModes[0] == SkShader::kMirror_TileMode) {
        SkMatrix xMirror;
        xMirror.setScale(-1, 1);
        xMirror.postTranslate(2 * width, 0);
        draw_image_matrix(&canvas, image, xMirror, paint);
        patternBBox.fRight += width;
    }
    if (tileModes[1] == SkShader::kMirror_TileMode) {
        SkMatrix yMirror;
        yMirror.setScale(SK_Scalar1, -SK_Scalar1);
        yMirror.postTranslate(0, 2 * height);
        draw_image_matrix(&canvas, image, yMirror, paint);
        patternBBox.fBottom += height;
    }
    if (tileModes[0] == SkShader::kMirror_TileMode &&
            tileModes[1] == SkShader::kMirror_TileMode) {
        SkMatrix mirror;
        mirror.setScale(-1, -1);
        mirror.postTranslate(2 * width, 2 * height);
        draw_image_matrix(&canvas, image, mirror, paint);
    }

    // Then handle Clamping, which requires expanding the pattern canvas to
    // cover the entire surfaceBBox.

    SkBitmap bitmap;
    if (tileModes[0] == SkShader::kClamp_TileMode ||
        tileModes[1] == SkShader::kClamp_TileMode) {
        // For now, the easiest way to access the colors in the corners and sides is
        // to just make a bitmap from the image.
        if (!SkPDFUtils::ToBitmap(image, &bitmap)) {
            bitmap.allocN32Pixels(image->width(), image->height());
            bitmap.eraseColor(0x00000000);
        }
    }

    // If both x and y are in clamp mode, we start by filling in the corners.
    // (Which are just a rectangles of the corner colors.)
    if (tileModes[0] == SkShader::kClamp_TileMode &&
            tileModes[1] == SkShader::kClamp_TileMode) {
        SkASSERT(!bitmap.drawsNothing());
        SkPaint paint;
        SkRect rect;
        rect = SkRect::MakeLTRB(deviceBounds.left(), deviceBounds.top(), 0, 0);
        if (!rect.isEmpty()) {
            paint.setColor(bitmap.getColor(0, 0));
            canvas.drawRect(rect, paint);
        }

        rect = SkRect::MakeLTRB(width, deviceBounds.top(),
                                deviceBounds.right(), 0);
        if (!rect.isEmpty()) {
            paint.setColor(bitmap.getColor(bitmap.width() - 1, 0));
            canvas.drawRect(rect, paint);
        }

        rect = SkRect::MakeLTRB(width, height,
                                deviceBounds.right(), deviceBounds.bottom());
        if (!rect.isEmpty()) {
            paint.setColor(bitmap.getColor(bitmap.width() - 1,
                                           bitmap.height() - 1));
            canvas.drawRect(rect, paint);
        }

        rect = SkRect::MakeLTRB(deviceBounds.left(), height,
                                0, deviceBounds.bottom());
        if (!rect.isEmpty()) {
            paint.setColor(bitmap.getColor(0, bitmap.height() - 1));
            canvas.drawRect(rect, paint);
        }
    }

    // Then expand the left, right, top, then bottom.
    if (tileModes[0] == SkShader::kClamp_TileMode) {
        SkASSERT(!bitmap.drawsNothing());
        SkIRect subset = SkIRect::MakeXYWH(0, 0, 1, bitmap.height());
        if (deviceBounds.left() < 0) {
            SkBitmap left;
            SkAssertResult(bitmap.extractSubset(&left, subset));

            SkMatrix leftMatrix;
            leftMatrix.setScale(-deviceBounds.left(), 1);
            leftMatrix.postTranslate(deviceBounds.left(), 0);
            draw_bitmap_matrix(&canvas, left, leftMatrix, paint);

            if (tileModes[1] == SkShader::kMirror_TileMode) {
                leftMatrix.postScale(SK_Scalar1, -SK_Scalar1);
                leftMatrix.postTranslate(0, 2 * height);
                draw_bitmap_matrix(&canvas, left, leftMatrix, paint);
            }
            patternBBox.fLeft = 0;
        }

        if (deviceBounds.right() > width) {
            SkBitmap right;
            subset.offset(bitmap.width() - 1, 0);
            SkAssertResult(bitmap.extractSubset(&right, subset));

            SkMatrix rightMatrix;
            rightMatrix.setScale(deviceBounds.right() - width, 1);
            rightMatrix.postTranslate(width, 0);
            draw_bitmap_matrix(&canvas, right, rightMatrix, paint);

            if (tileModes[1] == SkShader::kMirror_TileMode) {
                rightMatrix.postScale(SK_Scalar1, -SK_Scalar1);
                rightMatrix.postTranslate(0, 2 * height);
                draw_bitmap_matrix(&canvas, right, rightMatrix, paint);
            }
            patternBBox.fRight = deviceBounds.width();
        }
    }

    if (tileModes[1] == SkShader::kClamp_TileMode) {
        SkASSERT(!bitmap.drawsNothing());
        SkIRect subset = SkIRect::MakeXYWH(0, 0, bitmap.width(), 1);
        if (deviceBounds.top() < 0) {
            SkBitmap top;
            SkAssertResult(bitmap.extractSubset(&top, subset));

            SkMatrix topMatrix;
            topMatrix.setScale(SK_Scalar1, -deviceBounds.top());
            topMatrix.postTranslate(0, deviceBounds.top());
            draw_bitmap_matrix(&canvas, top, topMatrix, paint);

            if (tileModes[0] == SkShader::kMirror_TileMode) {
                topMatrix.postScale(-1, 1);
                topMatrix.postTranslate(2 * width, 0);
                draw_bitmap_matrix(&canvas, top, topMatrix, paint);
            }
            patternBBox.fTop = 0;
        }

        if (deviceBounds.bottom() > height) {
            SkBitmap bottom;
            subset.offset(0, bitmap.height() - 1);
            SkAssertResult(bitmap.extractSubset(&bottom, subset));

            SkMatrix bottomMatrix;
            bottomMatrix.setScale(SK_Scalar1, deviceBounds.bottom() - height);
            bottomMatrix.postTranslate(0, height);
            draw_bitmap_matrix(&canvas, bottom, bottomMatrix, paint);

            if (tileModes[0] == SkShader::kMirror_TileMode) {
                bottomMatrix.postScale(-1, 1);
                bottomMatrix.postTranslate(2 * width, 0);
                draw_bitmap_matrix(&canvas, bottom, bottomMatrix, paint);
            }
            patternBBox.fBottom = deviceBounds.height();
        }
    }

    auto imageShader = patternDevice->content();
    std::unique_ptr<SkPDFDict> resourceDict = patternDevice->makeResourceDict();
    std::unique_ptr<SkPDFDict> dict = SkPDFMakeDict();
    SkPDFUtils::PopulateTilingPatternDict(dict.get(), patternBBox,
                                          std::move(resourceDict), finalMatrix);
    return SkPDFStreamOut(std::move(dict), std::move(imageShader), doc);
}

// Generic fallback for unsupported shaders:
//  * allocate a surfaceBBox-sized bitmap
//  * shade the whole area
//  * use the result as a bitmap shader
static SkPDFIndirectReference make_fallback_shader(SkPDFDocument* doc,
                                                   SkShader* shader,
                                                   const SkMatrix& canvasTransform,
                                                   const SkIRect& surfaceBBox,
                                                   SkColor paintColor) {
    // TODO(vandebo) This drops SKComposeShader on the floor.  We could
    // handle compose shader by pulling things up to a layer, drawing with
    // the first shader, applying the xfer mode and drawing again with the
    // second shader, then applying the layer to the original drawing.
    SkPDFImageShaderKey key = {
        canvasTransform,
        SkMatrix::I(),
        surfaceBBox,
        {{0, 0, 0, 0}, 0},  // don't need the key; won't de-dup.
        {SkShader::kClamp_TileMode, SkShader::kClamp_TileMode},
        paintColor};

    key.fShaderTransform = shader->getLocalMatrix();

    // surfaceBBox is in device space. While that's exactly what we
    // want for sizing our bitmap, we need to map it into
    // shader space for adjustments (to match
    // MakeImageShader's behavior).
    SkRect shaderRect = SkRect::Make(surfaceBBox);
    if (!SkPDFUtils::InverseTransformBBox(canvasTransform, &shaderRect)) {
        return SkPDFIndirectReference();
    }
    // Clamp the bitmap size to about 1M pixels
    static const SkScalar kMaxBitmapArea = 1024 * 1024;
    SkScalar bitmapArea = surfaceBBox.width() * surfaceBBox.height();
    SkScalar rasterScale = 1.0f;
    if (bitmapArea > kMaxBitmapArea) {
        rasterScale *= SkScalarSqrt(kMaxBitmapArea / bitmapArea);
    }

    SkISize size = {SkScalarRoundToInt(rasterScale * surfaceBBox.width()),
                    SkScalarRoundToInt(rasterScale * surfaceBBox.height())};
    SkSize scale = {SkIntToScalar(size.width()) / shaderRect.width(),
                    SkIntToScalar(size.height()) / shaderRect.height()};

    auto surface = SkSurface::MakeRasterN32Premul(size.width(), size.height());
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorTRANSPARENT);

    SkPaint p;
    p.setShader(sk_ref_sp(shader));
    p.setColor(paintColor);

    canvas->scale(scale.width(), scale.height());
    canvas->translate(-shaderRect.x(), -shaderRect.y());
    canvas->drawPaint(p);

    key.fShaderTransform.setTranslate(shaderRect.x(), shaderRect.y());
    key.fShaderTransform.preScale(1 / scale.width(), 1 / scale.height());

    sk_sp<SkImage> image = surface->makeImageSnapshot();
    return make_image_shader(doc, key, image.get());
}

static SkColor adjust_color(SkShader* shader, SkColor paintColor) {
    if (SkImage* img = shader->isAImage(nullptr, nullptr)) {
        if (img->isAlphaOnly()) {
            return paintColor;
        }
    }
    // only preserve the alpha.
    return paintColor & SK_ColorBLACK;
}

SkPDFIndirectReference SkPDFMakeShader(SkPDFDocument* doc,
                                       SkShader* shader,
                                       const SkMatrix& canvasTransform,
                                       const SkIRect& surfaceBBox,
                                       SkColor paintColor) {
    SkASSERT(shader);
    SkASSERT(doc);
    if (SkShader::kNone_GradientType != shader->asAGradient(nullptr)) {
        return SkPDFGradientShader::Make(doc, shader, canvasTransform, surfaceBBox);
    }
    if (surfaceBBox.isEmpty()) {
        return SkPDFIndirectReference();
    }
    SkBitmap image;
    SkPDFImageShaderKey key = {
        canvasTransform,
        SkMatrix::I(),
        surfaceBBox,
        {{0, 0, 0, 0}, 0},
        {SkShader::kClamp_TileMode, SkShader::kClamp_TileMode},
        adjust_color(shader, paintColor)};

    SkASSERT(shader->asAGradient(nullptr) == SkShader::kNone_GradientType) ;
    if (SkImage* skimg = shader->isAImage(&key.fShaderTransform, key.fImageTileModes)) {
        key.fBitmapKey = SkBitmapKeyFromImage(skimg);
        SkPDFIndirectReference* shaderPtr = doc->fImageShaderMap.find(key);
        if (shaderPtr) {
            return *shaderPtr;
        }
        SkPDFIndirectReference pdfShader = make_image_shader(doc, key, skimg);
        doc->fImageShaderMap.set(std::move(key), pdfShader);
        return pdfShader;
    }
    // Don't bother to de-dup fallback shader.
    return make_fallback_shader(doc, shader, canvasTransform, surfaceBBox, key.fPaintColor);
}
