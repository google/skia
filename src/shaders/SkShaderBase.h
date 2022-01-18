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
#include "include/private/SkNoncopyable.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkMask.h"
#include "src/core/SkTLazy.h"
#include "src/core/SkVM_fwd.h"

#if SK_SUPPORT_GPU
#include "src/gpu/GrFPArgs.h"
#endif

class GrFragmentProcessor;
class SkArenaAlloc;
enum class SkBackend : uint8_t;
class SkColorSpace;
class SkImage;
struct SkImageInfo;
class SkPaint;
class SkPaintParamsKey;
class SkRasterPipeline;
class SkRuntimeEffect;
class SkShaderCodeDictionary;
class SkStageUpdater;

class SkUpdatableShader;

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
            : fMatrix(&matrix)
            , fLocalMatrix(localM)
            , fDstColorType(dstColorType)
            , fDstColorSpace(dstColorSpace) {
                fPaintAlpha = paint.getAlpha();
                fPaintDither = paint.isDither();
            }

        const SkMatrix* fMatrix;           // the current matrix in the canvas
        const SkMatrix* fLocalMatrix;      // optional local matrix
        SkColorType     fDstColorType;     // the color type of the dest surface
        SkColorSpace*   fDstColorSpace;    // the color space of the dest surface (if any)
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

    // If this returns false, then we draw nothing (do not fall back to shader context)
    SK_WARN_UNUSED_RESULT
    bool appendStages(const SkStageRec&) const;

    bool SK_WARN_UNUSED_RESULT computeTotalInverse(const SkMatrix& ctm,
                                                   const SkMatrix* outerLocalMatrix,
                                                   SkMatrix* totalInverse) const;

    // Returns the total local matrix for this shader:
    //
    //   M = postLocalMatrix x shaderLocalMatrix x preLocalMatrix
    //
    SkTCopyOnFirstWrite<SkMatrix> totalLocalMatrix(const SkMatrix* preLocalMatrix) const;

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

    SkUpdatableShader* updatableShader(SkArenaAlloc* alloc) const;
    virtual SkUpdatableShader* onUpdatableShader(SkArenaAlloc* alloc) const;

    SkStageUpdater* appendUpdatableStages(const SkStageRec& rec) const {
        return this->onAppendUpdatableStages(rec);
    }

    SK_WARN_UNUSED_RESULT
    skvm::Color program(skvm::Builder*, skvm::Coord device, skvm::Coord local, skvm::Color paint,
                        const SkMatrixProvider&, const SkMatrix* localM, const SkColorInfo& dst,
                        skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const;


    /**
        Add implementation details, for the specified backend, of this SkShader to the
        provided key.

        @param dictionary dictionary of code fragments available to be used in the key
        @param backend    the backend that would be carrying out the drawing
        @param key        destination for implementation details of this SkShader
    */
    virtual void addToKey(SkShaderCodeDictionary* dictionary,
                          SkBackend backend,
                          SkPaintParamsKey* key) const;

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

    virtual SkStageUpdater* onAppendUpdatableStages(const SkStageRec&) const { return nullptr; }

protected:
    static skvm::Coord ApplyMatrix(skvm::Builder*, const SkMatrix&, skvm::Coord, skvm::Uniforms*);

private:
    // This is essentially const, but not officially so it can be modified in constructors.
    SkMatrix fLocalMatrix;

    virtual skvm::Color onProgram(skvm::Builder*,
                                  skvm::Coord device, skvm::Coord local, skvm::Color paint,
                                  const SkMatrixProvider&, const SkMatrix* localM,
                                  const SkColorInfo& dst, skvm::Uniforms*, SkArenaAlloc*) const = 0;

    using INHERITED = SkShader;
};

/**
 *  Shaders can optionally return a subclass of this when appending their stages.
 *  Doing so tells the caller that the stages can be reused with different CTMs (but nothing
 *  else can change), by calling the updater's update() method before each use.
 *
 *  This can be a perf-win bulk draws like drawAtlas and drawVertices, where most of the setup
 *  (i.e. uniforms) are constant, and only something small is changing (i.e. matrices). This
 *  reuse skips the cost of computing the stages (and/or avoids having to allocate a separate
 *  shader for each small draw.
 */
class SkStageUpdater {
public:
    virtual ~SkStageUpdater() {}
    virtual bool SK_WARN_UNUSED_RESULT update(const SkMatrix& ctm) const = 0;
};

// TODO: use the SkStageUpdater as an interface until all the code is converted over to use
//       SkUpdatableShader.
class SkUpdatableShader : public SkShaderBase, public SkStageUpdater {
private:
    // For serialization.  This will never be called.
    Factory getFactory() const override { return nullptr; }
    const char* getTypeName() const override { return nullptr; }
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
