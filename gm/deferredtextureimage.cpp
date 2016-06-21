/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <vector>

#include "gm.h"
#include "GrContext.h"
#include "SkMipMap.h"
#include "Resources.h"

#if SK_SUPPORT_GPU

class DeferredTextureImage_Medium : public skiagm::GM {
protected:
    SkString onShortName() override {
        return SkString("deferredtextureimage_medium");
    }

    SkISize onISize() override {
        return SkISize::Make(512 + 512 + 30, 1110);
    }

    void onDraw(SkCanvas* canvas) override {
        GrContext* context = canvas->getGrContext();
        if (!context) {
            skiagm::GM::DrawGpuOnlyMessage(canvas);
            return;
        }

        SkPaint paint;
        paint.setFilterQuality(kMedium_SkFilterQuality);

        int mipLevelCount = SkMipMap::ComputeLevelCount(512, 512);

        // create the deferred texture image
        SkImage::DeferredTextureImageUsageParams params[1];
        // These should cause mipmaps to be generated
        params[0].fMatrix = SkMatrix::MakeScale(0.25f, 0.25f);
        params[0].fQuality = kMedium_SkFilterQuality;
        size_t requiredMemoryInBytes = fDecodedImage->getDeferredTextureImageData(
            *context->threadSafeProxy(), params, 1, nullptr, SkSourceGammaTreatment::kRespect);
        if (requiredMemoryInBytes == 0) {
            SkDebugf("\nCould not create DeferredTextureImageData.\n");
            return;
        }

        std::vector<uint8_t> memory;
        memory.resize(requiredMemoryInBytes);
        fDecodedImage->getDeferredTextureImageData(
            *context->threadSafeProxy(), params, 1, memory.data(),
            SkSourceGammaTreatment::kRespect);
        sk_sp<SkImage> uploadedImage = SkImage::MakeFromDeferredTextureImageData(
            context, memory.data(), SkBudgeted::kNo);

        // draw a column using deferred texture images
        SkScalar offsetHeight = 10.f;
        // handle base mipmap level
        canvas->save();
        canvas->translate(10.f, offsetHeight);
        canvas->drawImage(uploadedImage, 0, 0, &paint);
        canvas->restore();
        offsetHeight += 512 + 10;
        // handle generated mipmap levels
        for (int i = 0; i < mipLevelCount; i++) {
            SkISize mipSize = SkMipMap::ComputeLevelSize(512, 512, i);
            canvas->save();
            canvas->translate(10.f, offsetHeight);
            canvas->scale(mipSize.width() / 512.f, mipSize.height() / 512.f);
            canvas->drawImage(uploadedImage, 0, 0, &paint);
            canvas->restore();
            offsetHeight += mipSize.height() + 10;
        }

        // draw a column using SkImage
        offsetHeight = 10;
        // handle base mipmap level
        canvas->save();
        canvas->translate(512.f + 20.f, offsetHeight);
        canvas->drawImage(fDecodedImage, 0, 0, &paint);
        canvas->restore();
        offsetHeight += 512 + 10;
        // handle generated mipmap levels
        for (int i = 0; i < mipLevelCount; i++) {
            SkISize mipSize = SkMipMap::ComputeLevelSize(512, 512, i);
            canvas->save();
            canvas->translate(512.f + 20.f, offsetHeight);
            canvas->scale(mipSize.width() / 512.f, mipSize.height() / 512.f);
            canvas->drawImage(fDecodedImage, 0, 0, &paint);
            canvas->restore();
            offsetHeight += mipSize.height() + 10;
        }
    }

private:

    void onOnceBeforeDraw() override {
        SkBitmap bitmap;
        if (!GetResourceAsBitmap("mandrill_512.png", &bitmap)) {
            bitmap.allocN32Pixels(1, 1);
            bitmap.eraseARGB(255, 255, 0, 0); // red == bad
        }
        fDecodedImage = SkImage::MakeFromBitmap(bitmap);
    }

    sk_sp<SkImage> fDecodedImage;

    typedef skiagm::GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new DeferredTextureImage_Medium; )

#endif
