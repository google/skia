/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkScalerContext_DEFINED
#define SkScalerContext_DEFINED

#include "SkMask.h"
#include "SkMaskGamma.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkTypeface.h"

#ifdef SK_BUILD_FOR_ANDROID
    #include "SkPaintOptionsAndroid.h"
#endif

struct SkGlyph;
class SkDescriptor;
class SkMaskFilter;
class SkPathEffect;
class SkRasterizer;

/*
 *  To allow this to be forward-declared, it must be its own typename, rather
 *  than a nested struct inside SkScalerContext (where it started).
 */
struct SkScalerContextRec {
    uint32_t    fOrigFontID;
    uint32_t    fFontID;
    SkScalar    fTextSize, fPreScaleX, fPreSkewX;
    SkScalar    fPost2x2[2][2];
    SkScalar    fFrameWidth, fMiterLimit;

    //These describe the parameters to create (uniquely identify) the pre-blend.
    uint32_t    fLumBits;
    uint8_t     fDeviceGamma; //2.6, (0.0, 4.0) gamma, 0.0 for sRGB
    uint8_t     fPaintGamma;  //2.6, (0.0, 4.0) gamma, 0.0 for sRGB
    uint8_t     fContrast;    //0.8+1, [0.0, 1.0] artificial contrast
    uint8_t     fReservedAlign;

    SkScalar getDeviceGamma() const {
        return SkIntToScalar(fDeviceGamma) / (1 << 6);
    }
    void setDeviceGamma(SkScalar dg) {
        SkASSERT(0 <= dg && dg < SkIntToScalar(4));
        fDeviceGamma = SkScalarFloorToInt(dg * (1 << 6));
    }

    SkScalar getPaintGamma() const {
        return SkIntToScalar(fPaintGamma) / (1 << 6);
    }
    void setPaintGamma(SkScalar pg) {
        SkASSERT(0 <= pg && pg < SkIntToScalar(4));
        fPaintGamma = SkScalarFloorToInt(pg * (1 << 6));
    }

    SkScalar getContrast() const {
        return SkIntToScalar(fContrast) / ((1 << 8) - 1);
    }
    void setContrast(SkScalar c) {
        SkASSERT(0 <= c && c <= SK_Scalar1);
        fContrast = SkScalarRoundToInt(c * ((1 << 8) - 1));
    }

    /**
     *  Causes the luminance color and contrast to be ignored, and the
     *  paint and device gamma to be effectively 1.0.
     */
    void ignorePreBlend() {
        setLuminanceColor(SK_ColorTRANSPARENT);
        setPaintGamma(SK_Scalar1);
        setDeviceGamma(SK_Scalar1);
        setContrast(0);
    }

    uint8_t     fMaskFormat;
    uint8_t     fStrokeJoin;
    uint16_t    fFlags;
    // Warning: when adding members note that the size of this structure
    // must be a multiple of 4. SkDescriptor requires that its arguments be
    // multiples of four and this structure is put in an SkDescriptor in
    // SkPaint::MakeRec.

    void    getMatrixFrom2x2(SkMatrix*) const;
    void    getLocalMatrix(SkMatrix*) const;
    void    getSingleMatrix(SkMatrix*) const;

    inline SkPaint::Hinting getHinting() const;
    inline void setHinting(SkPaint::Hinting);

    SkMask::Format getFormat() const {
        return static_cast<SkMask::Format>(fMaskFormat);
    }

    SkColor getLuminanceColor() const {
        return fLumBits;
    }

    void setLuminanceColor(SkColor c) {
        fLumBits = c;
    }
};

//The following typedef hides from the rest of the implementation the number of
//most significant bits to consider when creating mask gamma tables. Two bits
//per channel was chosen as a balance between fidelity (more bits) and cache
//sizes (fewer bits). Three bits per channel was chosen when #303942; (used by
//the Chrome UI) turned out too green.
typedef SkTMaskGamma<3, 3, 3> SkMaskGamma;

class SkScalerContext {
public:
    typedef SkScalerContextRec Rec;

    enum Flags {
        kFrameAndFill_Flag        = 0x0001,
        kDevKernText_Flag         = 0x0002,
        kEmbeddedBitmapText_Flag  = 0x0004,
        kEmbolden_Flag            = 0x0008,
        kSubpixelPositioning_Flag = 0x0010,
        kForceAutohinting_Flag    = 0x0020,  // Use auto instead of bytcode hinting if hinting.
        kVertical_Flag            = 0x0040,

