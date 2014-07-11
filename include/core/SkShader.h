/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkShader_DEFINED
#define SkShader_DEFINED

#include "SkBitmap.h"
#include "SkFlattenable.h"
#include "SkMask.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "../gpu/GrColor.h"

class SkPath;
class SkPicture;
class SkXfermode;
class GrContext;
class GrEffect;

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
    SK_DECLARE_INST_COUNT(SkShader)

    SkShader(const SkMatrix* localMatrix = NULL);
    virtual ~SkShader();

    /**
     *  Returns the local matrix.
     *
     *  FIXME: This can be incorrect for a Shader with its own local matrix
     *  that is also wrapped via CreateLocalMatrixShader.
     */
    const SkMatrix& getLocalMatrix() const { return fLocalMatrix; }

    /**
     *  Returns true if the local matrix is not an identity matrix.
     *
     *  FIXME: This can be incorrect for a Shader with its own local matrix
     *  that is also wrapped via CreateLocalMatrixShader.
     */
    bool hasLocalMatrix() const { return !fLocalMatrix.isIdentity(); }

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

        kTileModeCount
    };

    // override these in your subclass

    enum Flags {
        //!< set if all of the colors will be opaque
        kOpaqueAlpha_Flag  = 0x01,

        //! set if this shader's shadeSpan16() method can be called
        kHasSpan16_Flag = 0x02,

        /** Set this bit if the shader's native data type is instrinsically 16
            bit, meaning that calling the 32bit shadeSpan() entry point will
            mean the the impl has to up-sample 16bit data into 32bit. Used as a
            a means of clearing a dither request if the it will have no effect
        */
        kIntrinsicly16_Flag = 0x04,

        /** set if the spans only vary in X (const in Y).
            e.g. an Nx1 bitmap that is being tiled in Y, or a linear-gradient
            that varies from left-to-right. This flag specifies this for
            shadeSpan().
         */
        kConstInY32_Flag = 0x08,

        /** same as kConstInY32_Flag, but is set if this is true for shadeSpan16
            which may not always be the case, since shadeSpan16 may be
            predithered, which would mean it was not const in Y, even though
            the 32bit shadeSpan() would be const.
         */
        kConstInY16_Flag = 0x10
    };

    /**
     *  Returns true if the shader is guaranteed to produce only opaque
     *  colors, subject to the SkPaint using the shader to apply an opaque
     *  alpha value. Subclasses should override this to allow some
     *  optimizations.
     */
    virtual bool isOpaque() const { return false; }

    /**
     *  ContextRec acts as a parameter bundle for creating Contexts.
     */
    struct ContextRec {
        ContextRec() : fDevice(NULL), fPaint(NULL), fMatrix(NULL), fLocalMatrix(NULL) {}
        ContextRec(const SkBitmap& device, const SkPaint& paint, const SkMatrix& matrix)
            : fDevice(&device)
            , fPaint(&paint)
            , fMatrix(&matrix)
            , fLocalMatrix(NULL) {}

        const SkBitmap* fDevice;        // the bitmap we are drawing into
        const SkPaint*  fPaint;         // the current paint associated with the draw
        const SkMatrix* fMatrix;        // the current matrix in the canvas
        const SkMatrix* fLocalMatrix;   // optional local matrix
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
         *  Return the alpha associated with the data returned by shadeSpan16(). If
         *  kHasSpan16_Flag is not set, this value is meaningless.
         */
        virtual uint8_t getSpan16Alpha() const { return fPaintAlpha; }

        /**
         *  Called for each span of the object being drawn. Your subclass should
         *  set the appropriate colors (with premultiplied alpha) that correspond
         *  to the specified device coordinates.
         */
        virtual void shadeSpan(int x, int y, SkPMColor[], int count) = 0;

        typedef void (*ShadeProc)(void* ctx, int x, int y, SkPMColor[], int count);
        virtual ShadeProc asAShadeProc(void** ctx);

        /**
         *  Called only for 16bit devices when getFlags() returns
         *  kOpaqueAlphaFlag | kHasSpan16_Flag
         */
        virtual void shadeSpan16(int x, int y, uint16_t[], int count);

        /**
         *  Similar to shadeSpan, but only returns the alpha-channel for a span.
         *  The default implementation calls shadeSpan() and then extracts the alpha
         *  values from the returned colors.
         */
        virtual void shadeSpanAlpha(int x, int y, uint8_t alpha[], int count);

        /**
         *  Helper function that returns true if this shader's shadeSpan16() method
         *  can be called.
         */
        bool canCallShadeSpan16() {
            return SkShader::CanCallShadeSpan16(this->getFlags());
        }

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
    private:
        SkMatrix    fCTM;
        SkMatrix    fTotalInverse;
        uint8_t     fPaintAlpha;
        uint8_t     fTotalInverseClass;

        typedef SkNoncopyable INHERITED;
    };

    /**
     *  Create the actual object that does the shading.
     *  Size of storage must be >= contextSize.
     */
    Context* createContext(const ContextRec&, void* storage) const;

    /**
     *  Return the size of a Context returned by createContext.
     *
     *  Override this if your subclass overrides createContext, to return the correct size of
     *  your subclass' context.
     */
    virtual size_t contextSize() const;

    /**
     *  Helper to check the flags to know if it is legal to call shadeSpan16()
     */
    static bool CanCallShadeSpan16(uint32_t flags) {
        return (flags & kHasSpan16_Flag) != 0;
    }

    /**
     Gives method bitmap should be read to implement a shader.
     Also determines number and interpretation of "extra" parameters returned
     by asABitmap
     */
    enum BitmapType {
        kNone_BitmapType,   //<! Shader is not represented as a bitmap
        kDefault_BitmapType,//<! Access bitmap using local coords transformed
                            //   by matrix. No extras
        kRadial_BitmapType, //<! Access bitmap by transforming local coordinates
                            //   by the matrix and taking the distance of result
                            //   from  (0,0) as bitmap column. Bitmap is 1 pixel
                            //   tall. No extras
        kSweep_BitmapType,  //<! Access bitmap by transforming local coordinates
                            //   by the matrix and taking the angle of result
                            //   to (0,0) as bitmap x coord, where angle = 0 is
                            //   bitmap left edge of bitmap = 2pi is the
                            //   right edge. Bitmap is 1 pixel tall. No extras
        kTwoPointRadial_BitmapType,
                            //<! Matrix transforms to space where (0,0) is
                            //   the center of the starting circle.  The second
                            //   circle will be centered (x, 0) where x  may be
                            //   0. The post-matrix space is normalized such
                            //   that 1 is the second radius - first radius.
                            //   Three extra parameters are returned:
                            //      0: x-offset of second circle center
                            //         to first.
                            //      1: radius of first circle in post-matrix
                            //         space
                            //      2: the second radius minus the first radius
                            //         in pre-transformed space.
        kTwoPointConical_BitmapType,
                            //<! Matrix transforms to space where (0,0) is
                            //   the center of the starting circle.  The second
                            //   circle will be centered (x, 0) where x  may be
                            //   0.
                            //   Three extra parameters are returned:
                            //      0: x-offset of second circle center
                            //         to first.
                            //      1: radius of first circle
                            //      2: the second radius minus the first radius
        kLinear_BitmapType, //<! Access bitmap using local coords transformed
                            //   by matrix. No extras

       kLast_BitmapType = kLinear_BitmapType
    };
    /** Optional methods for shaders that can pretend to be a bitmap/texture
        to play along with opengl. Default just returns kNone_BitmapType and
        ignores the out parameters.

        @param outTexture if non-NULL will be the bitmap representing the shader
                          after return.
        @param outMatrix  if non-NULL will be the matrix to apply to vertices
                          to access the bitmap after return.
        @param xy         if non-NULL will be the tile modes that should be
                          used to access the bitmap after return.
        @param twoPointRadialParams Two extra return values needed for two point
                                    radial bitmaps. The first is the x-offset of
                                    the second point and the second is the radius
                                    about the first point.
    */
    virtual BitmapType asABitmap(SkBitmap* outTexture, SkMatrix* outMatrix,
                         TileMode xy[2]) const;

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
     *  Radial2:
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
        kRadial2_GradientType,
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
        const SkXfermode*   fMode;
    };

    virtual bool asACompose(ComposeRec* rec) const { return false; }


    /**
     *  Returns true if the shader subclass succeeds in creating an effect or if none is required.
     *  False is returned if it fails or if there is not an implementation of this method in the
     *  shader subclass.
     *
     *  On success an implementation of this method must inspect the SkPaint and set paintColor to
     *  the color the effect expects as its input color. If the SkShader wishes to emit a solid
     *  color then it should set paintColor to that color and not create an effect. Note that
     *  GrColor is always premul. The common patterns are to convert paint's SkColor to GrColor or
     *  to extract paint's alpha and replicate it to all channels in paintColor. Upon failure
     *  paintColor should not be modified. It is not recommended to specialize the effect to
     *  the paint's color as then many GPU shaders may be generated.
     *
     *  The GrContext may be used by the effect to create textures. The GPU device does not
     *  call createContext. Instead we pass the SkPaint here in case the shader needs paint info.
     */
    virtual bool asNewEffect(GrContext* context, const SkPaint& paint,
                             const SkMatrix* localMatrixOrNull, GrColor* paintColor,
                             GrEffect** effect) const;

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    /**
     *  If the shader is a custom shader which has data the caller might want, call this function
     *  to get that data.
     */
    virtual bool asACustomShader(void** customData) const { return false; }
