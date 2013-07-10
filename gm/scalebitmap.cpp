/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkImageDecoder.h"
#include "SkStream.h"

class ScaleBitmapGM : public skiagm::GM {

public:

    ScaleBitmapGM(const char filename[], float scale)
        : fFilename(filename), fScale(scale)
    {
        this->setBGColor(0xFFDDDDDD);
        fName.printf("scalebitmap_%s_%f", filename, scale);

        SkString path(skiagm::GM::gResourcePath);
        path.append("/");
        path.append(fFilename);

        SkImageDecoder *codec = NULL;
        SkFILEStream stream(path.c_str());
        if (stream.isValid()) {
            codec = SkImageDecoder::Factory(&stream);
        }
        if (codec) {
            stream.rewind();
            codec->decode(&stream, &fBM, SkBitmap::kARGB_8888_Config,
                SkImageDecoder::kDecodePixels_Mode);
            SkDELETE(codec);
        } else {
            fBM.setConfig(SkBitmap::kARGB_8888_Config, 1, 1);
            fBM.allocPixels();
            *(fBM.getAddr32(0,0)) = 0xFF0000FF; // red == bad
        }
        fSize = fBM.height();
    }

protected:


    SkBitmap    fBM;
    SkString    fName;
    SkString    fFilename;
    int         fSize;
    float       fScale;


    virtual SkString onShortName() SK_OVERRIDE {
        return fName;
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(fBM.width() * fScale, fBM.height() * fScale);
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        SkBitmap dst;
        dst.setConfig(SkBitmap::kARGB_8888_Config, fBM.width() * fScale, fBM.height() * fScale);
        dst.allocPixels();
        fBM.scale(&dst);

        canvas->drawBitmap(dst, 0, 0);
    }

private:
    typedef skiagm::GM INHERITED;
};

class ScaleBitmapMipmapGM: public ScaleBitmapGM {
    SkMatrix fMatrix;

public:
    ScaleBitmapMipmapGM(const char filename[], float scale)
        : INHERITED(filename, scale)
    {
        fName.printf("scalebitmap_mipmap_%s_%f", filename, scale);
        fBM.buildMipMap();
        fMatrix.setScale(scale, scale);
    }
protected:
    virtual void onDraw(SkCanvas *canvas) SK_OVERRIDE {
        SkPaint paint;

        paint.setFilterBitmap(true);
        canvas->drawBitmapMatrix(fBM, fMatrix, &paint);
    }
private:
     typedef ScaleBitmapGM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new ScaleBitmapGM("mandrill_128.png", 2); )
DEF_GM( return new ScaleBitmapGM("mandrill_64.png", 4); )
DEF_GM( return new ScaleBitmapGM("mandrill_32.png", 8); )
DEF_GM( return new ScaleBitmapGM("mandrill_16.png", 16); )

DEF_GM( return new ScaleBitmapGM("nature.jpg", 0.5f); )
DEF_GM( return new ScaleBitmapGM("nature.jpg", 0.25f); )
DEF_GM( return new ScaleBitmapGM("nature.jpg", 0.125f); )
DEF_GM( return new ScaleBitmapGM("nature.jpg", 0.0625f); )

DEF_GM( return new ScaleBitmapMipmapGM("nature.jpg", 0.5f); )
DEF_GM( return new ScaleBitmapMipmapGM("nature.jpg", 0.25f); )
DEF_GM( return new ScaleBitmapMipmapGM("nature.jpg", 0.125f); )
DEF_GM( return new ScaleBitmapMipmapGM("nature.jpg", 0.0625f); )
