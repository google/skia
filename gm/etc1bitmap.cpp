/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "Resources.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkImageGenerator.h"
#include "SkImageDecoder.h"
#include "SkOSFile.h"

#ifndef SK_IGNORE_ETC1_SUPPORT

#include "etc1.h"

/**
 *  Remove the last row and column of ETC1 blocks, effectively
 *  making a texture that started as power of two into a texture
 *  that is no longer power of two...
 */
bool slice_etc1_data(void *data, int* width, int* height) {

    // First, parse the data and get to it...
    etc1_byte *origData = reinterpret_cast<etc1_byte *>(data);
    if (!etc1_pkm_is_valid(origData)) {
        return false;
    }

    int origW = etc1_pkm_get_width(origData);
    int origH = etc1_pkm_get_height(origData);

    int blockWidth = (origW + 3) >> 2;
    int blockHeight = (origH + 3) >> 2;

    // Make sure that we have blocks to trim off..
    if (blockWidth < 2 || blockHeight < 2) {
        return false;
    }

    int newWidth = (blockWidth - 1) << 2;
    int newHeight = (blockHeight - 1) << 2;

    size_t newDataSz = etc1_get_encoded_data_size(newWidth, newHeight) + ETC_PKM_HEADER_SIZE;
    SkAutoMalloc am(newDataSz);

    etc1_byte *newData = reinterpret_cast<etc1_byte *>(am.get());

    etc1_pkm_format_header(newData, newWidth, newHeight);
    newData += ETC_PKM_HEADER_SIZE;
    origData += ETC_PKM_HEADER_SIZE;

    for (int j = 0; j < blockHeight - 1; ++j) {
        memcpy(newData, origData, (blockWidth - 1)*ETC1_ENCODED_BLOCK_SIZE);
        origData += blockWidth*ETC1_ENCODED_BLOCK_SIZE;
        newData += (blockWidth - 1)*ETC1_ENCODED_BLOCK_SIZE;
    }

    // Stick the data back whence it came
    memcpy(data, am.get(), newDataSz);
    *width = newWidth;
    *height = newHeight;

    return true;
}
#endif  // SK_IGNORE_ETC1_SUPPORT

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
    SkString onShortName() override {
        SkString str = SkString("etc1bitmap_");
        str.append(this->fileExtension());
        return str;
    }

    SkISize onISize() override {
        return SkISize::Make(128, 128);
    }

    virtual SkString fileExtension() const = 0;

    void onDraw(SkCanvas* canvas) override {
        SkBitmap bm;
        SkString filename = GetResourcePath("mandrill_128.");
        filename.append(this->fileExtension());
        SkAutoTUnref<SkData> fileData(SkData::NewFromFileName(filename.c_str()));
        if (NULL == fileData) {
            SkDebugf("Could not open the file. Did you forget to set the resourcePath?\n");
            return;
        }

        if (!SkInstallDiscardablePixelRef(fileData, &bm)) {
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

    SkString fileExtension() const override { return SkString("pkm"); }

private:
    typedef ETC1BitmapGM INHERITED;
};

// This class specializes ETC1BitmapGM to load the mandrill_128.ktx file.
class ETC1Bitmap_KTX_GM : public ETC1BitmapGM {
public:
    ETC1Bitmap_KTX_GM() : ETC1BitmapGM() { }
    virtual ~ETC1Bitmap_KTX_GM() { }

protected:

    SkString fileExtension() const override { return SkString("ktx"); }

private:
    typedef ETC1BitmapGM INHERITED;
};

// This class specializes ETC1BitmapGM to load the mandrill_128.r11.ktx file.
class ETC1Bitmap_R11_KTX_GM : public ETC1BitmapGM {
public:
    ETC1Bitmap_R11_KTX_GM() : ETC1BitmapGM() { }
    virtual ~ETC1Bitmap_R11_KTX_GM() { }

protected:

    SkString fileExtension() const override { return SkString("r11.ktx"); }

private:
    typedef ETC1BitmapGM INHERITED;
};

#ifndef SK_IGNORE_ETC1_SUPPORT
/**
 *  Test decoding an image from a PKM file and then
 *  from non-power-of-two compressed ETC1 data. First slice
 *  off a row and column of blocks in order to make it non-power
 *  of two.
 */
class ETC1Bitmap_NPOT_GM : public GM {
public:
    ETC1Bitmap_NPOT_GM() { }
    virtual ~ETC1Bitmap_NPOT_GM() { }

protected:
    SkString onShortName() override {
        return SkString("etc1bitmap_npot");
    }

    SkISize onISize() override {
        return SkISize::Make(124, 124);
    }

    void onDraw(SkCanvas* canvas) override {
        SkBitmap bm;
        SkString pkmFilename = GetResourcePath("mandrill_128.pkm");
        SkAutoDataUnref fileData(SkData::NewFromFileName(pkmFilename.c_str()));
        if (NULL == fileData) {
            SkDebugf("Could not open the file. Did you forget to set the resourcePath?\n");
            return;
        }

        SkAutoMalloc am(fileData->size());
        memcpy(am.get(), fileData->data(), fileData->size());

        int width, height;
        if (!slice_etc1_data(am.get(), &width, &height)) {
            SkDebugf("ETC1 Data is poorly formatted.\n");
            return;
        }

        SkASSERT(124 == width);
        SkASSERT(124 == height);

        size_t dataSz = etc1_get_encoded_data_size(width, height) + ETC_PKM_HEADER_SIZE;
        SkAutoDataUnref nonPOTData(SkData::NewWithCopy(am.get(), dataSz));

        if (!SkInstallDiscardablePixelRef(nonPOTData, &bm)) {
            SkDebugf("Could not install discardable pixel ref.\n");
            return;
        }

        canvas->drawBitmap(bm, 0, 0);
    }

private:
    typedef GM INHERITED;
};
#endif  // SK_IGNORE_ETC1_SUPPORT

}  // namespace skiagm

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return SkNEW(skiagm::ETC1Bitmap_PKM_GM); )
DEF_GM( return SkNEW(skiagm::ETC1Bitmap_KTX_GM); )
DEF_GM( return SkNEW(skiagm::ETC1Bitmap_R11_KTX_GM); )

#ifndef SK_IGNORE_ETC1_SUPPORT
DEF_GM( return SkNEW(skiagm::ETC1Bitmap_NPOT_GM); )
#endif  // SK_IGNORE_ETC1_SUPPORT
