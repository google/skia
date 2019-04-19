/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "AnimTimer.h"
#include "Sample.h"
#include "SkBitmapProcShader.h"
#include "SkCanvas.h"
#include "SkDrawable.h"
#include "SkLightingShader.h"
#include "SkLights.h"
#include "SkNormalSource.h"
#include "SkRSXform.h"
#include "SkRandom.h"

#include "ToolUtils.h"

// A crude normal mapped asteroids-like sample
class DrawLitAtlasDrawable : public SkDrawable {
public:
    DrawLitAtlasDrawable(const SkRect& r)
            : fBounds(r)
            , fUseColors(false)
            , fLightDir(SkVector3::Make(1.0f, 0.0f, 0.0f)) {
        fAtlas = MakeAtlas();

        SkRandom rand;
        for (int i = 0; i < kNumAsteroids; ++i) {
            fAsteroids[i].initAsteroid(&rand, fBounds, &fDiffTex[i], &fNormTex[i]);
        }

        fShip.initShip(fBounds, &fDiffTex[kNumAsteroids], &fNormTex[kNumAsteroids]);

        this->updateLights();
    }

    void toggleUseColors() {
        fUseColors = !fUseColors;
    }

    void rotateLight() {
        SkScalar r = SK_ScalarPI / 6.0f,
                 s = SkScalarSin(r),
                 c = SkScalarCos(r);

        SkScalar newX = c * fLightDir.fX - s * fLightDir.fY;
        SkScalar newY = s * fLightDir.fX + c * fLightDir.fY;

        fLightDir.set(newX, newY, 0.0f);

        this->updateLights();
    }

    void left() {
        SkScalar newRot = SkScalarMod(fShip.rot() + (2*SK_ScalarPI - SK_ScalarPI/32.0f),
                                      2 * SK_ScalarPI);
        fShip.setRot(newRot);
    }

    void right() {
        SkScalar newRot = SkScalarMod(fShip.rot() + SK_ScalarPI/32.0f, 2 * SK_ScalarPI);
        fShip.setRot(newRot);
    }

    void thrust() {
        SkScalar s = SkScalarSin(fShip.rot()),
                 c = SkScalarCos(fShip.rot());

        SkVector newVel = fShip.velocity();
        newVel.fX += s;
        newVel.fY += -c;

        SkScalar len = newVel.length();
        if (len > kMaxShipSpeed) {
            newVel.setLength(SkIntToScalar(kMaxShipSpeed));
        }

        fShip.setVelocity(newVel);
    }

protected:
    void onDraw(SkCanvas* canvas) override {
        SkRSXform xforms[kNumAsteroids+kNumShips];
        SkColor colors[kNumAsteroids+kNumShips];

        for (int i = 0; i < kNumAsteroids; ++i) {
            fAsteroids[i].advance(fBounds);
            xforms[i] = fAsteroids[i].asRSXform();
            if (fUseColors) {
                colors[i] = SkColorSetARGB(0xFF, 0xFF, 0xFF, 0xFF);
            }
        }

        fShip.advance(fBounds);
        xforms[kNumAsteroids] = fShip.asRSXform();
        if (fUseColors) {
            colors[kNumAsteroids] = SkColorSetARGB(0xFF, 0xFF, 0xFF, 0xFF);
        }

#ifdef SK_DEBUG
        canvas->drawBitmap(fAtlas, 0, 0); // just to see the atlas

        this->drawLightDir(canvas, fBounds.centerX(), fBounds.centerY());
#endif

#if 0
        // TODO: revitalize when drawLitAtlas API lands
        SkPaint paint;
        paint.setFilterQuality(kLow_SkFilterQuality);

        const SkRect cull = this->getBounds();
        const SkColor* colorsPtr = fUseColors ? colors : NULL;

        canvas->drawLitAtlas(fAtlas, xforms, fDiffTex, fNormTex, colorsPtr, kNumAsteroids+1,
                             SkXfermode::kModulate_Mode, &cull, &paint, fLights);
#else
        SkMatrix diffMat, normalMat;

        for (int i = 0; i < kNumAsteroids+1; ++i) {
            colors[i] = colors[i] & 0xFF000000; // to silence compilers
            SkPaint paint;

            SkRect r = fDiffTex[i];
            r.offsetTo(0, 0);

            diffMat.setRectToRect(fDiffTex[i], r, SkMatrix::kFill_ScaleToFit);
            normalMat.setRectToRect(fNormTex[i], r, SkMatrix::kFill_ScaleToFit);

            SkMatrix m;
            m.setRSXform(xforms[i]);

            sk_sp<SkShader> normalMap = fAtlas.makeShader(&normalMat);
            sk_sp<SkNormalSource> normalSource = SkNormalSource::MakeFromNormalMap(
                    std::move(normalMap), m);
            sk_sp<SkShader> diffuseShader = fAtlas.makeShader(&diffMat);
            paint.setShader(SkLightingShader::Make(std::move(diffuseShader),
                    std::move(normalSource), fLights));

            canvas->save();
                canvas->setMatrix(m);
                canvas->drawRect(r, paint);
            canvas->restore();
        }
#endif

#ifdef SK_DEBUG
        {
            SkPaint paint;
            paint.setColor(SK_ColorRED);

            for (int i = 0; i < kNumAsteroids; ++i) {
                canvas->drawCircle(fAsteroids[i].pos().x(), fAsteroids[i].pos().y(), 2, paint);
            }
            canvas->drawCircle(fShip.pos().x(), fShip.pos().y(), 2, paint);

            paint.setStyle(SkPaint::kStroke_Style);
            canvas->drawRect(this->getBounds(), paint);
        }
#endif
    }

