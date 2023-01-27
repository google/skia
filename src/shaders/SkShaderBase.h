/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShaderBase_DEFINED
#define SkShaderBase_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkShader.h"
#include "include/core/SkSurfaceProps.h"
#include "include/private/base/SkNoncopyable.h"
#include "src/base/SkTLazy.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkMask.h"
#include "src/core/SkVM_fwd.h"

class GrFragmentProcessor;
struct GrFPArgs;
class SkArenaAlloc;
class SkColorSpace;
class SkImage;
struct SkImageInfo;
class SkPaint;
class SkRasterPipeline;
class SkRuntimeEffect;
class SkStageUpdater;
class SkUpdatableShader;

namespace skgpu::graphite {
class KeyContext;
class PaintParamsKeyBuilder;
class PipelineDataGatherer;
}

class SkShaderBase : public SkShader {
public:
    ~SkShaderBase() override;

    sk_sp<SkShader> makeInvertAlpha() const;
    sk_sp<SkShader> makeWithCTM(const SkMatrix&) const;  // owns its own ctm

    /**
     *  Returns true if the shader is guaranteed to produce only a single color.
     *  Subclasses can override this to allow loop-hoisting optimization.
     */
    virtual bool isConstant() const { return false; }

    enum class GradientType {
        kNone,
        kColor,
        kLinear,
        kRadial,
        kSweep,
        kConical
    };

    /**
     *  If the shader subclass can be represented as a gradient, asGradient
     *  returns the matching GradientType enum (or GradientType::kNone if it
     *  cannot). Also, if info is not null, asGradient populates info with
     *  the relevant (see below) parameters for the gradient.  fColorCount
     *  is both an input and output parameter.  On input, it indicates how
     *  many entries in fColors and fColorOffsets can be used, if they are
     *  non-NULL.  After asGradient has run, fColorCount indicates how
     *  many color-offset pairs there are in the gradient.  If there is
     *  insufficient space to store all of the color-offset pairs, fColors
     *  and fColorOffsets will not be altered.  fColorOffsets specifies
     *  where on the range of 0 to 1 to transition to the given color.
     *  The meaning of fPoint and fRadius is dependent on the type of gradient.
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
    struct GradientInfo {
        int         fColorCount    = 0;        //!< In-out parameter, specifies passed size
                                               //   of fColors/fColorOffsets on input, and
                                               //   actual number of colors/offsets on
                                               //   output.
        SkColor*    fColors        = nullptr;  //!< The colors in the gradient.
        SkScalar*   fColorOffsets  = nullptr;  //!< The unit offset for color transitions.
        SkPoint     fPoint[2];                 //!< Type specific, see above.
        SkScalar    fRadius[2];                //!< Type specific, see above.
        SkTileMode  fTileMode;
        uint32_t    fGradientFlags = 0;        //!< see SkGradientShader::Flags
    };

    virtual GradientType asGradient(GradientInfo* info    = nullptr,
                                    SkMatrix* localMatrix = nullptr) const {
        return GradientType::kNone;
    }

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
                   SkColorType dstColorType, SkColorSpace* dstColorSpace, SkSurfaceProps props)
            : fMatrix(&matrix)
            , fLocalMatrix(localM)
            , fDstColorType(dstColorType)
            , fDstColorSpace(dstColorSpace)
            , fProps(props) {
                fPaintAlpha = paint.getAlpha();
                fPaintDither = paint.isDither();
            }

        const SkMatrix* fMatrix;           // the current matrix in the canvas
        const SkMatrix* fLocalMatrix;      // optional local matrix
        SkColorType     fDstColorType;     // the color type of the dest surface
        SkColorSpace*   fDstColorSpace;    // the color space of the dest surface (if any)
        SkSurfaceProps  fProps;            // props of the dest surface
        SkAlpha         fPaintAlpha;
        bool            fPaintDither;

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

        using INHERITED = SkNoncopyable;
    };

    /**
     * This is used when building up SkRasterPipeline to accumulate matrices during downward
     * SkShader tree traversal rather than adding a matrix multiply stage for each one. It also
     * tracks the dubious concept of a "total matrix", which includes all matrices encountered,
     * including ones that have already been incorporated into matrix multiply stages.
     *
     * The total matrix is used for mip map level selection and a filter downgrade optimizations in
     * SkImageShader and sizing of the SkImage created by SkPictureShader. If we can remove usages
     * of the "total matrix" this could just be replaced by an SkMatrix or SkM44.
     */
    class MatrixRec {
    public:
        MatrixRec() = default;

        MatrixRec(const SkMatrixProvider&);

        /**
         * Returns a new MatrixRec that represents the existing total and pending matrix
         * pre-concat'ed with m.
         */
        MatrixRec SK_WARN_UNUSED_RESULT concat(const SkMatrix& m) const;

        /**
         * Appends a mul by the inverse of the pending local matrix to the pipeline. 'postInv' is an
         * additional matrix to post-apply to the inverted pending matrix. If the pending matrix is
         * not invertible the std::optional result won't have a value and the pipeline will be
         * unmodified.
         */
        std::optional<MatrixRec> SK_WARN_UNUSED_RESULT apply(const SkStageRec& rec,
                                                             const SkMatrix& postInv = {}) const;

        /** Call to indicate that the mapping from shader to device space is not known. */
        void markTotalMatrixInvalid() { fTotalMatrixIsValid = false; }

