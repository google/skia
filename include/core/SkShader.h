/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShader_DEFINED
#define SkShader_DEFINED

#include "SkBitmap.h"
#include "SkFilterQuality.h"
#include "SkFlattenable.h"
#include "SkImageInfo.h"
#include "SkMask.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "../gpu/GrColor.h"

class SkArenaAlloc;
class SkColorFilter;
class SkColorSpace;
class SkImage;
class SkPath;
class SkPicture;
class SkRasterPipeline;
class GrContext;
class GrFragmentProcessor;

/** \class SkShader
 *
 *  Shaders specify the source color(s) for what is being drawn. If a paint
 *  has no shader, then the paint's color is used. If the paint has a
 *  shader, then the shader's color(s) are use instead, but they are
 *  modulated by the paint's alpha. This makes it easy to create a shader
 *  once (e.g. bitmap tiling or gradient) and then change its transparency
 *  w/o having to modify the original shader... only the paint's alpha needs
 *  to be modified.
 */
class SK_API SkShader : public SkFlattenable {
public:
    SkShader(const SkMatrix* localMatrix = NULL);
    ~SkShader() override;

    /**
     *  Returns the local matrix.
     *
     *  FIXME: This can be incorrect for a Shader with its own local matrix
     *  that is also wrapped via CreateLocalMatrixShader.
     */
    const SkMatrix& getLocalMatrix() const { return fLocalMatrix; }

    enum TileMode {
        /** replicate the edge color if the shader draws outside of its
         *  original bounds
         */
        kClamp_TileMode,

        /** repeat the shader's image horizontally and vertically */
        kRepeat_TileMode,

        /** repeat the shader's image horizontally and vertically, alternating
         *  mirror images so that adjacent images always seam
         */
        kMirror_TileMode,

#if 0
        /** only draw within the original domain, return 0 everywhere else */
        kDecal_TileMode,
#endif
    };

    enum {
        kTileModeCount = kMirror_TileMode + 1
    };

    // override these in your subclass

    enum Flags {
        //!< set if all of the colors will be opaque
        kOpaqueAlpha_Flag = 1 << 0,

        /** set if the spans only vary in X (const in Y).
            e.g. an Nx1 bitmap that is being tiled in Y, or a linear-gradient
            that varies from left-to-right. This flag specifies this for
            shadeSpan().
         */
        kConstInY32_Flag = 1 << 1,

        /** hint for the blitter that 4f is the preferred shading mode.
         */
        kPrefers4f_Flag  = 1 << 2,
    };

    /**
     *  Returns true if the shader is guaranteed to produce only opaque
     *  colors, subject to the SkPaint using the shader to apply an opaque
     *  alpha value. Subclasses should override this to allow some
     *  optimizations.
     */
    virtual bool isOpaque() const { return false; }

    /**
     *  Returns true if the shader is guaranteed to produce only a single color.
     *  Subclasses can override this to allow loop-hoisting optimization.
     */
    virtual bool isConstant() const { return false; }

    /**
     *  ContextRec acts as a parameter bundle for creating Contexts.
     */
    struct ContextRec {
        enum DstType {
            kPMColor_DstType, // clients prefer shading into PMColor dest
            kPM4f_DstType,    // clients prefer shading into PM4f dest
        };

        ContextRec(const SkPaint& paint, const SkMatrix& matrix, const SkMatrix* localM,
                   DstType dstType, SkColorSpace* dstColorSpace)
            : fPaint(&paint)
            , fMatrix(&matrix)
            , fLocalMatrix(localM)
            , fPreferredDstType(dstType)
            , fDstColorSpace(dstColorSpace) {}

        const SkPaint*  fPaint;            // the current paint associated with the draw
        const SkMatrix* fMatrix;           // the current matrix in the canvas
        const SkMatrix* fLocalMatrix;      // optional local matrix
        const DstType   fPreferredDstType; // the "natural" client dest type
        SkColorSpace*   fDstColorSpace;    // the color space of the dest surface (if any)
    };

    class Context : public ::SkNoncopyable {
    public:
        Context(const SkShader& shader, const ContextRec&);

        virtual ~Context();

        /**
         *  Called sometimes before drawing with this shader. Return the type of
         *  alpha your shader will return. The default implementation returns 0.
         *  Your subclass should override if it can (even sometimes) report a
         *  non-zero value, since that will enable various blitters to perform
         *  faster.
         */
        virtual uint32_t getFlags() const { return 0; }

        /**
         *  Called for each span of the object being drawn. Your subclass should
         *  set the appropriate colors (with premultiplied alpha) that correspond
         *  to the specified device coordinates.
         */
        virtual void shadeSpan(int x, int y, SkPMColor[], int count) = 0;

