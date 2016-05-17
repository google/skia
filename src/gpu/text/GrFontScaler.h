/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFontScaler_DEFINED
#define GrFontScaler_DEFINED

#include "GrGlyph.h"
#include "GrTypes.h"

#include "SkDescriptor.h"

class SkGlyph;
class SkPath;

/*
 *  This is Gr's interface to the host platform's font scaler.
 *
 *  The client is responsible for instantiating this. The instance is created
 *  for a specific font+size+matrix.
 */
class GrFontScaler final : public SkNoncopyable {
public:
    explicit GrFontScaler(SkGlyphCache* strike);

    const SkDescriptor& getKey();
    GrMaskFormat getPackedGlyphMaskFormat(const SkGlyph&) const;
    bool getPackedGlyphBounds(const SkGlyph&, SkIRect* bounds);
    bool getPackedGlyphImage(const SkGlyph&, int width, int height, int rowBytes,
                             GrMaskFormat expectedMaskFormat, void* image);
    bool getPackedGlyphDFBounds(const SkGlyph&, SkIRect* bounds);
    bool getPackedGlyphDFImage(const SkGlyph&, int width, int height, void* image);
    const SkPath* getGlyphPath(const SkGlyph&);
    const SkGlyph& grToSkGlyph(GrGlyph::PackedID);

private:
    // The SkGlyphCache actually owns this GrFontScaler. The GrFontScaler is deleted when the
    // SkGlyphCache is deleted.
    SkGlyphCache*  fStrike;

    typedef SkNoncopyable INHERITED;
};

#endif