        // together, these two flags resulting in a two bit value which matches
        // up with the SkPaint::Hinting enum.
        kHinting_Shift            = 7, // to shift into the other flags above
        kHintingBit1_Flag         = 0x0080,
        kHintingBit2_Flag         = 0x0100,

        // Pixel geometry information.
        // only meaningful if fMaskFormat is LCD16 or LCD32
        kLCD_Vertical_Flag        = 0x0200,    // else Horizontal
        kLCD_BGROrder_Flag        = 0x0400,    // else RGB order

        // Generate A8 from LCD source (for GDI and CoreGraphics).
        // only meaningful if fMaskFormat is kA8
        kGenA8FromLCD_Flag        = 0x0800, // could be 0x200 (bit meaning dependent on fMaskFormat)
    };

    // computed values
    enum {
        kHinting_Mask   = kHintingBit1_Flag | kHintingBit2_Flag,
    };


    SkScalerContext(SkTypeface*, const SkDescriptor*);
    virtual ~SkScalerContext();

    SkTypeface* getTypeface() const { return fTypeface.get(); }

    SkMask::Format getMaskFormat() const {
        return (SkMask::Format)fRec.fMaskFormat;
    }

    bool isSubpixel() const {
        return SkToBool(fRec.fFlags & kSubpixelPositioning_Flag);
    }

    bool isVertical() const {
        return SkToBool(fRec.fFlags & kVertical_Flag);
    }

    // remember our glyph offset/base
    void setBaseGlyphCount(unsigned baseGlyphCount) {
        fBaseGlyphCount = baseGlyphCount;
    }

    /** Return the corresponding glyph for the specified unichar. Since contexts
        may be chained (under the hood), the glyphID that is returned may in
        fact correspond to a different font/context. In that case, we use the
        base-glyph-count to know how to translate back into local glyph space.
     */
    uint16_t charToGlyphID(SkUnichar uni);

    /** Map the glyphID to its glyph index, and then to its char code. Unmapped
        glyphs return zero.
    */
    SkUnichar glyphIDToChar(uint16_t glyphID);

    unsigned    getGlyphCount() { return this->generateGlyphCount(); }
    void        getAdvance(SkGlyph*);
    void        getMetrics(SkGlyph*);
    void        getImage(const SkGlyph&);
    void        getPath(const SkGlyph&, SkPath*);
    void        getFontMetrics(SkPaint::FontMetrics*);

    /** Return the size in bytes of the associated gamma lookup table
     */
    static size_t GetGammaLUTSize(SkScalar contrast, SkScalar paintGamma, SkScalar deviceGamma,
                                  int* width, int* height);

    /** Get the associated gamma lookup table. The 'data' pointer must point to pre-allocated
        memory, with size in bytes greater than or equal to the return value of getGammaLUTSize().
     */
    static void   GetGammaLUTData(SkScalar contrast, SkScalar paintGamma, SkScalar deviceGamma,
                                  void* data);

#ifdef SK_BUILD_FOR_ANDROID
    unsigned getBaseGlyphCount(SkUnichar charCode);

    // This function must be public for SkTypeface_android.h, but should not be
    // called by other callers
    SkFontID findTypefaceIdForChar(SkUnichar uni);
#endif

    static void MakeRec(const SkPaint&, const SkDeviceProperties* deviceProperties,
                        const SkMatrix*, Rec* rec);
    static inline void PostMakeRec(const SkPaint&, Rec*);

    static SkMaskGamma::PreBlend GetMaskPreBlend(const Rec& rec);

protected:
    Rec         fRec;
    unsigned    fBaseGlyphCount;

    /** Generates the contents of glyph.fAdvanceX and glyph.fAdvanceY.
     *  May call getMetrics if that would be just as fast.
     */
    virtual void generateAdvance(SkGlyph* glyph) = 0;

    /** Generates the contents of glyph.fWidth, fHeight, fTop, fLeft,
     *  as well as fAdvanceX and fAdvanceY if not already set.
     *
     *  TODO: fMaskFormat is set by getMetrics later; cannot be set here.
     */
    virtual void generateMetrics(SkGlyph* glyph) = 0;

    /** Generates the contents of glyph.fImage.
     *  When called, glyph.fImage will be pointing to a pre-allocated,
     *  uninitialized region of memory of size glyph.computeImageSize().
     *  This method may change glyph.fMaskFormat if the new image size is
     *  less than or equal to the old image size.
     *
     *  Because glyph.computeImageSize() will determine the size of fImage,
     *  generateMetrics will be called before generateImage.
     */
    virtual void generateImage(const SkGlyph& glyph) = 0;