    SkRect onGetBounds() override {
        return fBounds;
    }

private:

    enum ObjType {
        kBigAsteroid_ObjType = 0,
        kMedAsteroid_ObjType,
        kSmAsteroid_ObjType,
        kShip_ObjType,

        kLast_ObjType = kShip_ObjType
    };

    static const int kObjTypeCount = kLast_ObjType + 1;

    void updateLights() {
        SkLights::Builder builder;

        builder.add(SkLights::Light::MakeDirectional(
                SkColor3f::Make(1.0f, 1.0f, 1.0f), fLightDir));
        builder.setAmbientLightColor(SkColor3f::Make(0.2f, 0.2f, 0.2f));

        fLights = builder.finish();
    }

#ifdef SK_DEBUG
    // Draw a vector to the light
    void drawLightDir(SkCanvas* canvas, SkScalar centerX, SkScalar centerY) {
        static const int kBgLen = 30;
        static const int kSmLen = 5;

        // TODO: change the lighting coordinate system to be right handed
        SkPoint p1 = SkPoint::Make(centerX + kBgLen * fLightDir.fX,
                                   centerY - kBgLen * fLightDir.fY);
        SkPoint p2 = SkPoint::Make(centerX + (kBgLen-kSmLen) * fLightDir.fX,
                                   centerY - (kBgLen-kSmLen) * fLightDir.fY);

        SkPaint p;
        canvas->drawLine(centerX, centerY, p1.fX, p1.fY, p);
        canvas->drawLine(p1.fX, p1.fY,
                         p2.fX - kSmLen * fLightDir.fY, p2.fY - kSmLen * fLightDir.fX, p);
        canvas->drawLine(p1.fX, p1.fY,
                         p2.fX + kSmLen * fLightDir.fY, p2.fY + kSmLen * fLightDir.fX, p);
    }
#endif