#endif

    //////////////////////////////////////////////////////////////////////////
    //  Factory methods for stock shaders

    /**
     *  Call this to create a new "empty" shader, that will not draw anything.
     */
    static SkShader* CreateEmptyShader();

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
    static SkShader* CreateBitmapShader(const SkBitmap& src,
                                        TileMode tmx, TileMode tmy,
                                        const SkMatrix* localMatrix = NULL);

    /** Call this to create a new shader that will draw with the specified picture.
     *
     *  @param src  The picture to use inside the shader (if not NULL, its ref count
     *              is incremented). The SkPicture must not be changed after
     *              successfully creating a picture shader.
     *              FIXME: src cannot be const due to SkCanvas::drawPicture
     *  @param tmx  The tiling mode to use when sampling the bitmap in the x-direction.
     *  @param tmy  The tiling mode to use when sampling the bitmap in the y-direction.
     *  @return     Returns a new shader object. Note: this function never returns null.
    */
    static SkShader* CreatePictureShader(SkPicture* src, TileMode tmx, TileMode tmy,
                                         const SkMatrix* localMatrix = NULL);

    /**
     *  Return a shader that will apply the specified localMatrix to the proxy shader.
     *  The specified matrix will be applied before any matrix associated with the proxy.
     *
     *  Note: ownership of the proxy is not transferred (though a ref is taken).
     */
    static SkShader* CreateLocalMatrixShader(SkShader* proxy, const SkMatrix& localMatrix);

    /**
     *  If this shader can be represented by another shader + a localMatrix, return that shader
     *  and, if not NULL, the localMatrix. If not, return NULL and ignore the localMatrix parameter.
     *
     *  Note: the returned shader (if not NULL) will have been ref'd, and it is the responsibility
     *  of the caller to balance that with unref() when they are done.
     */
    virtual SkShader* refAsALocalMatrixShader(SkMatrix* localMatrix) const;

    SK_TO_STRING_VIRT()
    SK_DEFINE_FLATTENABLE_TYPE(SkShader)

protected:
    SkShader(SkReadBuffer& );
    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;

    bool computeTotalInverse(const ContextRec&, SkMatrix* totalInverse) const;

    /**
     *  Your subclass must also override contextSize() if it overrides onCreateContext().
     *  Base class impl returns NULL.
     */
    virtual Context* onCreateContext(const ContextRec&, void* storage) const;

private:
    // This is essentially const, but not officially so it can be modified in
    // constructors.
    SkMatrix fLocalMatrix;

    // So the SkLocalMatrixShader can whack fLocalMatrix in its SkReadBuffer constructor.
    friend class SkLocalMatrixShader;

    typedef SkFlattenable INHERITED;
};

#endif
