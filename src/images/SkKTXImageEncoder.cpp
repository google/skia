/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorPriv.h"
#include "SkImageEncoder.h"
#include "SkImageGenerator.h"
#include "SkPixelRef.h"
#include "SkStream.h"
#include "SkStreamPriv.h"
#include "SkTypes.h"

#include "ktx.h"
#include "etc1.h"

///////////////////////////////////////////////////////////////////////////////

// KTX Image Encoder
//
// KTX is a general texture data storage file format ratified by the Khronos Group. As an
// overview, a KTX file contains all of the appropriate values needed to fully specify a
// texture in an OpenGL application, including the use of compressed data.
//
// This encoder takes a best guess at how to encode the bitmap passed to it. If
// there is an installed discardable pixel ref with existing PKM data, then we
// will repurpose the existing ETC1 data into a KTX file. If the data contains
// KTX data, then we simply return a copy of the same data. For all other files,
// the underlying KTX library tries to do its best to encode the appropriate
// data specified by the bitmap based on the config. (i.e. kAlpha8_Config will
// be represented as a full resolution 8-bit image dump with the appropriate
// OpenGL defines in the header).

class SkKTXImageEncoder : public SkImageEncoder {
protected:
    bool onEncode(SkWStream* stream, const SkBitmap& bm, int quality) override;

private:
    virtual bool encodePKM(SkWStream* stream, const SkData *data);
    typedef SkImageEncoder INHERITED;
};

bool SkKTXImageEncoder::onEncode(SkWStream* stream, const SkBitmap& bitmap, int) {
    if (!bitmap.pixelRef()) {
        return false;
    }
    SkAutoDataUnref data(bitmap.pixelRef()->refEncodedData());

    // Is this even encoded data?
    if (data) {
        const uint8_t *bytes = data->bytes();
        if (etc1_pkm_is_valid(bytes)) {
            return this->encodePKM(stream, data);
        }

        // Is it a KTX file??
        if (SkKTXFile::is_ktx(bytes)) {
            return stream->write(bytes, data->size());
        }

        // If it's neither a KTX nor a PKM, then we need to
        // get at the actual pixels, so fall through and decompress...
    }

    return SkKTXFile::WriteBitmapToKTX(stream, bitmap);
}

bool SkKTXImageEncoder::encodePKM(SkWStream* stream, const SkData *data) {
    const uint8_t* bytes = data->bytes();
    SkASSERT(etc1_pkm_is_valid(bytes));

    etc1_uint32 width = etc1_pkm_get_width(bytes);
    etc1_uint32 height = etc1_pkm_get_height(bytes);

    // ETC1 Data is stored as compressed 4x4 pixel blocks, so we must make sure
    // that our dimensions are valid.
    if (width == 0 || (width & 3) != 0 || height == 0 || (height & 3) != 0) {
        return false;
    }

    // Advance pointer to etc1 data.
    bytes += ETC_PKM_HEADER_SIZE;

    return SkKTXFile::WriteETC1ToKTX(stream, bytes, width, height);
}

/////////////////////////////////////////////////////////////////////////////////////////
DEFINE_ENCODER_CREATOR(KTXImageEncoder);
/////////////////////////////////////////////////////////////////////////////////////////

SkImageEncoder* sk_libktx_efactory(SkImageEncoder::Type t) {
    return (SkImageEncoder::kKTX_Type == t) ? new SkKTXImageEncoder : nullptr;
}

static SkImageEncoder_EncodeReg gEReg(sk_libktx_efactory);
