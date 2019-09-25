/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkScalerContext_DEFINED
#define SkScalerContext_DEFINED

#include <memory>

#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkTypeface.h"
#include "include/private/SkMacros.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkMask.h"
#include "src/core/SkMaskGamma.h"
#include "src/core/SkStrikeInterface.h"
#include "src/core/SkSurfacePriv.h"
#include "src/core/SkWriteBuffer.h"

class SkAutoDescriptor;
class SkDescriptor;
class SkMaskFilter;
class SkPathEffect;
class SkScalerContext;
class SkScalerContext_DW;

enum SkScalerContextFlags : uint32_t {
    kNone                      = 0,
    kFakeGamma                 = 1 << 0,
    kBoostContrast             = 1 << 1,
    kFakeGammaAndBoostContrast = kFakeGamma | kBoostContrast,
};

enum SkAxisAlignment : uint32_t {
    kNone_SkAxisAlignment,
    kX_SkAxisAlignment,
    kY_SkAxisAlignment
};

/*
 *  To allow this to be forward-declared, it must be its own typename, rather
 *  than a nested struct inside SkScalerContext (where it started).
 *
 *  SkScalerContextRec must be dense, and all bytes must be set to a know quantity because this
 *  structure is used to calculate a checksum.
 */
SK_BEGIN_REQUIRE_DENSE
struct SkScalerContextRec {
    uint32_t    fFontID;
    SkScalar    fTextSize, fPreScaleX, fPreSkewX;
    SkScalar    fPost2x2[2][2];
    SkScalar    fFrameWidth, fMiterLimit;

private:
    //These describe the parameters to create (uniquely identify) the pre-blend.
    uint32_t      fLumBits;
    uint8_t       fDeviceGamma; //2.6, (0.0, 4.0) gamma, 0.0 for sRGB
    uint8_t       fPaintGamma;  //2.6, (0.0, 4.0) gamma, 0.0 for sRGB
    uint8_t       fContrast;    //0.8+1, [0.0, 1.0] artificial contrast
    const uint8_t fReservedAlign{0};

public:

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
        sk_ignore_unused_variable(fReservedAlign);
        return SkIntToScalar(fContrast) / ((1 << 8) - 1);
    }
    void setContrast(SkScalar c) {
        SkASSERT(0 <= c && c <= SK_Scalar1);
        fContrast = SkScalarRoundToInt(c * ((1 << 8) - 1));
    }

    /**
     *  Causes the luminance color to be ignored, and the paint and device
     *  gamma to be effectively 1.0
     */
    void ignoreGamma() {
        setLuminanceColor(SK_ColorTRANSPARENT);
        setPaintGamma(SK_Scalar1);
        setDeviceGamma(SK_Scalar1);
    }

    /**
     *  Causes the luminance color and contrast to be ignored, and the
     *  paint and device gamma to be effectively 1.0.
     */
    void ignorePreBlend() {
        ignoreGamma();
        setContrast(0);
    }

    uint8_t     fMaskFormat;
private:
    uint8_t     fStrokeJoin : 4;
    uint8_t     fStrokeCap : 4;

