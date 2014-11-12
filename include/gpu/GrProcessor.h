/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProcessor_DEFINED
#define GrProcessor_DEFINED

#include "GrBackendProcessorFactory.h"
#include "GrColor.h"
#include "GrProcessorUnitTest.h"
#include "GrProgramElement.h"
#include "GrTextureAccess.h"
#include "SkMath.h"

class GrContext;
class GrCoordTransform;
class GrInvariantOutput;

/** Provides custom shader code to the Ganesh shading pipeline. GrProcessor objects *must* be
    immutable: after being constructed, their fields may not change.

    Dynamically allocated GrProcessors are managed by a per-thread memory pool. The ref count of an
    processor must reach 0 before the thread terminates and the pool is destroyed. To create a
    static processor use the helper macro GR_CREATE_STATIC_PROCESSOR declared below.
 */
class GrProcessor : public GrProgramElement {
public:
    SK_DECLARE_INST_COUNT(GrProcessor)

    virtual ~GrProcessor();

    /**
     * This function is used to perform optimizations. When called the invarientOuput param
     * indicate whether the input components to this processor in the FS will have known values.
     * In inout the validFlags member is a bitfield of GrColorComponentFlags. The isSingleComponent
     * member indicates whether the input will be 1 or 4 bytes. The function updates the members of
     * inout to indicate known values of its output. A component of the color member only has
     * meaning if the corresponding bit in validFlags is set.
     */
    void computeInvariantOutput(GrInvariantOutput* inout) const; 

    /** This object, besides creating back-end-specific helper objects, is used for run-time-type-
        identification. The factory should be an instance of templated class,
        GrTBackendProcessorFactory. It is templated on the subclass of GrProcessor. The subclass
        must have a nested type (or typedef) named GLProcessor which will be the subclass of
        GrGLProcessor created by the factory.

        Example:
        class MyCustomProcessor : public GrProcessor {
        ...
            virtual const GrBackendProcessorFactory& getFactory() const SK_OVERRIDE {
                return GrTBackendProcessorFactory<MyCustomProcessor>::getInstance();
            }
        ...
        };
     */
    virtual const GrBackendProcessorFactory& getFactory() const = 0;

    /** Human-meaningful string to identify this prcoessor; may be embedded
        in generated shader code. */
    const char* name() const;

    int numTextures() const { return fTextureAccesses.count(); }

    /** Returns the access pattern for the texture at index. index must be valid according to
        numTextures(). */
    const GrTextureAccess& textureAccess(int index) const { return *fTextureAccesses[index]; }

    /** Shortcut for textureAccess(index).texture(); */
    GrTexture* texture(int index) const { return this->textureAccess(index).getTexture(); }

    /** Will this processor read the fragment position? */
    bool willReadFragmentPosition() const { return fWillReadFragmentPosition; }

    void* operator new(size_t size);
    void operator delete(void* target);

    void* operator new(size_t size, void* placement) {
        return ::operator new(size, placement);
    }
    void operator delete(void* target, void* placement) {
        ::operator delete(target, placement);
    }

    /**
      * Helper for down-casting to a GrProcessor subclass
      */
    template <typename T> const T& cast() const { return *static_cast<const T*>(this); }

protected:
    GrProcessor() : fWillReadFragmentPosition(false) {}

    /**
     * Subclasses call this from their constructor to register GrTextureAccesses. The processor
     * subclass manages the lifetime of the accesses (this function only stores a pointer). The
     * GrTextureAccess is typically a member field of the GrProcessor subclass. This must only be
     * called from the constructor because GrProcessors are immutable.
     */
    void addTextureAccess(const GrTextureAccess* textureAccess);

    bool hasSameTextureAccesses(const GrProcessor&) const;

    /**
     * If the prcoessor will generate a backend-specific processor that will read the fragment
     * position in the FS then it must call this method from its constructor. Otherwise, the
     * request to access the fragment position will be denied.
     */
    void setWillReadFragmentPosition() { fWillReadFragmentPosition = true; }

private:
    /** 
     * Subclass implements this to support getConstantColorComponents(...).
     */
    virtual void onComputeInvariantOutput(GrInvariantOutput* inout) const = 0;

    SkSTArray<4, const GrTextureAccess*, true>   fTextureAccesses;
    bool                                         fWillReadFragmentPosition;

    typedef GrProgramElement INHERITED;
};


/**
 * This creates a processor outside of the memory pool. The processor's destructor will be called
 * at global destruction time. NAME will be the name of the created instance.
 */
#define GR_CREATE_STATIC_PROCESSOR(NAME, PROC_CLASS, ARGS)                                 \
static SkAlignedSStorage<sizeof(PROC_CLASS)> g_##NAME##_Storage;                           \
static PROC_CLASS* NAME SkNEW_PLACEMENT_ARGS(g_##NAME##_Storage.get(), PROC_CLASS, ARGS);  \
static SkAutoTDestroy<GrProcessor> NAME##_ad(NAME);

#endif