        /**
         * Indicates whether the total matrix of a MatrixRec passed to a SkShader actually
         * represents the full transform between that shader's coordinate space and device space.
         */
        bool totalMatrixIsValid() const { return fTotalMatrixIsValid; }

        /**
         * Gets the total transform from the current shader's space to device space. This may or
         * may not be valid. Shaders should avoid making decisions based on this matrix if
         * totalMatrixIsValid() is false.
         */
        SkMatrix totalMatrix() const { return fTotalMatrix; }

        /** Gets the inverse of totalMatrix(), if invertible. */
        bool SK_WARN_UNUSED_RESULT totalInverse(SkMatrix* out) const {
                return fTotalMatrix.invert(out);
        }

        /** Is there a transform that has not yet been applied by a parent shader? */
        bool hasPendingMatrix() const { return !fPendingMatrix.isIdentity(); }

    private:
        MatrixRec(const SkMatrix& pending, const SkMatrix& total, bool totalIsValid, bool rpSeeded)
                : fPendingMatrix(pending)
                , fTotalMatrix(total)
                , fTotalMatrixIsValid(totalIsValid)
                , fRPSeeded(rpSeeded) {}

        // The accumulated local matrices from walking down the shader hierarchy that have NOT yet
        // been incorporated into the SkRasterPipeline.
        const SkMatrix fPendingMatrix;
        // The total of all local matrices accumulated walking down the shader hierarchy including
        // those already incorporated into the SkRasterPipelines (including fPendingLocalMatrix).
        const SkMatrix fTotalMatrix;

        bool fTotalMatrixIsValid = true;

        // When used with raster pipeline, have the initial device coords been seeded.
        bool fRPSeeded = false;
    };

    /**
     * Make a context using the memory provided by the arena.
     *
     * @return pointer to context or nullptr if can't be created
     */
    Context* makeContext(const ContextRec&, SkArenaAlloc*) const;

#if SK_SUPPORT_GPU
    /**
     *  Returns a GrFragmentProcessor that implements the shader for the GPU backend. nullptr is
     *  returned if there is no GPU implementation.
     *
     *  The GPU device does not call SkShader::createContext(), instead we pass the view matrix,
     *  local matrix, and filter quality directly.
     *
     *  The GrRecordingContext may be used by the to create textures that are required by the
     *  returned processor.
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

    /**
     * If this returns false, then we draw nothing (do not fall back to shader context). This should
     * only be called on a root-level effect. It assumes that the initial device coordinates have
     * not yet been seeded.
     */
    SK_WARN_UNUSED_RESULT
    bool appendRootStages(const SkStageRec& rec, const SkMatrixProvider&) const;

    /**
     * Adds stages to implement this shader. To ensure that the correct input coords are present
     * in r,g MatrixRec::apply() must be called (unless the shader doesn't require it's input
     * coords). The default impl creates shadercontext and calls that (not very efficient).
     */
    virtual bool appendStages(const SkStageRec&, const MatrixRec&) const;

    bool SK_WARN_UNUSED_RESULT computeTotalInverse(const SkMatrix& ctm,
                                                   const SkMatrix* localMatrix,
                                                   SkMatrix* totalInverse) const;

    virtual SkImage* onIsAImage(SkMatrix*, SkTileMode[2]) const {
        return nullptr;
    }

    virtual SkRuntimeEffect* asRuntimeEffect() const { return nullptr; }

    static Type GetFlattenableType() { return kSkShader_Type; }
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
    SK_WARN_UNUSED_RESULT
    skvm::Color program(skvm::Builder*, skvm::Coord device, skvm::Coord local, skvm::Color paint,
                        const SkMatrixProvider&, const SkMatrix* localM, const SkColorInfo& dst,
                        skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const;


#ifdef SK_GRAPHITE_ENABLED
    /**
        Add implementation details, for the specified backend, of this SkShader to the
        provided key.

        @param keyContext backend context for key creation
        @param builder    builder for creating the key for this SkShader
        @param gatherer   if non-null, storage for this shader's data
    */
    virtual void addToKey(const skgpu::graphite::KeyContext& keyContext,
                          skgpu::graphite::PaintParamsKeyBuilder* builder,
                          skgpu::graphite::PipelineDataGatherer* gatherer) const;
#endif

    static SkMatrix ConcatLocalMatrices(const SkMatrix& parentLM, const SkMatrix& childLM) {
#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)  // b/256873449
        return SkMatrix::Concat(childLM, parentLM);
#endif
        return SkMatrix::Concat(parentLM, childLM);
    }

protected:
    SkShaderBase();

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

protected:
    static skvm::Coord ApplyMatrix(skvm::Builder*, const SkMatrix&, skvm::Coord, skvm::Uniforms*);

private:
    virtual skvm::Color onProgram(skvm::Builder*,
                                  skvm::Coord device, skvm::Coord local, skvm::Color paint,
                                  const SkMatrixProvider&, const SkMatrix* localM,
                                  const SkColorInfo& dst, skvm::Uniforms*, SkArenaAlloc*) const = 0;

    using INHERITED = SkShader;
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

void SkRegisterColor4ShaderFlattenable();
void SkRegisterColorShaderFlattenable();
void SkRegisterComposeShaderFlattenable();
void SkRegisterEmptyShaderFlattenable();

#endif // SkShaderBase_DEFINED