    // Create the mixed diffuse & normal atlas
    //
    //    big color circle  |  big normal hemi
    //    ------------------------------------
    //    med color circle  |  med normal pyra
    //    ------------------------------------
    //    sm color circle   |   sm normal hemi
    //    ------------------------------------
    //    big ship          | big tetra normal
    static SkBitmap MakeAtlas() {

        SkBitmap atlas;
        atlas.allocN32Pixels(kAtlasWidth, kAtlasHeight);

        for (int y = 0; y < kAtlasHeight; ++y) {
            int x = 0;
            for ( ; x < kBigSize+kPad; ++x) {
                *atlas.getAddr32(x, y) = SK_ColorTRANSPARENT;
            }
            for ( ; x < kAtlasWidth; ++x) {
                *atlas.getAddr32(x, y) = SkPackARGB32(0xFF, 0x88, 0x88, 0xFF);
            }
        }

        // big asteroid
        {
            SkPoint bigCenter = SkPoint::Make(kDiffXOff + kBigSize/2.0f, kBigYOff + kBigSize/2.0f);

            for (int y = kBigYOff; y < kBigYOff+kBigSize; ++y) {
                for (int x = kDiffXOff; x < kDiffXOff+kBigSize; ++x) {
                    SkScalar distSq = (x - bigCenter.fX) * (x - bigCenter.fX) +
                                      (y - bigCenter.fY) * (y - bigCenter.fY);
                    if (distSq > kBigSize*kBigSize/4.0f) {
                        *atlas.getAddr32(x, y) = SkPreMultiplyARGB(0, 0, 0, 0);
                    } else {
                        *atlas.getAddr32(x, y) = SkPackARGB32(0xFF, 0xFF, 0, 0);
                    }
                }
            }

            ToolUtils::create_hemi_normal_map(
                    &atlas, SkIRect::MakeXYWH(kNormXOff, kBigYOff, kBigSize, kBigSize));
        }

        // medium asteroid
        {
            for (int y = kMedYOff; y < kMedYOff+kMedSize; ++y) {
                for (int x = kDiffXOff; x < kDiffXOff+kMedSize; ++x) {
                    *atlas.getAddr32(x, y) = SkPackARGB32(0xFF, 0, 0xFF, 0);
                }
            }

            ToolUtils::create_frustum_normal_map(
                    &atlas, SkIRect::MakeXYWH(kNormXOff, kMedYOff, kMedSize, kMedSize));
        }

        // small asteroid
        {
            SkPoint smCenter = SkPoint::Make(kDiffXOff + kSmSize/2.0f, kSmYOff + kSmSize/2.0f);

            for (int y = kSmYOff; y < kSmYOff+kSmSize; ++y) {
                for (int x = kDiffXOff; x < kDiffXOff+kSmSize; ++x) {
                    SkScalar distSq = (x - smCenter.fX) * (x - smCenter.fX) +
                                      (y - smCenter.fY) * (y - smCenter.fY);
                    if (distSq > kSmSize*kSmSize/4.0f) {
                        *atlas.getAddr32(x, y) = SkPreMultiplyARGB(0, 0, 0, 0);
                    } else {
                        *atlas.getAddr32(x, y) = SkPackARGB32(0xFF, 0, 0, 0xFF);
                    }
                }
            }

            ToolUtils::create_hemi_normal_map(
                    &atlas, SkIRect::MakeXYWH(kNormXOff, kSmYOff, kSmSize, kSmSize));
        }

        // ship
        {
            SkScalar shipMidLine = kDiffXOff + kMedSize/2.0f;

            for (int y = kShipYOff; y < kShipYOff+kMedSize; ++y) {
                SkScalar scaledY = (y - kShipYOff)/(float)kMedSize; // 0..1

                for (int x = kDiffXOff; x < kDiffXOff+kMedSize; ++x) {
                    SkScalar scaledX;

                    if (x < shipMidLine) {
                        scaledX = 1.0f - (x - kDiffXOff)/(kMedSize/2.0f); // 0..1
                    } else {
                        scaledX = (x - shipMidLine)/(kMedSize/2.0f);      // 0..1
                    }

                    if (scaledX < scaledY) {
                        *atlas.getAddr32(x, y) = SkPackARGB32(0xFF, 0, 0xFF, 0xFF);
                    } else {
                        *atlas.getAddr32(x, y) = SkPackARGB32(0, 0, 0, 0);
                    }
                }
            }

            ToolUtils::create_tetra_normal_map(
                    &atlas, SkIRect::MakeXYWH(kNormXOff, kShipYOff, kMedSize, kMedSize));
        }

        return atlas;
    }

    class ObjectRecord {
    public:
        void initAsteroid(SkRandom *rand, const SkRect& bounds,
                          SkRect* diffTex, SkRect* normTex) {
            static const SkScalar gMaxSpeeds[3] = { 1, 2, 5 }; // smaller asteroids can go faster
            static const SkScalar gYOffs[3] = { kBigYOff, kMedYOff, kSmYOff };
            static const SkScalar gSizes[3] = { kBigSize, kMedSize, kSmSize };

            static unsigned int asteroidType = 0;
            fObjType = static_cast<ObjType>(asteroidType++ % 3);

            fPosition.set(bounds.fLeft + rand->nextUScalar1() * bounds.width(),
                          bounds.fTop + rand->nextUScalar1() * bounds.height());
            fVelocity.fX = rand->nextSScalar1();
            fVelocity.fY = sqrt(1.0f - fVelocity.fX * fVelocity.fX);
            SkASSERT(SkScalarNearlyEqual(fVelocity.length(), 1.0f));
            fVelocity *= gMaxSpeeds[fObjType];
            fRot = 0;
            fDeltaRot = rand->nextSScalar1() / 32;

            diffTex->setXYWH(SkIntToScalar(kDiffXOff), gYOffs[fObjType],
                             gSizes[fObjType], gSizes[fObjType]);
            normTex->setXYWH(SkIntToScalar(kNormXOff), gYOffs[fObjType],
                             gSizes[fObjType], gSizes[fObjType]);
        }

