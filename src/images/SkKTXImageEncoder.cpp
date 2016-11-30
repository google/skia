/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImageEncoderPriv.h"

#include "SkColorPriv.h"
#include "SkImageGenerator.h"
#include "SkPixelRef.h"
#include "SkStream.h"
#include "SkStreamPriv.h"

#include "ktx.h"
#include "etc1.h"

///////////////////////////////////////////////////////////////////////////////

// KTX Image Encoder
//
// KTX is a general texture data storage file format ratified by the Khronos Group. As an
// overview, a KTX file contains all of the appropriate values needed to fully specify a
// texture in an OpenGL application, including the use of compressed data.
//
// This encoder takes a best guess at how to encode the bitmap passed to it.
// The KTX library tries to do its best to encode the appropriate
// data specified by the bitmap based on the config. (i.e. kAlpha8_Config will
// be represented as a full resolution 8-bit image dump with the appropriate
// OpenGL defines in the header).

bool SkEncodeImageAsKTX(SkWStream* stream, const SkPixmap& pixmap) {
    return SkKTXFile::WritePixmapToKTX(stream, pixmap);
}
