/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProcessor_DEFINED
#define GrProcessor_DEFINED

#include "GrColor.h"
#include "GrProcessorUnitTest.h"
#include "GrProgramElement.h"
#include "GrTextureAccess.h"
#include "GrBufferAccess.h"
#include "SkMath.h"
#include "SkString.h"
#include "../private/SkAtomics.h"

class GrContext;
class GrCoordTransform;
class GrInvariantOutput;

/**
 * Used by processors to build their keys. It incorporates each per-processor key into a larger
 * shader key.
 */
class GrProcessorKeyBuilder {
public:
    GrProcessorKeyBuilder(SkTArray<unsigned char, true>* data) : fData(data), fCount(0) {
        SkASSERT(0 == fData->count() % sizeof(uint32_t));
    }

    void add32(uint32_t v) {
        ++fCount;
        fData->push_back_n(4, reinterpret_cast<uint8_t*>(&v));
    }

    /** Inserts count uint32_ts into the key. The returned pointer is only valid until the next
        add*() call. */
    uint32_t* SK_WARN_UNUSED_RESULT add32n(int count) {
        SkASSERT(count > 0);
        fCount += count;
        return reinterpret_cast<uint32_t*>(fData->push_back_n(4 * count));
    }

    size_t size() const { return sizeof(uint32_t) * fCount; }

private:
    SkTArray<uint8_t, true>* fData; // unowned ptr to the larger key.
    int fCount;                     // number of uint32_ts added to fData by the processor.
};

/** Provides custom shader code to the Ganesh shading pipeline. GrProcessor objects *must* be
    immutable: after being constructed, their fields may not change.

    Dynamically allocated GrProcessors are managed by a per-thread memory pool. The ref count of an
    processor must reach 0 before the thread terminates and the pool is destroyed.
 */
class GrProcessor : public GrProgramElement {
public:
    virtual ~GrProcessor();

    /** Human-meaningful string to identify this prcoessor; may be embedded
        in generated shader code. */
    virtual const char* name() const = 0;

    // Human-readable dump of all information 
    virtual SkString dumpInfo() const {
        SkString str;
        str.appendf("Missing data");
        return str;
    }

    int numTextures() const { return fTextureAccesses.count(); }

    /** Returns the access pattern for the texture at index. index must be valid according to
        numTextures(). */
    const GrTextureAccess& textureAccess(int index) const { return *fTextureAccesses[index]; }

    /** Shortcut for textureAccess(index).texture(); */
    GrTexture* texture(int index) const { return this->textureAccess(index).getTexture(); }

    int numBuffers() const { return fBufferAccesses.count(); }

    /** Returns the access pattern for the buffer at index. index must be valid according to
        numBuffers(). */
    const GrBufferAccess& bufferAccess(int index) const {
        return *fBufferAccesses[index];
    }

    /**
     * Platform specific built-in features that a processor can request for the fragment shader.
     */
    enum RequiredFeatures {
        kNone_RequiredFeatures             = 0,
        kFragmentPosition_RequiredFeature  = 1 << 0,
        kSampleLocations_RequiredFeature   = 1 << 1
    };

    GR_DECL_BITFIELD_OPS_FRIENDS(RequiredFeatures);

    RequiredFeatures requiredFeatures() const { return fRequiredFeatures; }

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

    uint32_t classID() const { SkASSERT(kIllegalProcessorClassID != fClassID); return fClassID; }

protected:
    GrProcessor() : fClassID(kIllegalProcessorClassID), fRequiredFeatures(kNone_RequiredFeatures) {}

    /**
     * Subclasses call these from their constructor to register sampler sources. The processor
     * subclass manages the lifetime of the objects (these functions only store pointers). The
     * GrTextureAccess and/or GrBufferAccess instances are typically member fields of the
     * GrProcessor subclass. These must only be called from the constructor because GrProcessors
     * are immutable.
     */
    virtual void addTextureAccess(const GrTextureAccess* textureAccess);
    virtual void addBufferAccess(const GrBufferAccess* bufferAccess);

    bool hasSameSamplers(const GrProcessor&) const;

    /**
     * If the prcoessor will generate code that uses platform specific built-in features, then it
     * must call these methods from its constructor. Otherwise, requests to use these features will
     * be denied.
     */
    void setWillReadFragmentPosition() { fRequiredFeatures |= kFragmentPosition_RequiredFeature; }
    void setWillUseSampleLocations() { fRequiredFeatures |= kSampleLocations_RequiredFeature; }

    void combineRequiredFeatures(const GrProcessor& other) {
        fRequiredFeatures |= other.fRequiredFeatures;
    }

    template <typename PROC_SUBCLASS> void initClassID() {
         static uint32_t kClassID = GenClassID();
         fClassID = kClassID;
    }

    uint32_t fClassID;
    SkSTArray<4, const GrTextureAccess*, true>   fTextureAccesses;
    SkSTArray<2, const GrBufferAccess*, true>    fBufferAccesses;

private:
    static uint32_t GenClassID() {
        // fCurrProcessorClassID has been initialized to kIllegalProcessorClassID. The
        // atomic inc returns the old value not the incremented value. So we add
        // 1 to the returned value.
        uint32_t id = static_cast<uint32_t>(sk_atomic_inc(&gCurrProcessorClassID)) + 1;
        if (!id) {
            SkFAIL("This should never wrap as it should only be called once for each GrProcessor "
                   "subclass.");
        }
        return id;
    }

    enum {
        kIllegalProcessorClassID = 0,
    };
    static int32_t gCurrProcessorClassID;

    RequiredFeatures fRequiredFeatures;

    typedef GrProgramElement INHERITED;
};

GR_MAKE_BITFIELD_OPS(GrProcessor::RequiredFeatures);

#endif