public:
    uint16_t    fFlags;

    // Warning: when adding members note that the size of this structure
    // must be a multiple of 4. SkDescriptor requires that its arguments be
    // multiples of four and this structure is put in an SkDescriptor in
    // SkPaint::MakeRecAndEffects.

    SkString dump() const {
        SkString msg;
        msg.appendf("Rec\n");
        msg.appendf("  textsize %g prescale %g preskew %g post [%g %g %g %g]\n",
                   fTextSize, fPreScaleX, fPreSkewX, fPost2x2[0][0],
                   fPost2x2[0][1], fPost2x2[1][0], fPost2x2[1][1]);
        msg.appendf("  frame %g miter %g format %d join %d cap %d flags %#hx\n",
                   fFrameWidth, fMiterLimit, fMaskFormat, fStrokeJoin, fStrokeCap, fFlags);
        msg.appendf("  lum bits %x, device gamma %d, paint gamma %d contrast %d\n", fLumBits,
                    fDeviceGamma, fPaintGamma, fContrast);
        return msg;
    }

    void    getMatrixFrom2x2(SkMatrix*) const;
    void    getLocalMatrix(SkMatrix*) const;
    void    getSingleMatrix(SkMatrix*) const;

    /** The kind of scale which will be applied by the underlying port (pre-matrix). */
    enum PreMatrixScale {
        kFull_PreMatrixScale,  // The underlying port can apply both x and y scale.
        kVertical_PreMatrixScale,  // The underlying port can only apply a y scale.
        kVerticalInteger_PreMatrixScale  // The underlying port can only apply an integer y scale.
    };
    /**
     *  Compute useful matrices for use with sizing in underlying libraries.
     *
     *  There are two kinds of text size, a 'requested/logical size' which is like asking for size
     *  '12' and a 'real' size which is the size after the matrix is applied. The matrices produced
     *  by this method are based on the 'real' size. This method effectively finds the total device
     *  matrix and decomposes it in various ways.
     *
     *  The most useful decomposition is into 'scale' and 'remaining'. The 'scale' is applied first
     *  and then the 'remaining' to fully apply the total matrix. This decomposition is useful when
     *  the text size ('scale') may have meaning apart from the total matrix. This is true when
     *  hinting, and sometimes true for other properties as well.
     *
     *  The second (optional) decomposition is of 'remaining' into a non-rotational part
     *  'remainingWithoutRotation' and a rotational part 'remainingRotation'. The 'scale' is applied
     *  first, then 'remainingWithoutRotation', then 'remainingRotation' to fully apply the total
     *  matrix. This decomposition is helpful when only horizontal metrics can be trusted, so the
     *  'scale' and 'remainingWithoutRotation' will be handled by the underlying library, but
     *  the final rotation 'remainingRotation' will be handled manually.
     *
     *  The 'total' matrix is also (optionally) available. This is useful in cases where the
     *  underlying library will not be used, often when working directly with font data.
     *
     *  The parameters 'scale' and 'remaining' are required, the other pointers may be nullptr.
     *
     *  @param preMatrixScale the kind of scale to extract from the total matrix.
     *  @param scale the scale extracted from the total matrix (both values positive).
     *  @param remaining apply after scale to apply the total matrix.
     *  @param remainingWithoutRotation apply after scale to apply the total matrix sans rotation.
     *  @param remainingRotation apply after remainingWithoutRotation to apply the total matrix.
     *  @param total the total matrix.
     *  @return false if the matrix was singular. The output will be valid but not invertible.
     */
    bool computeMatrices(PreMatrixScale preMatrixScale,
                         SkVector* scale, SkMatrix* remaining,
                         SkMatrix* remainingWithoutRotation = nullptr,
                         SkMatrix* remainingRotation = nullptr,
                         SkMatrix* total = nullptr);

    SkAxisAlignment computeAxisAlignmentForHText() const;

    inline SkFontHinting getHinting() const;
    inline void setHinting(SkFontHinting);

    SkMask::Format getFormat() const {
        return static_cast<SkMask::Format>(fMaskFormat);
    }

    SkColor getLuminanceColor() const {
        return fLumBits;
    }

    // setLuminanceColor forces the alpha to be 0xFF because the blitter that draws the glyph
    // will apply the alpha from the paint. Don't apply the alpha twice.
    void setLuminanceColor(SkColor c);

private:
    // TODO: remove
    friend class SkScalerContext;
};
SK_END_REQUIRE_DENSE

// TODO: rename SkScalerContextEffects -> SkStrikeEffects
struct SkScalerContextEffects {
    SkScalerContextEffects() : fPathEffect(nullptr), fMaskFilter(nullptr) {}
    SkScalerContextEffects(SkPathEffect* pe, SkMaskFilter* mf)
            : fPathEffect(pe), fMaskFilter(mf) {}
    explicit SkScalerContextEffects(const SkPaint& paint)
            : fPathEffect(paint.getPathEffect())
            , fMaskFilter(paint.getMaskFilter()) {}

    SkPathEffect*   fPathEffect;
    SkMaskFilter*   fMaskFilter;
};

//The following typedef hides from the rest of the implementation the number of
//most significant bits to consider when creating mask gamma tables. Two bits
//per channel was chosen as a balance between fidelity (more bits) and cache
//sizes (fewer bits). Three bits per channel was chosen when #303942; (used by
//the Chrome UI) turned out too green.
typedef SkTMaskGamma<3, 3, 3> SkMaskGamma;

