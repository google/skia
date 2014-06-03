/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkDecodingImageGenerator.h"
#include "SkImageDecoder.h"
#include "SkOSFile.h"

namespace skiagm {

/**
 *  Test decoding an image from a PKM or KTX file and then
 *  from compressed ETC1 data.
 */
class ETC1BitmapGM : public GM {
public:
    ETC1BitmapGM() { }
    virtual ~ETC1BitmapGM() { }

protected:
    virtual SkString onShortName() SK_OVERRIDE {
        SkString str = SkString("etc1bitmap_");
        str.append(this->fileExtension());
        return str;
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return make_isize(128, 128);
    }

    virtual SkString fileExtension() const = 0;

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        SkBitmap bm;
        SkString filename = SkOSPath::SkPathJoin(
                INHERITED::gResourcePath.c_str(), "mandrill_128.");
        filename.append(this->fileExtension());

        SkAutoTUnref<SkData> fileData(SkData::NewFromFileName(filename.c_str()));
        if (NULL == fileData) {
            SkDebugf("Could not open the file. Did you forget to set the resourcePath?\n");
            return;
        }

        if (!SkInstallDiscardablePixelRef(
                SkDecodingImageGenerator::Create(
                    fileData, SkDecodingImageGenerator::Options()), &bm)) {
            SkDebugf("Could not install discardable pixel ref.\n");
            return;
        }

        canvas->drawBitmap(bm, 0, 0);
    }

private:
    typedef GM INHERITED;
};

// This class specializes ETC1BitmapGM to load the mandrill_128.pkm file.
class ETC1Bitmap_PKM_GM : public ETC1BitmapGM {
public:
    ETC1Bitmap_PKM_GM() : ETC1BitmapGM() { }
    virtual ~ETC1Bitmap_PKM_GM() { }

protected:

    virtual SkString fileExtension() const SK_OVERRIDE { return SkString("pkm"); }

private:
    typedef ETC1BitmapGM INHERITED;
};

// This class specializes ETC1BitmapGM to load the mandrill_128.ktx file.
class ETC1Bitmap_KTX_GM : public ETC1BitmapGM {
public:
    ETC1Bitmap_KTX_GM() : ETC1BitmapGM() { }
    virtual ~ETC1Bitmap_KTX_GM() { }

protected:

    virtual SkString fileExtension() const SK_OVERRIDE { return SkString("ktx"); }

private:
    typedef ETC1BitmapGM INHERITED;
};

}  // namespace skiagm

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return SkNEW(skiagm::ETC1Bitmap_PKM_GM); )
DEF_GM( return SkNEW(skiagm::ETC1Bitmap_KTX_GM); )
