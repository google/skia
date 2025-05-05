/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkScalerContext_DEFINED
#define SkScalerContext_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkFourByteTag.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkMacros.h"
#include "include/private/base/SkPoint_impl.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkMask.h"
#include "src/core/SkMaskGamma.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>

class SkArenaAlloc;
class SkAutoDescriptor;
class SkDescriptor;
class SkDrawable;
class SkFont;
class SkMaskFilter;
class SkPathEffect;
enum class SkFontHinting;
struct SkFontMetrics;

//The following typedef hides from the rest of the implementation the number of
//most significant bits to consider when creating mask gamma tables. Two bits
//per channel was chosen as a balance between fidelity (more bits) and cache
//sizes (fewer bits). Three bits per channel was chosen when #303942; (used by
//the Chrome UI) turned out too green.
typedef SkTMaskGamma<3, 3, 3> SkMaskGamma;

enum class SkScalerContextFlags : uint32_t {
    kNone                      = 0,
    kFakeGamma                 = 1 << 0,
    kBoostContrast             = 1 << 1,
    kFakeGammaAndBoostContrast = kFakeGamma | kBoostContrast,
};
SK_MAKE_BITFIELD_OPS(SkScalerContextFlags)

/*
 *  To allow this to be forward-declared, it must be its own typename, rather
 *  than a nested struct inside SkScalerContext (where it started).
 *
 *  SkScalerContextRec must be dense, and all bytes must be set to a know quantity because this
 *  structure is used to calculate a checksum.
 */
SK_BEGIN_REQUIRE_DENSE
struct SkScalerContextRec {
    SkTypefaceID fTypefaceID;
    SkScalar     fTextSize, fPreScaleX, fPreSkewX;
    SkScalar     fPost2x2[2][2];
    SkScalar     fFrameWidth, fMiterLimit;

    // This will be set if to the paint's foreground color if
    // kNeedsForegroundColor is set, which will usually be the case for COLRv0 and
    // COLRv1 fonts.
    uint32_t fForegroundColor{SK_ColorBLACK};

private:
    //These describe the parameters to create (uniquely identify) the pre-blend.
    uint32_t      fLumBits;
    uint8_t       fDeviceGamma; //2.6, (0.0, 4.0) gamma, 0.0 for sRGB
    const uint8_t fReservedAlign2{0};
    uint8_t       fContrast;    //0.8+1, [0.0, 1.0] artificial contrast
    const uint8_t fReservedAlign{0};

    static constexpr SkScalar ExternalGammaFromInternal(uint8_t g) {
        return SkIntToScalar(g) / (1 << 6);
    }
    static constexpr uint8_t InternalGammaFromExternal(SkScalar g) {
        // C++23 use constexpr std::floor
        return static_cast<uint8_t>(g * (1 << 6));
    }
    static constexpr SkScalar ExternalContrastFromInternal(uint8_t c) {
        return SkIntToScalar(c) / ((1 << 8) - 1);
    }
    static constexpr uint8_t InternalContrastFromExternal(SkScalar c) {
        // C++23 use constexpr std::round
        return static_cast<uint8_t>((c * ((1 << 8) - 1)) + 0.5f);
    }
public:
    void setDeviceGamma(SkScalar g) {
        sk_ignore_unused_variable(fReservedAlign2);
        SkASSERT(SkSurfaceProps::kMinGammaInclusive <= g &&
                 g < SkIntToScalar(SkSurfaceProps::kMaxGammaExclusive));
        fDeviceGamma = InternalGammaFromExternal(g);
    }

    void setContrast(SkScalar c) {
        sk_ignore_unused_variable(fReservedAlign);
        SkASSERT(SkSurfaceProps::kMinContrastInclusive <= c &&
                 c <= SkIntToScalar(SkSurfaceProps::kMaxContrastInclusive));
        fContrast = InternalContrastFromExternal(c);
    }

    static const SkMaskGamma& CachedMaskGamma(uint8_t contrast, uint8_t gamma);
    const SkMaskGamma& cachedMaskGamma() const {
        return CachedMaskGamma(fContrast, fDeviceGamma);
    }