class SkScalerContext {
public:
    enum Flags {
        kFrameAndFill_Flag        = 0x0001,
        kUnused                   = 0x0002,
        kEmbeddedBitmapText_Flag  = 0x0004,
        kEmbolden_Flag            = 0x0008,
        kSubpixelPositioning_Flag = 0x0010,
        kForceAutohinting_Flag    = 0x0020,  // Use auto instead of bytcode hinting if hinting.

        // together, these two flags resulting in a two bit value which matches
        // up with the SkPaint::Hinting enum.
        kHinting_Shift            = 7, // to shift into the other flags above
        kHintingBit1_Flag         = 0x0080,
        kHintingBit2_Flag         = 0x0100,

        // Pixel geometry information.
        // only meaningful if fMaskFormat is kLCD16
        kLCD_Vertical_Flag        = 0x0200,    // else Horizontal
        kLCD_BGROrder_Flag        = 0x0400,    // else RGB order

        // Generate A8 from LCD source (for GDI and CoreGraphics).
        // only meaningful if fMaskFormat is kA8
        kGenA8FromLCD_Flag        = 0x0800, // could be 0x200 (bit meaning dependent on fMaskFormat)
        kLinearMetrics_Flag       = 0x1000,
        kBaselineSnap_Flag        = 0x2000,
    };

    // computed values
    enum {
        kHinting_Mask   = kHintingBit1_Flag | kHintingBit2_Flag,
    };

    SkScalerContext(sk_sp<SkTypeface>, const SkScalerContextEffects&, const SkDescriptor*);
    virtual ~SkScalerContext();

    SkTypeface* getTypeface() const { return fTypeface.get(); }

    SkMask::Format getMaskFormat() const {
        return (SkMask::Format)fRec.fMaskFormat;
    }

    bool isSubpixel() const {
        return SkToBool(fRec.fFlags & kSubpixelPositioning_Flag);
    }

    bool isLinearMetrics() const {
        return SkToBool(fRec.fFlags & kLinearMetrics_Flag);
    }

    // DEPRECATED
    bool isVertical() const { return false; }

    unsigned    getGlyphCount() { return this->generateGlyphCount(); }
    void        getAdvance(SkGlyph*);
    void        getMetrics(SkGlyph*);
    void        getImage(const SkGlyph&);
    bool SK_WARN_UNUSED_RESULT getPath(SkPackedGlyphID, SkPath*);
    void        getFontMetrics(SkFontMetrics*);

    /** Return the size in bytes of the associated gamma lookup table
     */
    static size_t GetGammaLUTSize(SkScalar contrast, SkScalar paintGamma, SkScalar deviceGamma,
                                  int* width, int* height);

    /** Get the associated gamma lookup table. The 'data' pointer must point to pre-allocated
     *  memory, with size in bytes greater than or equal to the return value of getGammaLUTSize().
     *
     *  If the lookup table hasn't been initialized (e.g., it's linear), this will return false.
     */
    static bool   GetGammaLUTData(SkScalar contrast, SkScalar paintGamma, SkScalar deviceGamma,
                                  uint8_t* data);

    static void MakeRecAndEffects(const SkFont& font, const SkPaint& paint,
                                  const SkSurfaceProps& surfaceProps,
                                  SkScalerContextFlags scalerContextFlags,
                                  const SkMatrix& deviceMatrix,
                                  SkScalerContextRec* rec,
                                  SkScalerContextEffects* effects);

    // If we are creating rec and effects from a font only, then there is no device around either.
    static void MakeRecAndEffectsFromFont(const SkFont& font,
                                          SkScalerContextRec* rec,
                                          SkScalerContextEffects* effects) {
        SkPaint paint;
        return MakeRecAndEffects(
                font, paint, SkSurfaceProps(SkSurfaceProps::kLegacyFontHost_InitType),
                SkScalerContextFlags::kNone, SkMatrix::I(), rec, effects);
    }

    static SkDescriptor*  MakeDescriptorForPaths(SkFontID fontID,
                                                 SkAutoDescriptor* ad);

    static SkDescriptor* AutoDescriptorGivenRecAndEffects(
        const SkScalerContextRec& rec,
        const SkScalerContextEffects& effects,
        SkAutoDescriptor* ad);

    static std::unique_ptr<SkDescriptor> DescriptorGivenRecAndEffects(
        const SkScalerContextRec& rec,
        const SkScalerContextEffects& effects);

    static void DescriptorBufferGiveRec(const SkScalerContextRec& rec, void* buffer);
    static bool CheckBufferSizeForRec(const SkScalerContextRec& rec,
                                      const SkScalerContextEffects& effects,
                                      size_t size);

