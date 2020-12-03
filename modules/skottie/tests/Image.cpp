/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSurface.h"
#include "modules/skottie/include/Skottie.h"
#include "tests/Test.h"

using namespace skottie;

DEF_TEST(Skottie_Image_CustomTransform, r) {
    static constexpr char json[] =
        R"({
             "v": "5.2.1",
             "w": 100,
             "h": 100,
             "fr": 10,
             "ip": 0,
             "op": 100,
             "assets": [{
               "id": "img_0",
               "p" : "img_0.png",
               "u" : "images/",
               "w" : 100,
               "h" :  50
             }],
             "layers": [
               {
                 "ip": 0,
                 "op": 100,
                 "ty": 2,
                 "refId": "img_0",
                 "ks": {
                   "p": { "a": 0, "k": [0,25] }
                 }
               }
             ]
           })";

    SkMemoryStream stream(json, strlen(json));

    static const struct TestData {
        float    t;
        SkMatrix m;
        SkColor  c[5]; // expected color samples at center/L/T/R/B
    } tests[] {
        { 0, SkMatrix::I(),
            {0xffff0000, 0xffff0000, 0xff00ff00, 0xffff0000, 0xff00ff00}},
        { 1, SkMatrix::Translate(50,25) * SkMatrix::Scale(.5f,.5f) * SkMatrix::Translate(-50,-25),
            {0xffff0000, 0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00}},
        { 2, SkMatrix::Translate(-50, 0),
            {0xff00ff00, 0xffff0000, 0xff00ff00, 0xff00ff00, 0xff00ff00}},
        { 3, SkMatrix::Translate(0, -25),
            {0xff00ff00, 0xff00ff00, 0xffff0000, 0xff00ff00, 0xff00ff00}},
        { 4, SkMatrix::Translate(50, 0),
            {0xffff0000, 0xff00ff00, 0xff00ff00, 0xffff0000, 0xff00ff00}},
        { 5, SkMatrix::Translate(0, 25),
            {0xffff0000, 0xffff0000, 0xff00ff00, 0xffff0000, 0xffff0000}},
    };

    class TestImageAsset final : public ImageAsset {
    public:
        TestImageAsset(const TestData* tst, skiatest::Reporter* r)
            : fTest(tst)
            , fReporter(r) {

            auto surf = SkSurface::MakeRasterN32Premul(200, 100);
            surf->getCanvas()->drawColor(0xffff0000);
            fImage = surf->makeImageSnapshot();
        }

    private:
        bool isMultiFrame() override { return true; }

        FrameData getFrameData(float t) override {
            REPORTER_ASSERT(fReporter, t == fTest->t);

            return { fImage, SkSamplingOptions(), fTest++->m };
        }

        sk_sp<SkImage>      fImage;
        const TestData*     fTest;
        skiatest::Reporter* fReporter;
    };

    class TestResourceProvider final : public ResourceProvider {
    public:
        TestResourceProvider(const TestData* tst, skiatest::Reporter* r)
            : fTest(tst)
            , fReporter(r) {}

    private:
        sk_sp<ImageAsset> loadImageAsset(const char[], const char[], const char[]) const override {
            return sk_make_sp<TestImageAsset>(fTest, fReporter);
        }

        const TestData*      fTest;
        skiatest::Reporter*  fReporter;
    };

    auto anim = Animation::Builder()
                    .setResourceProvider(sk_make_sp<TestResourceProvider>(tests, r))
                    .make(&stream);

    REPORTER_ASSERT(r, anim);

    static constexpr SkSize render_size{100, 100};
    auto surf = SkSurface::MakeRasterN32Premul(render_size.width(), render_size.height());
    auto rect = SkRect::MakeSize(render_size);

    SkPixmap pmap;
    surf->peekPixels(&pmap);

    for (const auto& tst : tests) {
        surf->getCanvas()->clear(0xff00ff00);
        anim->seekFrameTime(tst.t);
        anim->render(surf->getCanvas(), &rect);

        REPORTER_ASSERT(r,
            tst.c[0] == pmap.getColor(render_size.width() / 2, render_size.height() / 2));
        REPORTER_ASSERT(r,
            tst.c[1] == pmap.getColor(1                      , render_size.height() / 2));
        REPORTER_ASSERT(r,
            tst.c[2] == pmap.getColor(render_size.width() / 2, 1));
        REPORTER_ASSERT(r,
            tst.c[3] == pmap.getColor(render_size.width() - 1, render_size.height() / 2));
        REPORTER_ASSERT(r,
            tst.c[4] == pmap.getColor(render_size.width() /2 , render_size.height() - 1));
    }
}
