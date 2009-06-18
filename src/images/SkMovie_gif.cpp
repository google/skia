/* libs/graphics/images/SkImageDecoder_libgif.cpp
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include "SkMovie.h"
#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkStream.h"
#include "SkTemplates.h"

#include "gif_lib.h"

class SkGIFMovie : public SkMovie {
public:
    SkGIFMovie(SkStream* stream);
    virtual ~SkGIFMovie();

protected:
    virtual bool onGetInfo(Info*);
    virtual bool onSetTime(SkMSec);
    virtual bool onGetBitmap(SkBitmap*);
    
private:
    GifFileType* fGIF;
    SavedImage* fCurrSavedImage;
};

static int Decode(GifFileType* fileType, GifByteType* out, int size) {
    SkStream* stream = (SkStream*) fileType->UserData;
    return (int) stream->read(out, size);
}

SkGIFMovie::SkGIFMovie(SkStream* stream)
{
    fGIF = DGifOpen( stream, Decode );
    if (NULL == fGIF)
        return;

    if (DGifSlurp(fGIF) != GIF_OK)
    {
        DGifCloseFile(fGIF);
        fGIF = NULL;
    }
    fCurrSavedImage = NULL;
}

SkGIFMovie::~SkGIFMovie()
{
    if (fGIF)
        DGifCloseFile(fGIF);
}

static SkMSec savedimage_duration(const SavedImage* image)
{
    for (int j = 0; j < image->ExtensionBlockCount; j++)
    {
        if (image->ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE)
        {
            int size = image->ExtensionBlocks[j].ByteCount;
            SkASSERT(size >= 4);
            const uint8_t* b = (const uint8_t*)image->ExtensionBlocks[j].Bytes;
            return ((b[2] << 8) | b[1]) * 10;
        }
    }
    return 0;
}

bool SkGIFMovie::onGetInfo(Info* info)
{
    if (NULL == fGIF)
        return false;

    SkMSec dur = 0;
    for (int i = 0; i < fGIF->ImageCount; i++)
        dur += savedimage_duration(&fGIF->SavedImages[i]);

    info->fDuration = dur;
    info->fWidth = fGIF->SWidth;
    info->fHeight = fGIF->SHeight;
    info->fIsOpaque = false;    // how to compute?
    return true;
}

bool SkGIFMovie::onSetTime(SkMSec time)
{
    if (NULL == fGIF)
        return false;

    SkMSec dur = 0;
    for (int i = 0; i < fGIF->ImageCount; i++)
    {
        dur += savedimage_duration(&fGIF->SavedImages[i]);
        if (dur >= time)
        {
            SavedImage* prev = fCurrSavedImage;
            fCurrSavedImage = &fGIF->SavedImages[i];
            return prev != fCurrSavedImage;
        }
    }
    fCurrSavedImage = &fGIF->SavedImages[fGIF->ImageCount - 1];
    return true;
}

bool SkGIFMovie::onGetBitmap(SkBitmap* bm)
{
    GifFileType* gif = fGIF;
    if (NULL == gif)
        return false;

    // should we check for the Image cmap or the global (SColorMap) first? 
    ColorMapObject* cmap = gif->SColorMap;
    if (cmap == NULL)
        cmap = gif->Image.ColorMap;

    if (cmap == NULL || gif->ImageCount < 1 || cmap->ColorCount != (1 << cmap->BitsPerPixel))
    {
        SkASSERT(!"bad colortable setup");
        return false;
    }

    const int width = gif->SWidth;
    const int height = gif->SHeight;
    if (width <= 0 || height <= 0) {
        return false;
    }

    SavedImage*      gif_image = fCurrSavedImage;
    SkBitmap::Config config = SkBitmap::kIndex8_Config;

    SkColorTable* colorTable = SkNEW_ARGS(SkColorTable, (cmap->ColorCount));
    SkAutoUnref aur(colorTable);

    bm->setConfig(config, width, height, 0);
    if (!bm->allocPixels(colorTable)) {
        return false;
    }

    int transparent = -1;
    for (int i = 0; i < gif_image->ExtensionBlockCount; ++i) {
      ExtensionBlock* eb = gif_image->ExtensionBlocks + i;
      if (eb->Function == 0xF9 && 
          eb->ByteCount == 4) {
        bool has_transparency = ((eb->Bytes[0] & 1) == 1);
        if (has_transparency) {
          transparent = (unsigned char)eb->Bytes[3];
        }
      }
    }

    SkPMColor* colorPtr = colorTable->lockColors();

    if (transparent >= 0)
        memset(colorPtr, 0, cmap->ColorCount * 4);
    else
        colorTable->setFlags(colorTable->getFlags() | SkColorTable::kColorsAreOpaque_Flag);

    for (int index = 0; index < cmap->ColorCount; index++)
    {
        if (transparent != index)
            colorPtr[index] = SkPackARGB32(0xFF, cmap->Colors[index].Red, 
                cmap->Colors[index].Green, cmap->Colors[index].Blue);
    }
    colorTable->unlockColors(true);

    unsigned char* in = (unsigned char*)gif_image->RasterBits;
    unsigned char* out = bm->getAddr8(0, 0);
    if (gif->Image.Interlace) {

      // deinterlace
        int row;
      // group 1 - every 8th row, starting with row 0
      for (row = 0; row < height; row += 8) {
        memcpy(out + width * row, in, width);
        in += width;
      }

      // group 2 - every 8th row, starting with row 4
      for (row = 4; row < height; row += 8) {
        memcpy(out + width * row, in, width);
        in += width;
      }

      // group 3 - every 4th row, starting with row 2
      for (row = 2; row < height; row += 4) {
        memcpy(out + width * row, in, width);
        in += width;
      }

      for (row = 1; row < height; row += 2) {
        memcpy(out + width * row, in, width);
        in += width;
      }

    } else {
      memcpy(out, in, width * height);
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

#include "SkTRegistry.h"

SkMovie* Factory(SkStream* stream) {
    char buf[GIF_STAMP_LEN];
    if (stream->read(buf, GIF_STAMP_LEN) == GIF_STAMP_LEN) {
        if (memcmp(GIF_STAMP,   buf, GIF_STAMP_LEN) == 0 ||
                memcmp(GIF87_STAMP, buf, GIF_STAMP_LEN) == 0 ||
                memcmp(GIF89_STAMP, buf, GIF_STAMP_LEN) == 0) {
            return SkNEW_ARGS(SkGIFMovie, (stream));
        }
    }
    return NULL;
}

static SkTRegistry<SkMovie*, SkStream*> gReg(Factory);
