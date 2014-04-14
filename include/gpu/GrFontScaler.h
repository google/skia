/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFontScaler_DEFINED
#define GrFontScaler_DEFINED

#include "GrGlyph.h"
#include "GrKey.h"

class SkPath;

/**
 *  This is a virtual base class which Gr's interface to the host platform's
 *  font scaler.
 *
 *  The client is responsible for subclassing, and instantiating this. The
 *  instance is created for a specific font+size+matrix.
 */
class GrFontScaler : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrFontScaler)

    virtual const GrKey* getKey() = 0;
    virtual GrMaskFormat getMaskFormat() = 0;
    virtual bool getPackedGlyphBounds(GrGlyph::PackedID, SkIRect* bounds) = 0;
    virtual bool getPackedGlyphImage(GrGlyph::PackedID, int width, int height,
                                     int rowBytes, void* image) = 0;
    // get bounds for distance field associated with packed ID
    virtual bool getPackedGlyphDFBounds(GrGlyph::PackedID, SkIRect* bounds) = 0;
    // copies distance field bytes into pre-allocated dfImage
    // (should be width*height bytes in size)
    virtual bool getPackedGlyphDFImage(GrGlyph::PackedID, int width, int height,
                                       void* dfImage) = 0;
    virtual bool getGlyphPath(uint16_t glyphID, SkPath*) = 0;

private:
    typedef SkRefCnt INHERITED;
};

#endif
