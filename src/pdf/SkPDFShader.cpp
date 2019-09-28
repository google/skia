/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/pdf/SkPDFShader.h"

#include "include/core/SkData.h"
#include "include/core/SkMath.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/docs/SkPDFDocument.h"
#include "include/private/SkTemplates.h"
#include "src/pdf/SkPDFDevice.h"
#include "src/pdf/SkPDFDocumentPriv.h"
#include "src/pdf/SkPDFFormXObject.h"
#include "src/pdf/SkPDFGradientShader.h"
#include "src/pdf/SkPDFGraphicState.h"
#include "src/pdf/SkPDFResourceDict.h"
#include "src/pdf/SkPDFUtils.h"

static void draw(SkCanvas* canvas, const SkImage* image, SkColor4f paintColor) {
    SkPaint paint(paintColor);
    canvas->drawImage(image, 0, 0, &paint);
}

static SkBitmap to_bitmap(const SkImage* image) {
    SkBitmap bitmap;
    if (!SkPDFUtils::ToBitmap(image, &bitmap)) {
        bitmap.allocN32Pixels(image->width(), image->height());
        bitmap.eraseColor(0x00000000);
    }
    return bitmap;
}

static void draw_matrix(SkCanvas* canvas, const SkImage* image,
                        const SkMatrix& matrix, SkColor4f paintColor) {
    SkAutoCanvasRestore acr(canvas, true);
    canvas->concat(matrix);
    draw(canvas, image, paintColor);
}

static void draw_bitmap_matrix(SkCanvas* canvas, const SkBitmap& bm,
                               const SkMatrix& matrix, SkColor4f paintColor) {
    SkAutoCanvasRestore acr(canvas, true);
    canvas->concat(matrix);
    SkPaint paint(paintColor);
    canvas->drawBitmap(bm, 0, 0, &paint);
}

static void fill_color_from_bitmap(SkCanvas* canvas,
                                   float left, float top, float right, float bottom,
                                   const SkBitmap& bitmap, int x, int y, float alpha) {
    SkRect rect{left, top, right, bottom};
    if (!rect.isEmpty()) {
        SkColor4f color = SkColor4f::FromColor(bitmap.getColor(x, y));
        SkPaint paint(SkColor4f{color.fR, color.fG, color.fB, alpha * color.fA});
        canvas->drawRect(rect, paint);
    }
}

static SkMatrix scale_translate(SkScalar sx, SkScalar sy, SkScalar tx, SkScalar ty) {
    SkMatrix m;
    m.setScaleTranslate(sx, sy, tx, ty);
    return m;
}

static bool is_tiled(SkTileMode m) { return SkTileMode::kMirror == m || SkTileMode::kRepeat == m; }