    static SkMaskGamma::PreBlend GetMaskPreBlend(const SkScalerContextRec& rec);

    const SkScalerContextRec& getRec() const { return fRec; }

    SkScalerContextEffects getEffects() const {
        return { fPathEffect.get(), fMaskFilter.get() };
    }

    /**
    *  Return the axis (if any) that the baseline for horizontal text should land on.
    *  As an example, the identity matrix will return kX_SkAxisAlignment
    */
    SkAxisAlignment computeAxisAlignmentForHText() const;

    static SkDescriptor* CreateDescriptorAndEffectsUsingPaint(
        const SkFont&, const SkPaint&, const SkSurfaceProps&,
        SkScalerContextFlags scalerContextFlags,
        const SkMatrix& deviceMatrix, SkAutoDescriptor* ad,
        SkScalerContextEffects* effects);

protected:
    SkScalerContextRec fRec;

    /** Generates the contents of glyph.fAdvanceX and glyph.fAdvanceY if it can do so quickly.
     *  Returns true if it could, false otherwise.
     */
    virtual bool generateAdvance(SkGlyph* glyph) = 0;

    /** Generates the contents of glyph.fWidth, fHeight, fTop, fLeft,
     *  as well as fAdvanceX and fAdvanceY if not already set.
     *
     *  TODO: fMaskFormat is set by getMetrics later; cannot be set here.
     */
    virtual void generateMetrics(SkGlyph* glyph) = 0;

    /** Generates the contents of glyph.fImage.
     *  When called, glyph.fImage will be pointing to a pre-allocated,
     *  uninitialized region of memory of size glyph.imageSize().
     *  This method may change glyph.fMaskFormat if the new image size is
     *  less than or equal to the old image size.
     *
     *  Because glyph.imageSize() will determine the size of fImage,
     *  generateMetrics will be called before generateImage.
     */
    virtual void generateImage(const SkGlyph& glyph) = 0;

    /** Sets the passed path to the glyph outline.
     *  If this cannot be done the path is set to empty;
     *  @return false if this glyph does not have any path.
     */
    virtual bool SK_WARN_UNUSED_RESULT generatePath(SkGlyphID glyphId, SkPath* path) = 0;

    /** Retrieves font metrics. */
    virtual void generateFontMetrics(SkFontMetrics*) = 0;

    /** Returns the number of glyphs in the font. */
    virtual unsigned generateGlyphCount() = 0;

    void forceGenerateImageFromPath() { fGenerateImageFromPath = true; }
    void forceOffGenerateImageFromPath() { fGenerateImageFromPath = false; }

private:
    friend class RandomScalerContext;  // For debug purposes

    static SkScalerContextRec PreprocessRec(const SkTypeface& typeface,
                                            const SkScalerContextEffects& effects,
                                            const SkDescriptor& desc);

    // never null
    sk_sp<SkTypeface> fTypeface;

    // optional objects, which may be null
    sk_sp<SkPathEffect> fPathEffect;
    sk_sp<SkMaskFilter> fMaskFilter;

    // if this is set, we draw the image from a path, rather than
    // calling generateImage.
    bool fGenerateImageFromPath;

    /** Returns false if the glyph has no path at all. */
    bool internalGetPath(SkPackedGlyphID id, SkPath* devPath);

    // SkMaskGamma::PreBlend converts linear masks to gamma correcting masks.
protected:
    // Visible to subclasses so that generateImage can apply the pre-blend directly.
    const SkMaskGamma::PreBlend fPreBlend;
};

#define kRec_SkDescriptorTag            SkSetFourByteTag('s', 'r', 'e', 'c')
#define kEffects_SkDescriptorTag        SkSetFourByteTag('e', 'f', 'c', 't')

///////////////////////////////////////////////////////////////////////////////

SkFontHinting SkScalerContextRec::getHinting() const {
    unsigned hint = (fFlags & SkScalerContext::kHinting_Mask) >>
                                            SkScalerContext::kHinting_Shift;
    return static_cast<SkFontHinting>(hint);
}

void SkScalerContextRec::setHinting(SkFontHinting hinting) {
    fFlags = (fFlags & ~SkScalerContext::kHinting_Mask) |
                        (static_cast<unsigned>(hinting) << SkScalerContext::kHinting_Shift);
}


#endif
