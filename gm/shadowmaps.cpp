/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "gm.h"
#include "SkPaintFilterCanvas.h"
#include "SkPathEffect.h"
#include "SkPictureRecorder.h"
#include "SkShadowShader.h"
#include "SkSurface.h"

#ifdef SK_EXPERIMENTAL_SHADOWING

static sk_sp<SkShader> make_shadow_shader(sk_sp<SkImage> povDepth,
                                          sk_sp<SkImage> diffuse,
                                          sk_sp<SkLights> lights) {

    sk_sp<SkShader> povDepthShader = povDepth->makeShader(SkShader::kClamp_TileMode,
                                                          SkShader::kClamp_TileMode);

    sk_sp<SkShader> diffuseShader = diffuse->makeShader(SkShader::kClamp_TileMode,
                                                        SkShader::kClamp_TileMode);

    return SkShadowShader::Make(std::move(povDepthShader),
                                std::move(diffuseShader),
                                std::move(lights),
                                diffuse->width(), diffuse->height());
}

static sk_sp<SkPicture> make_test_picture(int width, int height) {
    SkPictureRecorder recorder;

    // LONG RANGE TODO: eventually add SkBBHFactory (bounding box factory)
    SkCanvas* canvas = recorder.beginRecording(SkRect::MakeIWH(width, height));

    SkASSERT(canvas->getTotalMatrix().isIdentity());
    SkPaint paint;
    paint.setColor(SK_ColorGRAY);

    // LONG RANGE TODO: tag occluders
    // LONG RANGE TODO: track number of IDs we need (hopefully less than 256)
    //                  and determinate the mapping from z to id

    // universal receiver, "ground"
    canvas->drawRect(SkRect::MakeIWH(width, height), paint);

    // TODO: Maybe add the ID here along with the depth

    paint.setColor(0xFFEE8888);

    canvas->translateZ(80);
    canvas->drawRect(SkRect::MakeLTRB(200,150,350,300), paint);

    paint.setColor(0xFF88EE88);

    canvas->translateZ(80);
    canvas->drawRect(SkRect::MakeLTRB(150,200,300,350), paint);

    paint.setColor(0xFF8888EE);

    canvas->translateZ(80);
    canvas->drawRect(SkRect::MakeLTRB(100,100,250,250), paint);
    // TODO: Add an assert that Z order matches painter's order
    // TODO: think about if the Z-order always matching painting order is too strict

    return recorder.finishRecordingAsPicture();
}

namespace skiagm {

/*  We override the onFilter method to draw depths into the canvas
 *  depending on the current draw depth of the canvas, throwing out
 *  the actual draw color.
 */
class SkShadowPaintFilterCanvas : public SkPaintFilterCanvas {
public:

    SkShadowPaintFilterCanvas(SkCanvas* canvas) : INHERITED(canvas) { }

    // TODO use a shader instead
    bool onFilter(SkTCopyOnFirstWrite<SkPaint>* paint, Type type) const override {
        if (*paint) {
            int z = this->getZ();
            SkASSERT(z <= 0xFF && z >= 0x00);

            SkPaint newPaint;
            newPaint.setPathEffect(sk_ref_sp<SkPathEffect>((*paint)->getPathEffect()));

            SkColor color = 0xFF000000; // init color to opaque black
            color |= z; // Put the index into the blue component
            newPaint.setColor(color);

            *paint->writable() = newPaint;
        }

        return true;
    }

    void onDrawPicture(const SkPicture* picture, const SkMatrix* matrix, const SkPaint* paint) {
        SkTCopyOnFirstWrite<SkPaint> filteredPaint(paint);
        if (this->onFilter(&filteredPaint, kPicture_Type)) {
            // we directly call SkCanvas's onDrawPicture because calling the one
            // that INHERITED has (SkPaintFilterCanvas) leads to wrong behavior
            this->SkCanvas::onDrawPicture(picture, matrix, filteredPaint);
        }
    }

    void updateMatrix() {
        this->save();

        // When we use the SkShadowPaintFilterCanvas, we can only render
        // one depth map at a time. Thus, we leave it up to the user to
        // set SkLights to only contain (or contain at the first position)
        // the light they intend to use for the current depth rendering.

        if (fLights->numLights() > 0 &&
            this->fLights->light(0).type() == SkLights::Light::kDirectional_LightType) {
            SkVector3 lightDir = this->fLights->light(0).dir();
            SkScalar x = lightDir.fX * this->getZ();
            SkScalar y = lightDir.fY * this->getZ();

            this->translate(x, y);
        }

    }

    void onDrawPaint(const SkPaint& paint) override {
        this->updateMatrix();
        this->INHERITED::onDrawPaint(paint);
        this->restore();
    }

    void onDrawPoints(PointMode mode, size_t count, const SkPoint pts[],
                      const SkPaint& paint) override {
        this->updateMatrix();
        this->INHERITED::onDrawPoints(mode, count, pts, paint);
        this->restore();
    }