        virtual void shadeSpan4f(int x, int y, SkPM4f[], int count);

        struct BlitState;
        typedef void (*BlitBW)(BlitState*,
                               int x, int y, const SkPixmap&, int count);
        typedef void (*BlitAA)(BlitState*,
                               int x, int y, const SkPixmap&, int count, const SkAlpha[]);

        struct BlitState {
            // inputs
            Context*    fCtx;
            SkBlendMode fMode;

            // outputs
            enum { N = 2 };
            void*       fStorage[N];
            BlitBW      fBlitBW;
            BlitAA      fBlitAA;
        };

        // Returns true if one or more of the blitprocs are set in the BlitState
        bool chooseBlitProcs(const SkImageInfo& info, BlitState* state) {
            state->fBlitBW = nullptr;
            state->fBlitAA = nullptr;
            if (this->onChooseBlitProcs(info, state)) {
                SkASSERT(state->fBlitBW || state->fBlitAA);
                return true;
            }
            return false;
        }

        /**
         * The const void* ctx is only const because all the implementations are const.
         * This can be changed to non-const if a new shade proc needs to change the ctx.
         */
        typedef void (*ShadeProc)(const void* ctx, int x, int y, SkPMColor[], int count);
        virtual ShadeProc asAShadeProc(void** ctx);

        /**
         *  Similar to shadeSpan, but only returns the alpha-channel for a span.
         *  The default implementation calls shadeSpan() and then extracts the alpha
         *  values from the returned colors.
         */
        virtual void shadeSpanAlpha(int x, int y, uint8_t alpha[], int count);

        // Notification from blitter::blitMask in case we need to see the non-alpha channels
        virtual void set3DMask(const SkMask*) {}

    protected:
        // Reference to shader, so we don't have to dupe information.
        const SkShader& fShader;

        enum MatrixClass {
            kLinear_MatrixClass,            // no perspective
            kFixedStepInX_MatrixClass,      // fast perspective, need to call fixedStepInX() each
                                            // scanline
            kPerspective_MatrixClass        // slow perspective, need to mappoints each pixel
        };
        static MatrixClass ComputeMatrixClass(const SkMatrix&);

        uint8_t         getPaintAlpha() const { return fPaintAlpha; }
        const SkMatrix& getTotalInverse() const { return fTotalInverse; }
        MatrixClass     getInverseClass() const { return (MatrixClass)fTotalInverseClass; }
        const SkMatrix& getCTM() const { return fCTM; }

        virtual bool onChooseBlitProcs(const SkImageInfo&, BlitState*) { return false; }

    private:
        SkMatrix    fCTM;
        SkMatrix    fTotalInverse;
        uint8_t     fPaintAlpha;
        uint8_t     fTotalInverseClass;

        typedef SkNoncopyable INHERITED;
    };

    /**
     * Make a context using the memory provided by the arena.
     *
     * @return pointer to context or nullptr if can't be created
     */
    Context* makeContext(const ContextRec&, SkArenaAlloc*) const;

#ifdef SK_SUPPORT_LEGACY_SHADER_ISABITMAP
    /**
     *  Returns true if this shader is just a bitmap, and if not null, returns the bitmap,
     *  localMatrix, and tilemodes. If this is not a bitmap, returns false and ignores the
     *  out-parameters.
     */
    bool isABitmap(SkBitmap* outTexture, SkMatrix* outMatrix, TileMode xy[2]) const {
        return this->onIsABitmap(outTexture, outMatrix, xy);
    }

    bool isABitmap() const {
        return this->isABitmap(nullptr, nullptr, nullptr);
    }
#endif

    /**
     *  Iff this shader is backed by a single SkImage, return its ptr (the caller must ref this
     *  if they want to keep it longer than the lifetime of the shader). If not, return nullptr.
     */
    SkImage* isAImage(SkMatrix* localMatrix, TileMode xy[2]) const {
        return this->onIsAImage(localMatrix, xy);
    }

    bool isAImage() const {
        return this->isAImage(nullptr, nullptr) != nullptr;
    }