    /** Sets the passed path to the glyph outline.
     *  If this cannot be done the path is set to empty;
     *  this is indistinguishable from a glyph with an empty path.
     *  This does not set glyph.fPath.
     *
     *  TODO: path is always glyph.fPath, no reason to pass separately.
     */
    virtual void generatePath(const SkGlyph& glyph, SkPath* path) = 0;

    /** Retrieves font metrics. */
    virtual void generateFontMetrics(SkPaint::FontMetrics*) = 0;

    /** Returns the number of glyphs in the font. */
    virtual unsigned generateGlyphCount() = 0;

    /** Returns the glyph id for the given unichar.
     *  If there is no 1:1 mapping from the unichar to a glyph id, returns 0.
     */
    virtual uint16_t generateCharToGlyph(SkUnichar unichar) = 0;

    /** Returns the unichar for the given glyph id.
     *  If there is no 1:1 mapping from the glyph id to a unichar, returns 0.
     *  The default implementation always returns 0, indicating failure.
     */
    virtual SkUnichar generateGlyphToChar(uint16_t glyphId);

    void forceGenerateImageFromPath() { fGenerateImageFromPath = true; }

private:
    // never null
    SkAutoTUnref<SkTypeface> fTypeface;

#ifdef SK_BUILD_FOR_ANDROID
    SkPaintOptionsAndroid fPaintOptionsAndroid;
#endif

    // optional object, which may be null
    SkPathEffect*   fPathEffect;
    SkMaskFilter*   fMaskFilter;
    SkRasterizer*   fRasterizer;

    // if this is set, we draw the image from a path, rather than
    // calling generateImage.
    bool fGenerateImageFromPath;

    void internalGetPath(const SkGlyph& glyph, SkPath* fillPath,
                         SkPath* devPath, SkMatrix* fillToDevMatrix);

    // Return the context associated with the next logical typeface, or NULL if
    // there are no more entries in the fallback chain.
    SkScalerContext* allocNextContext() const;

    // return the next context, treating fNextContext as a cache of the answer
    SkScalerContext* getNextContext();

    // returns the right context from our link-list for this glyph. If no match
    // is found, just returns the original context (this)
    SkScalerContext* getGlyphContext(const SkGlyph& glyph);

    // returns the right context from our link-list for this char. If no match
    // is found it returns NULL. If a match is found then the glyphID param is
    // set to the glyphID that maps to the provided char.
    SkScalerContext* getContextFromChar(SkUnichar uni, uint16_t* glyphID);

    // link-list of context, to handle missing chars. null-terminated.
    SkScalerContext* fNextContext;

    // SkMaskGamma::PreBlend converts linear masks to gamma correcting masks.
protected:
    // Visible to subclasses so that generateImage can apply the pre-blend directly.
    const SkMaskGamma::PreBlend fPreBlend;
private:
    // When there is a filter, previous steps must create a linear mask
    // and the pre-blend applied as a final step.
    const SkMaskGamma::PreBlend fPreBlendForFilter;
};

#define kRec_SkDescriptorTag            SkSetFourByteTag('s', 'r', 'e', 'c')
#define kPathEffect_SkDescriptorTag     SkSetFourByteTag('p', 't', 'h', 'e')
#define kMaskFilter_SkDescriptorTag     SkSetFourByteTag('m', 's', 'k', 'f')
#define kRasterizer_SkDescriptorTag     SkSetFourByteTag('r', 'a', 's', 't')
#ifdef SK_BUILD_FOR_ANDROID
#define kAndroidOpts_SkDescriptorTag    SkSetFourByteTag('a', 'n', 'd', 'r')
#endif

///////////////////////////////////////////////////////////////////////////////

enum SkAxisAlignment {
    kNone_SkAxisAlignment,
    kX_SkAxisAlignment,
    kY_SkAxisAlignment
};

/**
 *  Return the axis (if any) that the baseline for horizontal text will land on
 *  after running through the specified matrix.
 *
 *  As an example, the identity matrix will return kX_SkAxisAlignment
 */
SkAxisAlignment SkComputeAxisAlignmentForHText(const SkMatrix& matrix);

///////////////////////////////////////////////////////////////////////////////

SkPaint::Hinting SkScalerContextRec::getHinting() const {
    unsigned hint = (fFlags & SkScalerContext::kHinting_Mask) >>
                                            SkScalerContext::kHinting_Shift;
    return static_cast<SkPaint::Hinting>(hint);
}

void SkScalerContextRec::setHinting(SkPaint::Hinting hinting) {
    fFlags = (fFlags & ~SkScalerContext::kHinting_Mask) |
                                (hinting << SkScalerContext::kHinting_Shift);
}


#endif