    void onDrawRect(const SkRect& rect, const SkPaint& paint) override {
        this->updateMatrix();
        this->INHERITED::onDrawRect(rect, paint);
        this->restore();
    }

    void onDrawRRect(const SkRRect& rrect, const SkPaint& paint) override {
        this->updateMatrix();
        this->INHERITED::onDrawRRect(rrect, paint);
        this->restore();
    }

    void onDrawDRRect(const SkRRect& outer, const SkRRect& inner,
                      const SkPaint& paint) override {
        this->updateMatrix();
        this->INHERITED::onDrawDRRect(outer, inner, paint);
        this->restore();
    }

    void onDrawOval(const SkRect& rect, const SkPaint& paint) override {
        this->updateMatrix();
        this->INHERITED::onDrawOval(rect, paint);
        this->restore();
    }

    void onDrawPath(const SkPath& path, const SkPaint& paint) override {
        this->updateMatrix();
        this->INHERITED::onDrawPath(path, paint);
        this->restore();
    }

    void onDrawBitmap(const SkBitmap& bm, SkScalar left, SkScalar top,
                      const SkPaint* paint) override {
        this->updateMatrix();
        this->INHERITED::onDrawBitmap(bm, left, top, paint);
        this->restore();
    }

    void onDrawBitmapRect(const SkBitmap& bm, const SkRect* src, const SkRect& dst,
                          const SkPaint* paint, SrcRectConstraint constraint) override {
        this->updateMatrix();
        this->INHERITED::onDrawBitmapRect(bm, src, dst, paint, constraint);
        this->restore();
    }

    void onDrawBitmapNine(const SkBitmap& bm, const SkIRect& center,
                          const SkRect& dst, const SkPaint* paint) {
        this->updateMatrix();
        this->INHERITED::onDrawBitmapNine(bm, center, dst, paint);
        this->restore();
    }

    void onDrawImage(const SkImage* image, SkScalar left, SkScalar top,
                     const SkPaint* paint) override {
        this->updateMatrix();
        this->INHERITED::onDrawImage(image, left, top, paint);
        this->restore();
    }

    void onDrawImageRect(const SkImage* image, const SkRect* src, const SkRect& dst,
                         const SkPaint* paint, SrcRectConstraint constraint) override {
        this->updateMatrix();
        this->INHERITED::onDrawImageRect(image, src, dst, paint, constraint);
        this->restore();
    }

    void onDrawImageNine(const SkImage* image, const SkIRect& center,
                         const SkRect& dst, const SkPaint* paint) {
        this->updateMatrix();
        this->INHERITED::onDrawImageNine(image, center, dst, paint);
        this->restore();
    }

    void onDrawVertices(VertexMode vmode, int vertexCount, const SkPoint vertices[],
                        const SkPoint texs[], const SkColor colors[], SkXfermode* xmode,
                        const uint16_t indices[], int indexCount, const SkPaint& paint) override {
        this->updateMatrix();
        this->INHERITED::onDrawVertices(vmode, vertexCount, vertices, texs, colors,
                                        xmode, indices, indexCount, paint);
        this->restore();
    }

    void onDrawPatch(const SkPoint cubics[], const SkColor colors[], const SkPoint texCoords[],
                     SkXfermode* xmode, const SkPaint& paint) override {
        this->updateMatrix();
        this->INHERITED::onDrawPatch(cubics, colors, texCoords, xmode, paint);
        this->restore();
    }

    void onDrawText(const void* text, size_t byteLength,
                    SkScalar x, SkScalar y, const SkPaint& paint) override {
        this->updateMatrix();
        this->INHERITED::onDrawText(text, byteLength, x, y, paint);
        this->restore();
    }

    void onDrawPosText(const void* text, size_t byteLength,
                       const SkPoint pos[], const SkPaint& paint) override {
        this->updateMatrix();
        this->INHERITED::onDrawPosText(text, byteLength, pos, paint);
        this->restore();
    }

    void onDrawPosTextH(const void* text, size_t byteLength, const SkScalar xpos[],
                        SkScalar constY, const SkPaint& paint) override {
        this->updateMatrix();
        this->INHERITED::onDrawPosTextH(text, byteLength, xpos, constY, paint);
        this->restore();
    }

    void onDrawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
                          const SkMatrix* matrix, const SkPaint& paint) override {
        this->updateMatrix();
        this->INHERITED::onDrawTextOnPath(text, byteLength, path, matrix, paint);
        this->restore();
    }

    void onDrawTextRSXform(const void* text, size_t byteLength, const SkRSXform xform[],
                           const SkRect* cull, const SkPaint& paint) override {
        this->updateMatrix();
        this->INHERITED::onDrawTextRSXform(text, byteLength, xform, cull, paint);
        this->restore();
    }