        void initShip(const SkRect& bounds, SkRect* diffTex, SkRect* normTex) {
            fObjType = kShip_ObjType;
            fPosition.set(bounds.centerX(), bounds.centerY());
            fVelocity = SkVector::Make(0.0f, 0.0f);
            fRot = 0.0f;
            fDeltaRot = 0.0f;

            diffTex->setXYWH(SkIntToScalar(kDiffXOff), SkIntToScalar(kShipYOff),
                             SkIntToScalar(kMedSize), SkIntToScalar(kMedSize));
            normTex->setXYWH(SkIntToScalar(kNormXOff), SkIntToScalar(kShipYOff),
                             SkIntToScalar(kMedSize), SkIntToScalar(kMedSize));
        }

        void advance(const SkRect& bounds) {
            fPosition += fVelocity;
            if (fPosition.fX > bounds.right()) {
                SkASSERT(fVelocity.fX > 0);
                fVelocity.fX = -fVelocity.fX;
            } else if (fPosition.fX < bounds.left()) {
                SkASSERT(fVelocity.fX < 0);
                fVelocity.fX = -fVelocity.fX;
            }
            if (fPosition.fY > bounds.bottom()) {
                if (fVelocity.fY > 0) {
                    fVelocity.fY = -fVelocity.fY;
                }
            } else if (fPosition.fY < bounds.top()) {
                if (fVelocity.fY < 0) {
                    fVelocity.fY = -fVelocity.fY;
                }
            }

            fRot += fDeltaRot;
            fRot = SkScalarMod(fRot, 2 * SK_ScalarPI);
        }

        const SkPoint& pos() const { return fPosition; }

        SkScalar rot() const { return fRot; }
        void setRot(SkScalar rot) { fRot = rot; }

        const SkPoint& velocity() const { return fVelocity; }
        void setVelocity(const SkPoint& velocity) { fVelocity = velocity; }

        SkRSXform asRSXform() const {
            static const SkScalar gHalfSizes[kObjTypeCount] = {
                SkScalarHalf(kBigSize),
                SkScalarHalf(kMedSize),
                SkScalarHalf(kSmSize),
                SkScalarHalf(kMedSize),
            };

            return SkRSXform::MakeFromRadians(1.0f, fRot, fPosition.x(), fPosition.y(),
                                              gHalfSizes[fObjType],
                                              gHalfSizes[fObjType]);
        }

    private:
        ObjType     fObjType;
        SkPoint     fPosition;
        SkVector    fVelocity;
        SkScalar    fRot;        // In radians.
        SkScalar    fDeltaRot;   // In radiands. Not used by ship.
    };

private:
    static const int kNumLights = 2;
    static const int kNumAsteroids = 6;
    static const int kNumShips = 1;

    static const int kBigSize = 128;
    static const int kMedSize = 64;
    static const int kSmSize = 32;
    static const int kPad = 1;
    static const int kAtlasWidth = kBigSize + kBigSize + 2 * kPad; // 2 pads in the middle
    static const int kAtlasHeight = kBigSize + kMedSize + kSmSize + kMedSize + 3 * kPad;

    static const int kDiffXOff = 0;
    static const int kNormXOff = kBigSize + 2 * kPad;

    static const int kBigYOff = 0;
    static const int kMedYOff = kBigSize + kPad;
    static const int kSmYOff = kMedYOff + kMedSize + kPad;
    static const int kShipYOff = kSmYOff + kSmSize + kPad;
    static const int kMaxShipSpeed = 5;

    SkBitmap        fAtlas;
    ObjectRecord    fAsteroids[kNumAsteroids];
    ObjectRecord    fShip;
    SkRect          fDiffTex[kNumAsteroids+kNumShips];
    SkRect          fNormTex[kNumAsteroids+kNumShips];
    SkRect          fBounds;
    bool            fUseColors;
    SkVector3       fLightDir;
    sk_sp<SkLights> fLights;

    typedef SkDrawable INHERITED;
};

class DrawLitAtlasView : public Sample {
public:
    DrawLitAtlasView() : fDrawable(new DrawLitAtlasDrawable(SkRect::MakeWH(640, 480))) {}

protected:
    bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "DrawLitAtlas");
            return true;
        }
        SkUnichar uni;
        if (Sample::CharQ(*evt, &uni)) {
            switch (uni) {
                case 'C':
                    fDrawable->toggleUseColors();
                    return true;
                case 'j':
                    fDrawable->left();
                    return true;
                case 'k':
                    fDrawable->thrust();
                    return true;
                case 'l':
                    fDrawable->right();
                    return true;
                case 'o':
                    fDrawable->rotateLight();
                    return true;
                default:
                    break;
            }
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawDrawable(fDrawable.get());
    }

    bool onAnimate(const AnimTimer& timer) override { return true; }

private:
    sk_sp<DrawLitAtlasDrawable> fDrawable;

    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new DrawLitAtlasView(); )
