/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShaderBase_DEFINED
#define SkShaderBase_DEFINED

#include "include/core/SkFilterQuality.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkShader.h"
#include "include/private/SkNoncopyable.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkMask.h"
#include "src/core/SkTLazy.h"

#if SK_SUPPORT_GPU
#include "src/gpu/GrFPArgs.h"
#endif

class GrContext;
class GrFragmentProcessor;
class SkArenaAlloc;
class SkColorSpace;
class SkImage;
struct SkImageInfo;
class SkPaint;
class SkRasterPipeline;

class SkShaderBase : public SkShader {
public:
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
        ContextRec(const SkPaint& paint, const SkMatrix& matrix, const SkMatrix* localM,
                   SkColorType dstColorType, SkColorSpace* dstColorSpace)
            : fPaint(&paint)
            , fMatrix(&matrix)
            , fLocalMatrix(localM)
            , fDstColorType(dstColorType)
            , fDstColorSpace(dstColorSpace) {}

        const SkPaint*  fPaint;            // the current paint associated with the draw
        const SkMatrix* fMatrix;           // the current matrix in the canvas
        const SkMatrix* fLocalMatrix;      // optional local matrix
        SkColorType     fDstColorType;     // the color type of the dest surface
        SkColorSpace*   fDstColorSpace;    // the color space of the dest surface (if any)

        bool isLegacyCompatible(SkColorSpace* shadersColorSpace) const;
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

    protected:
        // Reference to shader, so we don't have to dupe information.
        const SkShaderBase& fShader;

        uint8_t         getPaintAlpha() const { return fPaintAlpha; }
        const SkMatrix& getTotalInverse() const { return fTotalInverse; }
        const SkMatrix& getCTM() const { return fCTM; }

    private:
        SkMatrix    fCTM;
        SkMatrix    fTotalInverse;
        uint8_t     fPaintAlpha;

        typedef SkNoncopyable INHERITED;
    };

    /**
     * Make a context using the memory provided by the arena.
     *
     * @return pointer to context or nullptr if can't be created
     */
    Context* makeContext(const ContextRec&, SkArenaAlloc*) const;

#if SK_SUPPORT_GPU
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
    virtual std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&) const;
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

    // If this returns false, then we draw nothing (do not fall back to shader context)
    bool appendStages(const SkStageRec&) const;

    bool SK_WARN_UNUSED_RESULT computeTotalInverse(const SkMatrix& ctm,
                                                   const SkMatrix* outerLocalMatrix,
                                                   SkMatrix* totalInverse) const;

    // Returns the total local matrix for this shader:
    //
    //   M = postLocalMatrix x shaderLocalMatrix x preLocalMatrix
    //
    SkTCopyOnFirstWrite<SkMatrix> totalLocalMatrix(const SkMatrix* preLocalMatrix,
                                                   const SkMatrix* postLocalMatrix = nullptr) const;

    virtual SkImage* onIsAImage(SkMatrix*, SkTileMode[2]) const {
        return nullptr;
    }
    virtual SkPicture* isAPicture(SkMatrix*, SkTileMode[2], SkRect* tile) const { return nullptr; }

    static Type GetFlattenableType() { return kSkShaderBase_Type; }
    Type getFlattenableType() const override { return GetFlattenableType(); }

    static sk_sp<SkShaderBase> Deserialize(const void* data, size_t size,
                                             const SkDeserialProcs* procs = nullptr) {
        return sk_sp<SkShaderBase>(static_cast<SkShaderBase*>(
                SkFlattenable::Deserialize(GetFlattenableType(), data, size, procs).release()));
    }
    static void RegisterFlattenables();

    /** DEPRECATED. skbug.com/8941
     *  If this shader can be represented by another shader + a localMatrix, return that shader and
     *  the localMatrix. If not, return nullptr and ignore the localMatrix parameter.
     */
    virtual sk_sp<SkShader> makeAsALocalMatrixShader(SkMatrix* localMatrix) const;

protected:
    SkShaderBase(const SkMatrix* localMatrix = nullptr);

    void flatten(SkWriteBuffer&) const override;

#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
    /**
     * Specialize creating a SkShader context using the supplied allocator.
     * @return pointer to context owned by the arena allocator.
     */
    virtual Context* onMakeContext(const ContextRec&, SkArenaAlloc*) const {
        return nullptr;
    }
#endif

    virtual bool onAsLuminanceColor(SkColor*) const {
        return false;
    }

    // Default impl creates shadercontext and calls that (not very efficient)
    virtual bool onAppendStages(const SkStageRec&) const;

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
