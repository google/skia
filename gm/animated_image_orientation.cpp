/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/android/SkAnimatedImage.h"
#include "include/codec/SkAndroidCodec.h"
#include "include/codec/SkCodec.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRRect.h"
#include "tools/Resources.h"

static sk_sp<SkPicture> post_processor(const SkRect& bounds) {
    int radius = (bounds.width() + bounds.height()) / 6;
    SkPathBuilder pathBuilder;
    pathBuilder.setFillType(SkPathFillType::kInverseEvenOdd)
               .addRRect(SkRRect::MakeRectXY(bounds, radius, radius));

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorTRANSPARENT);
    paint.setBlendMode(SkBlendMode::kSrc);

    SkPictureRecorder recorder;
    auto* canvas = recorder.beginRecording(bounds);
    canvas->drawPath(pathBuilder.detach(), paint);
    return recorder.finishRecordingAsPicture();
}

class AnimatedImageGM : public skiagm::GM {
    const char*   fPath;
    const char*   fName;
    const int     fStep;
    const SkIRect fCropRect;
    SkISize       fSize;
    int           fTranslate;
    sk_sp<SkData> fData;

    static const int kMaxFrames = 2;

    void init() {
        if (!fData) {
            fData = GetResourceAsData(fPath);
            auto codec = SkCodec::MakeFromData(fData);
            auto dimensions = codec->dimensions();

            fTranslate = std::max(dimensions.width(), dimensions.height()) // may be rotated
                         * 1.25f    // will be scaled up
                         + 2;       // padding

            auto origin = codec->getOrigin();
            bool respectOrigin = origin != kDefault_SkEncodedOrigin;
            fSize = { fTranslate * kMaxFrames * (respectOrigin ? 2 : 1)
                                 * 2    // crop and no-crop
                                 * 2,   // post-process and no post-process
                      fTranslate * 4    // 4 scales
                                 * 2 }; // newPictureSnapshot and getCurrentFrame
        }
    }
public:
    AnimatedImageGM(const char* path, const char* name, int step, SkIRect cropRect)
        : fPath(path)
        , fName(name)
        , fStep(step)
        , fCropRect(cropRect)
        , fSize{0, 0}
        , fTranslate(0)
    {}
    ~AnimatedImageGM() override = default;

    SkString onShortName() override {
        return SkStringPrintf("%s_animated_image", fName);
    }

    SkISize onISize() override {
        this->init();
        return fSize;
    }

    void onDraw(SkCanvas* canvas) override {
        this->init();
        for (bool usePic : { true, false }) {
            auto drawProc = [canvas, usePic](const sk_sp<SkAnimatedImage>& animatedImage) {
                if (usePic) {
                    sk_sp<SkPicture> pic(animatedImage->newPictureSnapshot());
                    canvas->drawPicture(pic);
                } else {
                    auto image = animatedImage->getCurrentFrame();
                    canvas->drawImage(image, 0, 0, nullptr);
                }
            };
            for (float scale : { 1.25f, 1.0f, .75f, .5f }) {
                canvas->save();
                for (auto behavior : { SkAndroidCodec::ExifOrientationBehavior::kIgnore,
                                       SkAndroidCodec::ExifOrientationBehavior::kRespect }) {
                    // Only use kRespect if the origin is not the default.
                    bool needRespect = true;
                    for (bool doCrop : { false, true }) {
                        for (bool doPostProcess : { false, true }) {
                            auto codec = SkCodec::MakeFromData(fData);
                            const auto origin = codec->getOrigin();
                            if (origin == kDefault_SkEncodedOrigin) {
                                needRespect = false;
                            }
                            auto androidCodec = SkAndroidCodec::MakeFromCodec(std::move(codec),
                                                                              behavior);
                            auto info = androidCodec->getInfo();
                            const SkISize unscaledSize = info.dimensions();

                            SkISize scaledSize = { SkScalarFloorToInt(info.width()  * scale) ,
                                                   SkScalarFloorToInt(info.height() * scale) };
                            info = info.makeDimensions(scaledSize);

                            auto cropRect = SkIRect::MakeSize(scaledSize);
                            if (doCrop) {
                                auto matrix = SkMatrix::MakeRectToRect(SkRect::Make(unscaledSize),
                                        SkRect::Make(scaledSize), SkMatrix::kFill_ScaleToFit);
                                if (behavior == SkAndroidCodec::ExifOrientationBehavior::kRespect
                                        && needRespect) {
                                    matrix.preConcat(SkEncodedOriginToMatrix(origin,
                                            unscaledSize.width(), unscaledSize.height()));
                                }
                                SkRect cropRectFloat = SkRect::Make(fCropRect);
                                matrix.mapRect(&cropRectFloat);
                                cropRectFloat.roundOut(&cropRect);
                            }

                            sk_sp<SkPicture> postProcessor = doPostProcess
                                    ? post_processor(SkRect::Make(cropRect.size())) : nullptr;
                            auto animatedImage = SkAnimatedImage::Make(std::move(androidCodec),
                                    info, cropRect, std::move(postProcessor));
                            animatedImage->setRepetitionCount(0);

                            for (int frame = 0; frame < kMaxFrames; frame++) {
                                {
                                    SkAutoCanvasRestore acr(canvas, doCrop);
                                    if (doCrop) {
                                        canvas->translate(cropRect.left(), cropRect.top());
                                    }
                                    drawProc(animatedImage);
                                }

                                canvas->translate(fTranslate, 0);
                                const auto duration = animatedImage->currentFrameDuration();
                                if (duration == SkAnimatedImage::kFinished) {
                                    break;
                                }
                                for (int i = 0; i < fStep; i++) {
                                    animatedImage->decodeNextFrame();
                                }
                            }
                        }
                    }
                    if (!needRespect) break;
                }
                canvas->restore();
                canvas->translate(0, fTranslate);
            }
        }
    }
};

DEF_GM( return new AnimatedImageGM("images/stoplight_h.webp", "stoplight", 2,
                                   // Deliberately not centered in X or Y, and shows all three
                                   // lights, but otherwise arbitrary.
                                   SkIRect::MakeLTRB(5, 6, 11, 29)); )
DEF_GM( return new AnimatedImageGM("images/flightAnim.gif", "flight", 20,
                                   // Deliberately starts in the upper left corner to exercise
                                   // a special case, but otherwise arbitrary.
                                   SkIRect::MakeLTRB(0, 0, 300, 200)); )
