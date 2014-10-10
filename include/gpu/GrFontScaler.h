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

class SkPath;

/*
 *  Wrapper class to turn a font cache descriptor into a key 
 *  for GrFontScaler-related lookups
 */
class GrFontDescKey : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrFontDescKey)
    
    typedef uint32_t Hash;
    
    explicit GrFontDescKey(const SkDescriptor& desc);
    virtual ~GrFontDescKey();
    
    Hash getHash() const { return fHash; }
    
    bool operator<(const GrFontDescKey& rh) const {
        return fHash < rh.fHash || (fHash == rh.fHash && this->lt(rh));
    }
    bool operator==(const GrFontDescKey& rh) const {
        return fHash == rh.fHash && this->eq(rh);
    }
    
private:
    // helper functions for comparisons
    bool lt(const GrFontDescKey& rh) const;
    bool eq(const GrFontDescKey& rh) const;
    
    SkDescriptor* fDesc;
    enum {
        kMaxStorageInts = 16
    };
    uint32_t fStorage[kMaxStorageInts];
    const Hash fHash;
    
    typedef SkRefCnt INHERITED;
};

/*
 *  This is Gr's interface to the host platform's font scaler.
 *
 *  The client is responsible for instantiating this. The instance is created 
 *  for a specific font+size+matrix.
 */
class GrFontScaler : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrFontScaler)

    explicit GrFontScaler(SkGlyphCache* strike);
    virtual ~GrFontScaler();
    
    const GrFontDescKey* getKey();
    GrMaskFormat getMaskFormat();
    bool getPackedGlyphBounds(GrGlyph::PackedID, SkIRect* bounds);
    bool getPackedGlyphImage(GrGlyph::PackedID, int width, int height,
                                     int rowBytes, void* image);
    bool getPackedGlyphDFBounds(GrGlyph::PackedID, SkIRect* bounds);
    bool getPackedGlyphDFImage(GrGlyph::PackedID, int width, int height,
                                       void* image);
    bool getGlyphPath(uint16_t glyphID, SkPath*);
    
private:
    SkGlyphCache*  fStrike;
    GrFontDescKey* fKey;
    
    typedef SkRefCnt INHERITED;
};

#endif
