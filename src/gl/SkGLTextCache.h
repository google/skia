#ifndef SkGLTextCache_DEFINED
#define SkGLTextCache_DEFINED

#include "SkGL.h"

class SkGlyph;

class SkGLTextCache {
public:
    SkGLTextCache();
    ~SkGLTextCache();
    
    /** Delete all of the strikes in the cache. Pass true if the texture IDs are
        still valid, in which case glDeleteTextures will be called. Pass false
        if they are invalid (e.g. the gl-context has changed), in which case
        they will just be abandoned.
    */
    void deleteAllStrikes(bool texturesAreValid);

    class Strike {
    public:
        int width() const { return fStrikeWidth; }
        int height() const { return fStrikeHeight; }
        GLuint texture() const { return fTexName; }
        int widthShift() const { return fStrikeWidthShift; }
        int heightShift() const { return fStrikeHeightShift; }

        // call this to force us to ignore the texture name in our destructor
        // only call it right before our destructor
        void abandonTexture() { fTexName = 0; }

    private:
        // if next is non-null, its height must match our height
        Strike(Strike* next, int width, int height);
        ~Strike();

        Strike* findGlyph(const SkGlyph&, int* offset);
        Strike* addGlyphAndBind(const SkGlyph&, const uint8_t*, int* offset);

        enum {
            kMinStrikeWidth = 1024,
            kMaxGlyphCount = 256
        };

        Strike*     fNext;
        GLuint      fTexName;
        uint32_t    fGlyphIDArray[kMaxGlyphCount];  // stores glyphIDs
        uint16_t    fGlyphOffsetX[kMaxGlyphCount];  // stores x-offsets
        uint16_t    fGlyphCount;
        uint16_t    fNextFreeOffsetX;
        uint16_t    fStrikeWidth;
        uint16_t    fStrikeHeight;
        uint8_t     fStrikeWidthShift;      // pow2(fStrikeWidth)
        uint8_t     fStrikeHeightShift;     // pow2(fStrikeHeight)

        friend class SkGLTextCache;
    };

    /** If found, returns the exact strike containing it (there may be more than
        one with a given height), and sets offset to the offset for that glyph
        (if not null). Does NOT bind the texture.
        If not found, returns null and ignores offset param.
    */
    Strike* findGlyph(const SkGlyph&, int* offset);

    /** Adds the specified glyph to this list of strikes, returning the new
        head of the list. If offset is not null, it is set to the offset
        for this glyph within the strike. The associated texture is bound
        to the gl context.
     */
    Strike* addGlyphAndBind(const SkGlyph&, const uint8_t image[], int* offset);

private:
    enum {
        // greater than this we won't cache
        kMaxGlyphHeightShift = 9,
        
        kMaxGlyphHeight = 1 << kMaxGlyphHeightShift,
        kMaxStrikeListCount = kMaxGlyphHeightShift + 1
    };

    // heads of the N families, one for each pow2 height
    Strike* fStrikeList[kMaxStrikeListCount];
};

#endif
