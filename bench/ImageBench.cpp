/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkImage.h"
#include "SkSurface.h"

class Image2RasterBench : public Benchmark {
public:
    Image2RasterBench() {
        fName.set("native_image_to_raster_surface");
    }

    bool isSuitableFor(Backend backend) override {
        return kGPU_Backend == backend || kRaster_Backend == backend;
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    // We explicitly want to bench drawing a Image [cpu or gpu backed] into a raster target,
    // to ensure that we can cache the read-back in the case of gpu -> raster
    //
    void onPerCanvasPreDraw(SkCanvas* canvas) override {
        // create an Image reflecting the canvas (gpu or cpu)
        SkImageInfo info = SkImageInfo::MakeN32Premul(100, 100);
        SkAutoTUnref<SkSurface> surface(canvas->newSurface(info));
        canvas->drawColor(SK_ColorRED);
        fImage.reset(surface->newImageSnapshot());

        // create a cpu-backed Surface
        fRasterSurface.reset(SkSurface::NewRaster(info));
    }

    void onPerCanvasPostDraw(SkCanvas*) override {
        // Release the image and raster surface here to prevent out of order destruction
        // between these and the gpu interface.
        fRasterSurface.reset(nullptr);
        fImage.reset(nullptr);
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; i++) {
            for (int inner = 0; inner < 10; ++inner) {
                fRasterSurface->getCanvas()->drawImage(fImage, 0, 0);
            }
        }
    }

private:
    SkString                fName;
    SkAutoTUnref<SkImage>   fImage;
    SkAutoTUnref<SkSurface> fRasterSurface;

    typedef Benchmark INHERITED;
};
DEF_BENCH( return new Image2RasterBench; )

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkOffsetImageFilter.h"

#if SK_SUPPORT_GPU
#include "SkGrPixelRef.h"
#endif

enum MyDrawType {
    kSprite_Type,
    kBitmap_Type,
    kImage_Type,
};

/*
 *  Want to time drawing images/bitmaps via drawSprite, and via drawBitmap/drawImage but with
 *  a non-scaling matrix and a clip that is tight to the image bounds. In this scenario, we
 *  should be able to match the speed of drawSprite.
 *
 *  An optimal result should be that all three types: sprite/bitmap/image draw at the same speed.
 */
class ImageFilterSpriteBench : public Benchmark {
    SkAutoTUnref<SkImage>       fImage;
    SkBitmap                    fBitmap;
    SkString                    fName;
    MyDrawType                  fType;

public:
    ImageFilterSpriteBench(MyDrawType dt, const char suffix[]) : fType(dt) {
        fName.printf("image-filter-sprite-draw-%s", suffix);
    }

    bool isSuitableFor(Backend backend) override {
        return kGPU_Backend == backend || kRaster_Backend == backend;
    }

protected:
    bool isVisual() override {
        return true;
    }

    const char* onGetName() override {
        return fName.c_str();
    }

    void onPerCanvasPreDraw(SkCanvas* canvas) override {
        const SkImageInfo info = SkImageInfo::MakeN32Premul(500, 500);
        SkAutoTUnref<SkSurface> surface(canvas->newSurface(info));

        surface->getCanvas()->drawColor(SK_ColorRED);
        fImage.reset(surface->newImageSnapshot());

        fBitmap.setInfo(info);
        if (fImage->getTexture()) {
#if SK_SUPPORT_GPU
            fBitmap.setPixelRef(new SkGrPixelRef(info, fImage->getTexture()))->unref();
#endif
        } else {
            SkPixmap pmap;
            if (!fImage->peekPixels(&pmap)) {
                sk_throw();
            }
            fBitmap.installPixels(pmap);
        }
    }

    void onPerCanvasPostDraw(SkCanvas*) override {
        // Release the image and raster surface here to prevent out of order destruction
        // between these and the gpu interface.
        fImage.reset(nullptr);
        fBitmap.reset();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        // This clip is important; it allows the drawImage/drawBitmap code to fall into the
        // fast (sprite) case, since the imagefilter's output should match.
        //
        // When we address skbug.com/4526 we should be able to remove the need for this clip.
        //
        canvas->clipRect(SkRect::MakeIWH(fImage->width(), fImage->height()));

        const SkScalar kDelta = 10;
        SkPaint paint;
        for (int i = 0; i < loops; i++) {
            for (int inner = 0; inner < 10; ++inner) {
                // build the filter everytime, so we don't accidentally draw a cached version,
                // since the point of this bench is to time the actual imagefilter
                // handling/overhead.
                SkAutoTUnref<SkImageFilter> filter(SkOffsetImageFilter::Create(kDelta, kDelta));
                paint.setImageFilter(filter);

                switch (fType) {
                    case kSprite_Type:
                        canvas->drawSprite(fBitmap, 0, 0, &paint);
                        break;
                    case kBitmap_Type:
                        canvas->drawBitmap(fBitmap, 0, 0, &paint);
                        break;
                    case kImage_Type:
                        canvas->drawImage(fImage, 0, 0, &paint);
                        break;
                }
            }
        }
    }

private:
    typedef Benchmark INHERITED;
};
DEF_BENCH( return new ImageFilterSpriteBench(kSprite_Type, "sprite"); )
DEF_BENCH( return new ImageFilterSpriteBench(kBitmap_Type, "bitmap"); )
DEF_BENCH( return new ImageFilterSpriteBench(kImage_Type,  "image"); )