    /**
     *  If the shader subclass can be represented as a gradient, asAGradient
     *  returns the matching GradientType enum (or kNone_GradientType if it
     *  cannot). Also, if info is not null, asAGradient populates info with
     *  the relevant (see below) parameters for the gradient.  fColorCount
     *  is both an input and output parameter.  On input, it indicates how
     *  many entries in fColors and fColorOffsets can be used, if they are
     *  non-NULL.  After asAGradient has run, fColorCount indicates how
     *  many color-offset pairs there are in the gradient.  If there is
     *  insufficient space to store all of the color-offset pairs, fColors
     *  and fColorOffsets will not be altered.  fColorOffsets specifies
     *  where on the range of 0 to 1 to transition to the given color.
     *  The meaning of fPoint and fRadius is dependant on the type of gradient.
     *
     *  None:
     *      info is ignored.
     *  Color:
     *      fColorOffsets[0] is meaningless.
     *  Linear:
     *      fPoint[0] and fPoint[1] are the end-points of the gradient
     *  Radial:
     *      fPoint[0] and fRadius[0] are the center and radius
     *  Conical:
     *      fPoint[0] and fRadius[0] are the center and radius of the 1st circle
     *      fPoint[1] and fRadius[1] are the center and radius of the 2nd circle
     *  Sweep:
     *      fPoint[0] is the center of the sweep.
     */

    enum GradientType {
        kNone_GradientType,
        kColor_GradientType,
        kLinear_GradientType,
        kRadial_GradientType,
        kSweep_GradientType,
        kConical_GradientType,
        kLast_GradientType = kConical_GradientType
    };

    struct GradientInfo {
        int         fColorCount;    //!< In-out parameter, specifies passed size
                                    //   of fColors/fColorOffsets on input, and
                                    //   actual number of colors/offsets on
                                    //   output.
        SkColor*    fColors;        //!< The colors in the gradient.
        SkScalar*   fColorOffsets;  //!< The unit offset for color transitions.
        SkPoint     fPoint[2];      //!< Type specific, see above.
        SkScalar    fRadius[2];     //!< Type specific, see above.
        TileMode    fTileMode;      //!< The tile mode used.
        uint32_t    fGradientFlags; //!< see SkGradientShader::Flags
    };

    virtual GradientType asAGradient(GradientInfo* info) const;

    /**
     *  If the shader subclass is composed of two shaders, return true, and if rec is not NULL,
     *  fill it out with info about the shader.
     *
     *  These are bare pointers; the ownership and reference count are unchanged.
     */

    struct ComposeRec {
        const SkShader*     fShaderA;
        const SkShader*     fShaderB;
        SkBlendMode         fBlendMode;
    };

    virtual bool asACompose(ComposeRec*) const { return false; }

#if SK_SUPPORT_GPU
    struct AsFPArgs {
        AsFPArgs() {}
        AsFPArgs(GrContext* context,
                 const SkMatrix* viewMatrix,
                 const SkMatrix* localMatrix,
                 SkFilterQuality filterQuality,
                 SkColorSpace* dstColorSpace)
            : fContext(context)
            , fViewMatrix(viewMatrix)
            , fLocalMatrix(localMatrix)
            , fFilterQuality(filterQuality)
            , fDstColorSpace(dstColorSpace) {}

        GrContext*                    fContext;
        const SkMatrix*               fViewMatrix;
        const SkMatrix*               fLocalMatrix;
        SkFilterQuality               fFilterQuality;
        SkColorSpace*                 fDstColorSpace;
    };

    /**
     *  Returns a GrFragmentProcessor that implements the shader for the GPU backend. NULL is
     *  returned if there is no GPU implementation.
     *
     *  The GPU device does not call SkShader::createContext(), instead we pass the view matrix,
     *  local matrix, and filter quality directly.
     *
     *  The GrContext may be used by the to create textures that are required by the returned
     *  processor.
     *
     *  The returned GrFragmentProcessor should expect an unpremultiplied input color and
     *  produce a premultiplied output.
     */
    virtual sk_sp<GrFragmentProcessor> asFragmentProcessor(const AsFPArgs&) const;
#endif

    /**
     *  If the shader can represent its "average" luminance in a single color, return true and
     *  if color is not NULL, return that color. If it cannot, return false and ignore the color
     *  parameter.
     *
     *  Note: if this returns true, the returned color will always be opaque, as only the RGB
     *  components are used to compute luminance.
     */
    bool asLuminanceColor(SkColor*) const;

    //////////////////////////////////////////////////////////////////////////
    //  Methods to create combinations or variants of shaders

    /**
     *  Return a shader that will apply the specified localMatrix to this shader.
     *  The specified matrix will be applied before any matrix associated with this shader.
     */
    sk_sp<SkShader> makeWithLocalMatrix(const SkMatrix&) const;

    /**
     *  Create a new shader that produces the same colors as invoking this shader and then applying
     *  the colorfilter.
     */
    sk_sp<SkShader> makeWithColorFilter(sk_sp<SkColorFilter>) const;

    //////////////////////////////////////////////////////////////////////////
    //  Factory methods for stock shaders

