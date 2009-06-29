#include "SkGLTextCache.h"
#include "SkScalerContext.h"
#include "SkTSearch.h"

const GLenum gTextTextureFormat = GL_ALPHA;
const GLenum gTextTextureType = GL_UNSIGNED_BYTE;

SkGLTextCache::Strike::Strike(Strike* next, int width, int height) {
    fStrikeWidth = SkNextPow2(SkMax32(kMinStrikeWidth, width));
    fStrikeHeight = SkNextPow2(height);
    fGlyphCount = 0;
    fNextFreeOffsetX = 0;
    fNext = next;

    fStrikeWidthShift = SkNextLog2(fStrikeWidth);
    fStrikeHeightShift = SkNextLog2(fStrikeHeight);
    
    if (next) {
        SkASSERT(next->fStrikeHeight == fStrikeHeight);
    }

    // create an empty texture to receive glyphs
    fTexName = 0;
    glGenTextures(1, &fTexName);
    glBindTexture(GL_TEXTURE_2D, fTexName);
    glTexImage2D(GL_TEXTURE_2D, 0, gTextTextureFormat, 
                 fStrikeWidth, fStrikeHeight, 0,
                 gTextTextureFormat, gTextTextureType, NULL);
    
    SK_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    SK_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    SK_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    SK_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);    
}

SkGLTextCache::Strike::~Strike() {
    if (fTexName != 0) {
        glDeleteTextures(1, &fTexName);
    }
}

SkGLTextCache::Strike*
SkGLTextCache::Strike::findGlyph(const SkGlyph& glyph, int* offset) {
    Strike* strike = this;
    SkDEBUGCODE(const int height = SkNextPow2(glyph.fHeight);)
    
    do {
        SkASSERT(height == strike->fStrikeHeight);
        
        int index = SkTSearch(strike->fGlyphIDArray, strike->fGlyphCount,
                              glyph.fID, sizeof(strike->fGlyphIDArray[0]));
        if (index >= 0) {
            if (offset) {
                *offset = strike->fGlyphOffsetX[index];
            }
            return strike;
        }
        strike = strike->fNext;
    } while (NULL != strike);
    return NULL;
}

static void make_a_whole(void* buffer, int index, int count, size_t elemSize) {
    SkASSERT(index >= 0 && index <= count);
    size_t offset = index * elemSize;
    memmove((char*)buffer + offset + elemSize,
            (const char*)buffer + offset,
            (count - index) * elemSize);
}

SkGLTextCache::Strike*
SkGLTextCache::Strike::addGlyphAndBind(const SkGlyph& glyph,
                                       const uint8_t image[], int* offset) {
#ifdef SK_DEBUG
    SkASSERT(this->findGlyph(glyph, NULL) == NULL);
    const int height = SkNextPow2(glyph.fHeight);
    SkASSERT(height <= fStrikeHeight && height > (fStrikeHeight >> 1));
#endif

    int rowBytes = glyph.rowBytes();
    SkASSERT(rowBytes >= glyph.fWidth);

    Strike* strike;
    if (fGlyphCount == kMaxGlyphCount ||
            fNextFreeOffsetX + rowBytes >= fStrikeWidth) {
        // this will bind the next texture for us
//        SkDebugf("--- extend strike %p\n", this);
        strike = SkNEW_ARGS(Strike, (this, rowBytes, glyph.fHeight));
    } else {
        glBindTexture(GL_TEXTURE_2D, fTexName);
        strike = this;
    }
    
    uint32_t* idArray = strike->fGlyphIDArray;
    uint16_t* offsetArray = strike->fGlyphOffsetX;
    const int glyphCount = strike->fGlyphCount;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, strike->fNextFreeOffsetX, 0, rowBytes,
                    glyph.fHeight, gTextTextureFormat, gTextTextureType,
                    image);

    // need to insert the offset
    int index = SkTSearch(idArray, glyphCount, glyph.fID, sizeof(idArray[0]));
    SkASSERT(index < 0);
    index = ~index; // this is where we should insert it
    make_a_whole(idArray, index, glyphCount, sizeof(idArray));
    make_a_whole(offsetArray, index, glyphCount, sizeof(offsetArray[0]));
    idArray[index] = glyph.fID;
    offsetArray[index] = strike->fNextFreeOffsetX;
    if (offset) {
        *offset = strike->fNextFreeOffsetX;
    }

#if 0
    SkDebugf("--- strike %p glyph %x [%d %d] offset %d count %d\n",
             strike, glyph.fID, glyph.fWidth, glyph.fHeight,
             strike->fNextFreeOffsetX, glyphCount + 1);
#endif

    // now update our header
    strike->fGlyphCount = glyphCount + 1;
    strike->fNextFreeOffsetX += glyph.fWidth;
    return strike;
}

///////////////////////////////////////////////////////////////////////////////

SkGLTextCache::SkGLTextCache() {
    sk_bzero(fStrikeList, sizeof(fStrikeList));
}

SkGLTextCache::~SkGLTextCache() {
    this->deleteAllStrikes(true);
}

void SkGLTextCache::deleteAllStrikes(bool texturesAreValid) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(fStrikeList); i++) {
        Strike* strike = fStrikeList[i];
        while (strike != NULL) {
            Strike* next = strike->fNext;
            if (!texturesAreValid) {
                strike->abandonTexture();
            }
            SkDELETE(strike);
            strike = next;
        }
    }
    sk_bzero(fStrikeList, sizeof(fStrikeList));
}

SkGLTextCache::Strike* SkGLTextCache::findGlyph(const SkGlyph& glyph,
                                                int* offset) {
    SkASSERT(glyph.fWidth != 0);
    SkASSERT(glyph.fHeight != 0);

    size_t index = SkNextLog2(glyph.fHeight);
    if (index >= SK_ARRAY_COUNT(fStrikeList)) {
        // too big for us to cache;
        return NULL;
    }

    Strike* strike = fStrikeList[index];
    if (strike) {
        strike = strike->findGlyph(glyph, offset);
    }
    return strike;
}

SkGLTextCache::Strike* SkGLTextCache::addGlyphAndBind(const SkGlyph& glyph,
                                        const uint8_t image[], int* offset) {
    SkASSERT(image != NULL);
    SkASSERT(glyph.fWidth != 0);
    SkASSERT(glyph.fHeight != 0);
    
    size_t index = SkNextLog2(glyph.fHeight);
    if (index >= SK_ARRAY_COUNT(fStrikeList)) {
        // too big for us to cache;
        return NULL;
    }
    
    Strike* strike = fStrikeList[index];
    if (NULL == strike) {
        strike = SkNEW_ARGS(Strike, (NULL, glyph.rowBytes(), glyph.fHeight));
//        SkDebugf("--- create strike [%d] %p cache %p\n", index, strike, this);
    }
    strike = strike->addGlyphAndBind(glyph, image, offset);
    fStrikeList[index] = strike;
    return strike;
}

