/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h" // IWYU pragma: keep

#if !defined(SK_BUILD_FOR_GOOGLE3)

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "third_party/etc1/etc1.h"

class GrContext;
class GrRenderTargetContext;

// Basic test of Ganesh's ETC1 support
class ETC1GM : public skiagm::GpuGM {
public:
    ETC1GM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:
    SkString onShortName() override {
        return SkString("etc1");
    }

    SkISize onISize() override {
        return SkISize::Make(kTexWidth + 2*kPad, kTexHeight + 2*kPad);
    }

    void onOnceBeforeDraw() override {
        SkBitmap bm;
        SkImageInfo ii = SkImageInfo::Make(kTexWidth, kTexHeight, kRGB_565_SkColorType,
                                           kOpaque_SkAlphaType);
        bm.allocPixels(ii);

        bm.erase(SK_ColorBLUE, SkIRect::MakeWH(kTexWidth, kTexHeight));

        for (int y = 0; y < kTexHeight; y += 4) {
            for (int x = 0; x < kTexWidth; x += 4) {
                bm.erase((x+y) % 8 ? SK_ColorRED : SK_ColorGREEN, SkIRect::MakeXYWH(x, y, 4, 4));
            }
        }

        int size = etc1_get_encoded_data_size(bm.width(), bm.height());
        fETC1Data = SkData::MakeUninitialized(size);

        unsigned char* pixels = (unsigned char*) fETC1Data->writable_data();

        if (etc1_encode_image((unsigned char*) bm.getAddr16(0, 0),
                              bm.width(), bm.height(), 2, bm.rowBytes(), pixels)) {
            fETC1Data = nullptr;
        }
    }

    void onDraw(GrContext* context, GrRenderTargetContext*, SkCanvas* canvas) override {
        sk_sp<SkImage> image = SkImage::MakeFromCompressed(context, fETC1Data,
                                                           kTexWidth, kTexHeight,
                                                           SkImage::kETC1_CompressionType);

        canvas->drawImage(image, 0, 0);
    }

private:
    static const int kPad = 8;
    static const int kTexWidth = 16;
    static const int kTexHeight = 20;

    sk_sp<SkData> fETC1Data;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ETC1GM;)

#endif