    /**
     *  Causes the luminance color to be ignored, and the paint and device
     *  gamma to be effectively 1.0
     */
    void ignoreGamma() {
        setLuminanceColor(SK_ColorTRANSPARENT);
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

    /** If the kEmbolden_Flag is set, drop it and use stroking instead. */
    void useStrokeForFakeBold();

    SkMask::Format fMaskFormat;

private:
    uint8_t        fStrokeJoin : 4;
    uint8_t        fStrokeCap  : 4;

public:
    uint16_t    fFlags;

    // Warning: when adding members note that the size of this structure
    // must be a multiple of 4. SkDescriptor requires that its arguments be
    // multiples of four and this structure is put in an SkDescriptor in
    // SkPaint::MakeRecAndEffects.

    SkString dump() const {
        SkString msg;
        msg.appendf("    Rec\n");
        msg.appendf("      textsize %a prescale %a preskew %a post [%a %a %a %a]\n",
                   fTextSize, fPreScaleX, fPreSkewX, fPost2x2[0][0],
                   fPost2x2[0][1], fPost2x2[1][0], fPost2x2[1][1]);
        msg.appendf("      frame %g miter %g format %d join %d cap %d flags %#hx\n",
                   fFrameWidth, fMiterLimit, fMaskFormat, fStrokeJoin, fStrokeCap, fFlags);
        msg.appendf("      lum bits %x, device gamma %d, contrast %d\n", fLumBits,
                    fDeviceGamma, fContrast);
        msg.appendf("      foreground color %x\n", fForegroundColor);
        return msg;
    }

    void    getMatrixFrom2x2(SkMatrix*) const;
    void    getLocalMatrix(SkMatrix*) const;
    void    getSingleMatrix(SkMatrix*) const;

    /** The kind of scale which will be applied by the underlying port (pre-matrix). */
    enum class PreMatrixScale {
        kFull,  // The underlying port can apply both x and y scale.
        kVertical,  // The underlying port can only apply a y scale.
        kVerticalInteger  // The underlying port can only apply an integer y scale.
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
                         SkMatrix* total = nullptr) const;

    SkAxisAlignment computeAxisAlignmentForHText() const;

    inline SkFontHinting getHinting() const;
    inline void setHinting(SkFontHinting);

    SkMask::Format getFormat() const {
        return fMaskFormat;
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

        kNeedsForegroundColor_Flag = 0x4000,
    };

    // computed values
    enum {
        kHinting_Mask   = kHintingBit1_Flag | kHintingBit2_Flag,
    };

    SkScalerContext(SkTypeface&, const SkScalerContextEffects&, const SkDescriptor*);
    virtual ~SkScalerContext();

    SkTypeface* getTypeface() const { return &fTypeface; }

    SkMask::Format getMaskFormat() const {
        return fRec.fMaskFormat;
    }

    bool isSubpixel() const {
        return SkToBool(fRec.fFlags & kSubpixelPositioning_Flag);
    }

    bool isLinearMetrics() const {
        return SkToBool(fRec.fFlags & kLinearMetrics_Flag);
    }

    // DEPRECATED
    bool isVertical() const { return false; }

    SkGlyph     makeGlyph(SkPackedGlyphID, SkArenaAlloc*);
    void        getImage(const SkGlyph&);
    void        getPath(SkGlyph&, SkArenaAlloc*);
    sk_sp<SkDrawable> getDrawable(SkGlyph&);
    void        getFontMetrics(SkFontMetrics*);

    /** Return the size in bytes of the associated gamma lookup table
     */
    static size_t GetGammaLUTSize(SkScalar contrast, SkScalar deviceGamma,
                                  int* width, int* height);

    /** Get the associated gamma lookup table. The 'data' pointer must point to pre-allocated
     *  memory, with size in bytes greater than or equal to the return value of getGammaLUTSize().
     *
     *  If the lookup table hasn't been initialized (e.g., it's linear), this will return false.
     */
    static bool GetGammaLUTData(SkScalar contrast, SkScalar deviceGamma, uint8_t* data);

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
                font, paint, SkSurfaceProps(),
                SkScalerContextFlags::kNone, SkMatrix::I(), rec, effects);
    }

    static std::unique_ptr<SkScalerContext> MakeEmpty(
            SkTypeface& typeface, const SkScalerContextEffects& effects,
            const SkDescriptor* desc);

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
    *  As an example, the identity matrix will return SkAxisAlignment::kX.
    */
    SkAxisAlignment computeAxisAlignmentForHText() const;

    static SkDescriptor* CreateDescriptorAndEffectsUsingPaint(
        const SkFont&, const SkPaint&, const SkSurfaceProps&,
        SkScalerContextFlags scalerContextFlags,
        const SkMatrix& deviceMatrix, SkAutoDescriptor* ad,
        SkScalerContextEffects* effects);