static SkPDFIndirectReference make_image_shader(SkPDFDocument* doc,
                                                SkMatrix finalMatrix,
                                                SkTileMode tileModesX,
                                                SkTileMode tileModesY,
                                                SkRect bBox,
                                                const SkImage* image,
                                                SkColor4f paintColor) {
    // The image shader pattern cell will be drawn into a separate device
    // in pattern cell space (no scaling on the bitmap, though there may be
    // translations so that all content is in the device, coordinates > 0).

    // Map clip bounds to shader space to ensure the device is large enough
    // to handle fake clamping.

    SkRect deviceBounds = bBox;
    if (!SkPDFUtils::InverseTransformBBox(finalMatrix, &deviceBounds)) {
        return SkPDFIndirectReference();
    }

    SkRect bitmapBounds = SkRect::MakeSize(SkSize::Make(image->dimensions()));

    // For tiling modes, the bounds should be extended to include the bitmap,
    // otherwise the bitmap gets clipped out and the shader is empty and awful.
    // For clamp modes, we're only interested in the clip region, whether
    // or not the main bitmap is in it.
    if (is_tiled(tileModesX) || is_tiled(tileModesY)) {
        deviceBounds.join(bitmapBounds);
    }

    SkISize patternDeviceSize = {SkScalarCeilToInt(deviceBounds.width()),
                                 SkScalarCeilToInt(deviceBounds.height())};
    auto patternDevice = sk_make_sp<SkPDFDevice>(patternDeviceSize, doc);
    SkCanvas canvas(patternDevice);

    SkRect patternBBox = SkRect::MakeSize(SkSize::Make(image->dimensions()));
    SkScalar width = patternBBox.width();
    SkScalar height = patternBBox.height();

    // Translate the canvas so that the bitmap origin is at (0, 0).
    canvas.translate(-deviceBounds.left(), -deviceBounds.top());
    patternBBox.offset(-deviceBounds.left(), -deviceBounds.top());
    // Undo the translation in the final matrix
    finalMatrix.preTranslate(deviceBounds.left(), deviceBounds.top());

    // If the bitmap is out of bounds (i.e. clamp mode where we only see the
    // stretched sides), canvas will clip this out and the extraneous data
    // won't be saved to the PDF.
    draw(&canvas, image, paintColor);

    // Tiling is implied.  First we handle mirroring.
    if (tileModesX == SkTileMode::kMirror) {
        draw_matrix(&canvas, image, scale_translate(-1, 1, 2 * width, 0), paintColor);
        patternBBox.fRight += width;
    }
    if (tileModesY == SkTileMode::kMirror) {
        draw_matrix(&canvas, image, scale_translate(1, -1, 0, 2 * height), paintColor);
        patternBBox.fBottom += height;
    }
    if (tileModesX == SkTileMode::kMirror && tileModesY == SkTileMode::kMirror) {
        draw_matrix(&canvas, image, scale_translate(-1, -1, 2 * width, 2 * height), paintColor);
    }

    // Then handle Clamping, which requires expanding the pattern canvas to
    // cover the entire surfaceBBox.

    SkBitmap bitmap;
    if (tileModesX == SkTileMode::kClamp || tileModesY == SkTileMode::kClamp) {
        // For now, the easiest way to access the colors in the corners and sides is
        // to just make a bitmap from the image.
        bitmap = to_bitmap(image);
    }

    // If both x and y are in clamp mode, we start by filling in the corners.
    // (Which are just a rectangles of the corner colors.)
    if (tileModesX == SkTileMode::kClamp && tileModesY == SkTileMode::kClamp) {
        SkASSERT(!bitmap.drawsNothing());

        fill_color_from_bitmap(&canvas, deviceBounds.left(), deviceBounds.top(), 0, 0,
                               bitmap, 0, 0, paintColor.fA);

        fill_color_from_bitmap(&canvas, width, deviceBounds.top(), deviceBounds.right(), 0,
                               bitmap, bitmap.width() - 1, 0, paintColor.fA);

        fill_color_from_bitmap(&canvas, width, height, deviceBounds.right(), deviceBounds.bottom(),
                               bitmap, bitmap.width() - 1, bitmap.height() - 1, paintColor.fA);

        fill_color_from_bitmap(&canvas, deviceBounds.left(), height, 0, deviceBounds.bottom(),
                               bitmap, 0, bitmap.height() - 1, paintColor.fA);
    }

    // Then expand the left, right, top, then bottom.
    if (tileModesX == SkTileMode::kClamp) {
        SkASSERT(!bitmap.drawsNothing());
        SkIRect subset = SkIRect::MakeXYWH(0, 0, 1, bitmap.height());
        if (deviceBounds.left() < 0) {
            SkBitmap left;
            SkAssertResult(bitmap.extractSubset(&left, subset));

            SkMatrix leftMatrix = scale_translate(-deviceBounds.left(), 1, deviceBounds.left(), 0);
            draw_bitmap_matrix(&canvas, left, leftMatrix, paintColor);

            if (tileModesY == SkTileMode::kMirror) {
                leftMatrix.postScale(SK_Scalar1, -SK_Scalar1);
                leftMatrix.postTranslate(0, 2 * height);
                draw_bitmap_matrix(&canvas, left, leftMatrix, paintColor);
            }
            patternBBox.fLeft = 0;
        }

        if (deviceBounds.right() > width) {
            SkBitmap right;
            subset.offset(bitmap.width() - 1, 0);
            SkAssertResult(bitmap.extractSubset(&right, subset));

            SkMatrix rightMatrix = scale_translate(deviceBounds.right() - width, 1, width, 0);
            draw_bitmap_matrix(&canvas, right, rightMatrix, paintColor);

            if (tileModesY == SkTileMode::kMirror) {
                rightMatrix.postScale(SK_Scalar1, -SK_Scalar1);
                rightMatrix.postTranslate(0, 2 * height);
                draw_bitmap_matrix(&canvas, right, rightMatrix, paintColor);
            }
            patternBBox.fRight = deviceBounds.width();
        }
    }
    if (tileModesX == SkTileMode::kDecal) {
        if (deviceBounds.left() < 0) {
            patternBBox.fLeft = 0;
        }
        if (deviceBounds.right() > width) {
            patternBBox.fRight = deviceBounds.width();
        }
    }

    if (tileModesY == SkTileMode::kClamp) {
        SkASSERT(!bitmap.drawsNothing());
        SkIRect subset = SkIRect::MakeXYWH(0, 0, bitmap.width(), 1);
        if (deviceBounds.top() < 0) {
            SkBitmap top;
            SkAssertResult(bitmap.extractSubset(&top, subset));

            SkMatrix topMatrix = scale_translate(1, -deviceBounds.top(), 0, deviceBounds.top());
            draw_bitmap_matrix(&canvas, top, topMatrix, paintColor);

            if (tileModesX == SkTileMode::kMirror) {
                topMatrix.postScale(-1, 1);
                topMatrix.postTranslate(2 * width, 0);
                draw_bitmap_matrix(&canvas, top, topMatrix, paintColor);
            }
            patternBBox.fTop = 0;
        }

        if (deviceBounds.bottom() > height) {
            SkBitmap bottom;
            subset.offset(0, bitmap.height() - 1);
            SkAssertResult(bitmap.extractSubset(&bottom, subset));

            SkMatrix bottomMatrix = scale_translate(1, deviceBounds.bottom() - height, 0, height);
            draw_bitmap_matrix(&canvas, bottom, bottomMatrix, paintColor);

            if (tileModesX == SkTileMode::kMirror) {
                bottomMatrix.postScale(-1, 1);
                bottomMatrix.postTranslate(2 * width, 0);
                draw_bitmap_matrix(&canvas, bottom, bottomMatrix, paintColor);
            }
            patternBBox.fBottom = deviceBounds.height();
        }
    }
    if (tileModesY == SkTileMode::kDecal) {
        if (deviceBounds.top() < 0) {
            patternBBox.fTop = 0;
        }
        if (deviceBounds.bottom() > height) {
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
                                                   SkColor4f paintColor) {
    // TODO(vandebo) This drops SKComposeShader on the floor.  We could
    // handle compose shader by pulling things up to a layer, drawing with
    // the first shader, applying the xfer mode and drawing again with the
    // second shader, then applying the layer to the original drawing.

    SkMatrix shaderTransform = as_SB(shader)->getLocalMatrix();

    // surfaceBBox is in device space. While that's exactly what we
    // want for sizing our bitmap, we need to map it into
    // shader space for adjustments (to match
    // MakeImageShader's behavior).
    SkRect shaderRect = SkRect::Make(surfaceBBox);
    if (!SkPDFUtils::InverseTransformBBox(canvasTransform, &shaderRect)) {
        return SkPDFIndirectReference();
    }
    // Clamp the bitmap size to about 1M pixels
    static const int kMaxBitmapArea = 1024 * 1024;
    SkScalar bitmapArea = (float)surfaceBBox.width() * (float)surfaceBBox.height();
    SkScalar rasterScale = 1.0f;
    if (bitmapArea > (float)kMaxBitmapArea) {
        rasterScale *= SkScalarSqrt((float)kMaxBitmapArea / bitmapArea);
    }

    SkISize size = {
        SkTClamp(SkScalarCeilToInt(rasterScale * surfaceBBox.width()),  1, kMaxBitmapArea),
        SkTClamp(SkScalarCeilToInt(rasterScale * surfaceBBox.height()), 1, kMaxBitmapArea)};
    SkSize scale = {SkIntToScalar(size.width()) / shaderRect.width(),
                    SkIntToScalar(size.height()) / shaderRect.height()};

    auto surface = SkSurface::MakeRasterN32Premul(size.width(), size.height());
    SkASSERT(surface);
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorTRANSPARENT);

    SkPaint p(paintColor);
    p.setShader(sk_ref_sp(shader));

    canvas->scale(scale.width(), scale.height());
    canvas->translate(-shaderRect.x(), -shaderRect.y());
    canvas->drawPaint(p);

    shaderTransform.setTranslate(shaderRect.x(), shaderRect.y());
    shaderTransform.preScale(1 / scale.width(), 1 / scale.height());

    sk_sp<SkImage> image = surface->makeImageSnapshot();
    SkASSERT(image);
    return make_image_shader(doc,
                             SkMatrix::Concat(canvasTransform, shaderTransform),
                             SkTileMode::kClamp, SkTileMode::kClamp,
                             SkRect::Make(surfaceBBox),
                             image.get(),
                             paintColor);
}

static SkColor4f adjust_color(SkShader* shader, SkColor4f paintColor) {
    if (SkImage* img = shader->isAImage(nullptr, (SkTileMode*)nullptr)) {
        if (img->isAlphaOnly()) {
            return paintColor;
        }
    }
    return SkColor4f{0, 0, 0, paintColor.fA};  // only preserve the alpha.
}

SkPDFIndirectReference SkPDFMakeShader(SkPDFDocument* doc,
                                       SkShader* shader,
                                       const SkMatrix& canvasTransform,
                                       const SkIRect& surfaceBBox,
                                       SkColor4f paintColor) {
    SkASSERT(shader);
    SkASSERT(doc);
    if (SkShader::kNone_GradientType != shader->asAGradient(nullptr)) {
        return SkPDFGradientShader::Make(doc, shader, canvasTransform, surfaceBBox);
    }
    if (surfaceBBox.isEmpty()) {
        return SkPDFIndirectReference();
    }
    SkBitmap image;

    SkASSERT(shader->asAGradient(nullptr) == SkShader::kNone_GradientType) ;

    paintColor = adjust_color(shader, paintColor);
    SkMatrix shaderTransform;
    SkTileMode imageTileModes[2];
    if (SkImage* skimg = shader->isAImage(&shaderTransform, imageTileModes)) {
        SkMatrix finalMatrix = SkMatrix::Concat(canvasTransform, shaderTransform);
        SkPDFImageShaderKey key = {
            finalMatrix,
            surfaceBBox,
            SkBitmapKeyFromImage(skimg),
            {imageTileModes[0], imageTileModes[1]},
            paintColor};
        SkPDFIndirectReference* shaderPtr = doc->fImageShaderMap.find(key);
        if (shaderPtr) {
            return *shaderPtr;
        }
        SkPDFIndirectReference pdfShader =
                make_image_shader(doc,
                                  finalMatrix,
                                  imageTileModes[0],
                                  imageTileModes[1],
                                  SkRect::Make(surfaceBBox),
                                  skimg,
                                  paintColor);
        doc->fImageShaderMap.set(std::move(key), pdfShader);
        return pdfShader;
    }
    // Don't bother to de-dup fallback shader.
    return make_fallback_shader(doc, shader, canvasTransform, surfaceBBox, paintColor);
}