    void onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                        const SkPaint& paint) override {
        this->updateMatrix();
        this->INHERITED::onDrawTextBlob(blob, x, y, paint);
        this->restore();
    }

private:
    typedef SkPaintFilterCanvas INHERITED;
};

class ShadowMapsGM : public GM {
public:
    ShadowMapsGM() {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFCCCCCC));
    }

    void onOnceBeforeDraw() override {
        // Create a light set consisting of
        //   - bluish directional light pointing more right than down
        //   - reddish directional light pointing more down than right
        //   - soft white ambient light

        SkLights::Builder builder;
        builder.add(SkLights::Light(SkColor3f::Make(0.2f, 0.3f, 0.4f),
                                    SkVector3::Make(0.2f, 0.1f, 1.0f)));
        builder.add(SkLights::Light(SkColor3f::Make(0.4f, 0.3f, 0.2f),
                                    SkVector3::Make(0.1f, 0.2f, 1.0f)));
        builder.add(SkLights::Light(SkColor3f::Make(0.4f, 0.4f, 0.4f)));
        fLights = builder.finish();
    }

protected:
    static const int kWidth = 400;
    static const int kHeight = 400;

    SkString onShortName() override {
        return SkString("shadowmaps");
    }

    SkISize onISize() override {
        return SkISize::Make(kWidth, kHeight);
    }

    void onDraw(SkCanvas* canvas) override {
        // This picture stores the picture of the scene.
        // It's used to generate the depth maps.
        sk_sp<SkPicture> pic(make_test_picture(kWidth, kHeight));

        for (int i = 0; i < fLights->numLights(); ++i) {
            // skip over ambient lights; they don't cast shadows
            if (SkLights::Light::kAmbient_LightType == fLights->light(i).type()) {
                continue;
            }
            // TODO: compute the correct size of the depth map from the light properties
            // TODO: maybe add a kDepth_8_SkColorType

            SkImageInfo info = SkImageInfo::Make(kWidth, kHeight,
                                                 kBGRA_8888_SkColorType,
                                                 kOpaque_SkAlphaType);

            // Create a new surface (that matches the backend of canvas)
            // for each shadow map
            sk_sp<SkSurface> surf(canvas->makeSurface(info));

            // Wrap another SPFCanvas around the surface
            sk_sp<SkShadowPaintFilterCanvas> depthMapCanvas =
                    sk_make_sp<SkShadowPaintFilterCanvas>(surf->getCanvas());

            // set the depth map canvas to have the light we're drawing.
            SkLights::Builder builder;
            builder.add(fLights->light(i));
            sk_sp<SkLights> curLight = builder.finish();

            depthMapCanvas->setLights(std::move(curLight));
            depthMapCanvas->drawPicture(pic);

            fLights->light(i).setShadowMap(surf->makeImageSnapshot());
        }

        sk_sp<SkImage> povDepthMap;
        sk_sp<SkImage> diffuseMap;

        // TODO: pass the depth to the shader in vertices, or uniforms
        //       so we don't have to render depth and color separately

        // povDepthMap
        {
            SkLights::Builder builder;
            builder.add(SkLights::Light(SkColor3f::Make(1.0f, 1.0f, 1.0f),
                                        SkVector3::Make(0.0f, 0.0f, 1.0f)));
            sk_sp<SkLights> povLight = builder.finish();

            SkImageInfo info = SkImageInfo::Make(kWidth, kHeight,
                                                 kBGRA_8888_SkColorType,
                                                 kOpaque_SkAlphaType);

            // Create a new surface (that matches the backend of canvas)
            // to create the povDepthMap
            sk_sp<SkSurface> surf(canvas->makeSurface(info));

            // Wrap another SPFCanvas around the surface
            sk_sp<SkShadowPaintFilterCanvas> depthMapCanvas =
                    sk_make_sp<SkShadowPaintFilterCanvas>(surf->getCanvas());

            // set the depth map canvas to have the light as the user's POV
            depthMapCanvas->setLights(std::move(povLight));

            depthMapCanvas->drawPicture(pic);

            povDepthMap = surf->makeImageSnapshot();
        }

        // diffuseMap
        {
            SkImageInfo info = SkImageInfo::Make(kWidth, kHeight,
                                                 kBGRA_8888_SkColorType,
                                                 kOpaque_SkAlphaType);

            sk_sp<SkSurface> surf(canvas->makeSurface(info));
            surf->getCanvas()->drawPicture(pic);

            diffuseMap = surf->makeImageSnapshot();
        }

        SkPaint paint;
        paint.setShader(make_shadow_shader(std::move(povDepthMap), std::move(diffuseMap), fLights));

        canvas->drawRect(SkRect::MakeIWH(kWidth, kHeight), paint);
    }

private:
    sk_sp<SkLights> fLights;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ShadowMapsGM;)
}

#endif