protected:
    const SkScalerContextRec fRec;

    struct GeneratedPath {
        SkPath path;
        bool modified;
    };
    struct GlyphMetrics {
        SkVector       advance;
        SkRect         bounds;
        SkMask::Format maskFormat;
        uint16_t       extraBits;
        bool           neverRequestPath;
        bool           computeFromPath;
        std::optional<GeneratedPath> generatedPath;
        GlyphMetrics(SkMask::Format format)
            : advance{0, 0}
            , bounds{0, 0, 0, 0}
            , maskFormat(format)
            , extraBits(0)
            , neverRequestPath(false)
            , computeFromPath(false)
            , generatedPath{std::nullopt}
        {}
    };

    virtual GlyphMetrics generateMetrics(const SkGlyph&, SkArenaAlloc*) = 0;

    static void GenerateMetricsFromPath(
        SkGlyph* glyph, const SkPath& path, SkMask::Format format,
        bool verticalLCD, bool a8FromLCD, bool hairline);

    static void SaturateGlyphBounds(SkGlyph* glyph, SkRect&&);
    static void SaturateGlyphBounds(SkGlyph* glyph, SkIRect const &);

    /** Generates the contents of glyph.fImage.
     *  When called, glyph.fImage will be pointing to a pre-allocated,
     *  uninitialized region of memory of size glyph.imageSize().
     *  This method may not change glyph.fMaskFormat.
     *
     *  Because glyph.imageSize() will determine the size of fImage,
     *  generateMetrics will be called before generateImage.
     */
    virtual void generateImage(const SkGlyph& glyph, void* imageBuffer) = 0;
    static void GenerateImageFromPath(
        SkMaskBuilder& dst, const SkPath& path, const SkMaskGamma::PreBlend& maskPreBlend,
        bool doBGR, bool verticalLCD, bool a8FromLCD, bool hairline);
    void generateImageFromPath(const SkGlyph& glyph, void* imageBuffer);

    /** Sets the passed path to the glyph outline.
     *  If this cannot be done the path is set to empty;
     *  Does not apply subpixel positioning to the path.
     *  @return false if this glyph does not have any path.
     */
    [[nodiscard]] virtual bool generatePath(const SkGlyph&, SkPath*, bool* modified) = 0;

    /** Returns the drawable for the glyph (if any).
     *
     *  The generated drawable will be lifetime scoped to the lifetime of this scaler context.
     *  This means the drawable may refer to the scaler context and associated font data.
     *
     *  The drawable does not need to be flattenable (e.g. implement getFactory and getTypeName).
     *  Any necessary serialization will be done with makePictureSnapshot.
     */
    virtual sk_sp<SkDrawable> generateDrawable(const SkGlyph&); // TODO: = 0

    /** Retrieves font metrics. */
    virtual void generateFontMetrics(SkFontMetrics*) = 0;

private:
    friend class PathText;  // For debug purposes
    friend class PathTextBench;  // For debug purposes
    friend class RandomScalerContext;  // For debug purposes
    friend class SkScalerContext_proxy;

    static SkScalerContextRec PreprocessRec(const SkTypeface&,
                                            const SkScalerContextEffects&,
                                            const SkDescriptor&);

    // In order for a SkScalerContext to be in use this typeface must exist.
    // The SkScalerContext does not keep a reference to this typeface, so this reference may be
    // a dangling reference when the SkScalerContext is destroyed.
    SkTypeface& fTypeface;

    // optional objects, which may be null
    sk_sp<SkPathEffect> fPathEffect;
    sk_sp<SkMaskFilter> fMaskFilter;

    // if this is set, we draw the image from a path, rather than
    // calling generateImage.
    const bool fGenerateImageFromPath;

    void internalGetPath(SkGlyph&, SkArenaAlloc*, std::optional<GeneratedPath>&&);
    SkGlyph internalMakeGlyph(SkPackedGlyphID, SkMask::Format, SkArenaAlloc*);

protected:
    // SkMaskGamma::PreBlend converts linear masks to gamma correcting masks.
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