    /**
     *  Call this to create a new "empty" shader, that will not draw anything.
     */
    static sk_sp<SkShader> MakeEmptyShader();

    /**
     *  Call this to create a new shader that just draws the specified color. This should always
     *  draw the same as a paint with this color (and no shader).
     */
    static sk_sp<SkShader> MakeColorShader(SkColor);

    /**
     *  Create a shader that draws the specified color (in the specified colorspace).
     *
     *  This works around the limitation that SkPaint::setColor() only takes byte values, and does
     *  not support specific colorspaces.
     */
    static sk_sp<SkShader> MakeColorShader(const SkColor4f&, sk_sp<SkColorSpace>);

    static sk_sp<SkShader> MakeComposeShader(sk_sp<SkShader> dst, sk_sp<SkShader> src, SkBlendMode);

    /** Call this to create a new shader that will draw with the specified bitmap.
     *
     *  If the bitmap cannot be used (e.g. has no pixels, or its dimensions
     *  exceed implementation limits (currently at 64K - 1)) then SkEmptyShader
     *  may be returned.
     *
     *  If the src is kA8_Config then that mask will be colorized using the color on
     *  the paint.
     *
     *  @param src  The bitmap to use inside the shader
     *  @param tmx  The tiling mode to use when sampling the bitmap in the x-direction.
     *  @param tmy  The tiling mode to use when sampling the bitmap in the y-direction.
     *  @return     Returns a new shader object. Note: this function never returns null.
    */
    static sk_sp<SkShader> MakeBitmapShader(const SkBitmap& src, TileMode tmx, TileMode tmy,
                                            const SkMatrix* localMatrix = nullptr);

    // NOTE: You can create an SkImage Shader with SkImage::newShader().

    /** Call this to create a new shader that will draw with the specified picture.
     *
     *  @param src  The picture to use inside the shader (if not NULL, its ref count
     *              is incremented). The SkPicture must not be changed after
     *              successfully creating a picture shader.
     *  @param tmx  The tiling mode to use when sampling the bitmap in the x-direction.
     *  @param tmy  The tiling mode to use when sampling the bitmap in the y-direction.
     *  @param tile The tile rectangle in picture coordinates: this represents the subset
     *              (or superset) of the picture used when building a tile. It is not
     *              affected by localMatrix and does not imply scaling (only translation
     *              and cropping). If null, the tile rect is considered equal to the picture
     *              bounds.
     *  @return     Returns a new shader object. Note: this function never returns null.
    */
    static sk_sp<SkShader> MakePictureShader(sk_sp<SkPicture> src, TileMode tmx, TileMode tmy,
                                             const SkMatrix* localMatrix, const SkRect* tile);

    /**
     *  If this shader can be represented by another shader + a localMatrix, return that shader and
     *  the localMatrix. If not, return nullptr and ignore the localMatrix parameter.
     */
    virtual sk_sp<SkShader> makeAsALocalMatrixShader(SkMatrix* localMatrix) const;

    SK_TO_STRING_VIRT()
    SK_DEFINE_FLATTENABLE_TYPE(SkShader)
    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP()

    bool appendStages(SkRasterPipeline*, SkColorSpace*, SkArenaAlloc*,
                      const SkMatrix& ctm, const SkPaint&) const;

protected:
    void flatten(SkWriteBuffer&) const override;

    bool computeTotalInverse(const ContextRec&, SkMatrix* totalInverse) const;

    /**
     * Specialize creating a SkShader context using the supplied allocator.
     * @return pointer to context owned by the arena allocator.
     */
    virtual Context* onMakeContext(const ContextRec&, SkArenaAlloc*) const {
        return nullptr;
    }

    virtual bool onAsLuminanceColor(SkColor*) const {
        return false;
    }

#ifdef SK_SUPPORT_LEGACY_SHADER_ISABITMAP
    virtual bool onIsABitmap(SkBitmap*, SkMatrix*, TileMode[2]) const {
        return false;
    }
#endif

    virtual SkImage* onIsAImage(SkMatrix*, TileMode[2]) const {
        return nullptr;
    }

    virtual bool onAppendStages(SkRasterPipeline*, SkColorSpace*, SkArenaAlloc*,
                                const SkMatrix&, const SkPaint&,
                                const SkMatrix* /*local matrix*/) const;

private:
    // This is essentially const, but not officially so it can be modified in
    // constructors.
    SkMatrix fLocalMatrix;

    // So the SkLocalMatrixShader can whack fLocalMatrix in its SkReadBuffer constructor.
    friend class SkLocalMatrixShader;
    friend class SkBitmapProcLegacyShader;    // for computeTotalInverse()

    typedef SkFlattenable INHERITED;
};

#endif
