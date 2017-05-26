/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShaderBase_DEFINED
#define SkShaderBase_DEFINED

#include "SkFilterQuality.h"
#include "SkMatrix.h"
#include "SkShader.h"

class GrContext;
class GrFragmentProcessor;
class SkArenaAlloc;
class SkColorSpace;
class SkColorSpaceXformer;
class SkImage;
struct SkImageInfo;
class SkPaint;
class SkRasterPipeline;

class SK_API SkShaderBase : public SkShader {
public:
    SkShaderBase(const SkMatrix* localMatrix = nullptr);

    ~SkShaderBase() override;

    /**
     *  Returns true if the shader is guaranteed to produce only a single color.
     *  Subclasses can override this to allow loop-hoisting optimization.
     */
    virtual bool isConstant() const { return false; }

    const SkMatrix& getLocalMatrix() const { return fLocalMatrix; }

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
        Context(const SkShaderBase& shader, const ContextRec&);

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
        const SkShaderBase& fShader;

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

    /**
     *  Returns a shader transformed into a new color space via the |xformer|.
     */
    sk_sp<SkShader> makeColorSpace(SkColorSpaceXformer* xformer) const {
        return this->onMakeColorSpace(xformer);
    }

    virtual bool isRasterPipelineOnly() const { return false; }

    bool appendStages(SkRasterPipeline*, SkColorSpace* dstCS, SkArenaAlloc*,
                      const SkMatrix& ctm, const SkPaint&, const SkMatrix* localM=nullptr) const;

    bool computeTotalInverse(const SkMatrix& ctm,
                             const SkMatrix* outerLocalMatrix,
                             SkMatrix* totalInverse) const;

#ifdef SK_SUPPORT_LEGACY_SHADER_ISABITMAP
    virtual bool onIsABitmap(SkBitmap*, SkMatrix*, TileMode[2]) const {
        return false;
    }
#endif

    virtual SkImage* onIsAImage(SkMatrix*, TileMode[2]) const {
        return nullptr;
    }

    SK_TO_STRING_VIRT()

    SK_DEFINE_FLATTENABLE_TYPE(SkShaderBase)
    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP()

protected:
    void flatten(SkWriteBuffer&) const override;

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

    virtual sk_sp<SkShader> onMakeColorSpace(SkColorSpaceXformer*) const {
        return sk_ref_sp(const_cast<SkShaderBase*>(this));
    }

    virtual bool onAppendStages(SkRasterPipeline*, SkColorSpace* dstCS, SkArenaAlloc*,
                                const SkMatrix&, const SkPaint&, const SkMatrix* localM) const;

private:
    // This is essentially const, but not officially so it can be modified in constructors.
    SkMatrix fLocalMatrix;

    typedef SkShader INHERITED;
};

inline SkShaderBase* as_SB(SkShader* shader) {
    return static_cast<SkShaderBase*>(shader);
}

inline const SkShaderBase* as_SB(const SkShader* shader) {
    return static_cast<const SkShaderBase*>(shader);
}

inline const SkShaderBase* as_SB(const sk_sp<SkShader>& shader) {
    return static_cast<SkShaderBase*>(shader.get());
}

#endif // SkShaderBase_DEFINED
