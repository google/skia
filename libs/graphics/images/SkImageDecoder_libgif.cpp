#include "SkImageDecoder.h"
#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkStream.h"
#include "SkTemplates.h"

class SkGIFImageDecoder : public SkImageDecoder {
protected:
	virtual bool onDecode(SkStream* stream, SkBitmap* bm, SkBitmap::Config pref);
};


extern "C" {
#include "gif_lib.h"
}

#define GIF_STAMP		"GIF"	 /* First chars in file - GIF stamp. */
#define GIF_STAMP_LEN	(sizeof(GIF_STAMP) - 1)

SkImageDecoder* SkImageDecoder_GIF_Factory(SkStream* stream)
{
	char buf[GIF_STAMP_LEN];
	if (stream->read(buf, GIF_STAMP_LEN) == GIF_STAMP_LEN &&
        memcmp(GIF_STAMP, buf, GIF_STAMP_LEN) == 0)
    {
        stream->rewind();
        return SkNEW(SkGIFImageDecoder);
    }
    return NULL;
}

static int Decode(GifFileType* fileType, GifByteType* out, int size)
{
	SkStream* stream = (SkStream*) fileType->UserData;
	return (int) stream->read(out, size);
}

bool SkGIFImageDecoder::onDecode(SkStream* sk_stream, SkBitmap* bm, SkBitmap::Config prefConfig)
{	
	GifFileType* gif = DGifOpen( sk_stream, Decode );
	if (gif == nil)
		return false;
    if (DGifSlurp(gif) != GIF_OK)
		return false;
    ColorMapObject* cmap = gif->SColorMap;
    if (cmap == 0 ||
        gif->ImageCount < 1 ||
        cmap->ColorCount != (1 << cmap->BitsPerPixel))
			return false;
    SavedImage* gif_image = gif->SavedImages + 0;
    const int width = gif->SWidth;
    const int height = gif->SHeight;
	SkBitmap::Config	config = SkBitmap::kIndex8_Config;
	bm->setConfig(config, width, height, 0);
	bm->allocPixels();
	
	SkColorTable* colorTable = SkNEW(SkColorTable);
	colorTable->setColors(cmap->ColorCount);
	bm->setColorTable(colorTable)->unref();

    int transparent = -1;
    for (int i = 0; i < gif_image->ExtensionBlockCount; ++i) {
      ExtensionBlock* eb = gif_image->ExtensionBlocks + i;
      if (eb->Function == 0xF9 && 
          eb->ByteCount == 4) {
        bool has_transparency = ((eb->Bytes[0] & 1) == 1);
        if (has_transparency) {
          transparent = eb->Bytes[3];
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
			colorPtr[index] = SkColorSetRGB(cmap->Colors[index].Red, 
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

    DGifCloseFile(gif);
	return true;
}
