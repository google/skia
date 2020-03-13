/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrContext.h"
#include "src/core/SkLRUCache.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrContextThreadSafeProxyPriv.h"
#include "src/gpu/GrProgramDesc.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/effects/GrSkSLFP.h"

/**
 * The DDL Context is the one in effect during DDL Recording. It isn't backed by a GrGPU and
 * cannot allocate any GPU resources.
 */
class GrDDLContext final : public GrContext {
public:
    GrDDLContext(sk_sp<GrContextThreadSafeProxy> proxy)
            : INHERITED(proxy->backend(), proxy->priv().options(), proxy->priv().contextID()) {
        fThreadSafeProxy = std::move(proxy);
    }

    ~GrDDLContext() override {}

    void abandonContext() override {
        SkASSERT(0); // abandoning in a DDL Recorder doesn't make a whole lot of sense
        INHERITED::abandonContext();
    }

    void releaseResourcesAndAbandonContext() override {
        SkASSERT(0); // abandoning in a DDL Recorder doesn't make a whole lot of sense
        INHERITED::releaseResourcesAndAbandonContext();
    }

    void freeGpuResources() override {
        SkASSERT(0); // freeing resources in a DDL Recorder doesn't make a whole lot of sense
        INHERITED::freeGpuResources();
    }

private:
    // TODO: Here we're pretending this isn't derived from GrContext. Switch this to be derived from
    // GrRecordingContext!
    GrContext* asDirectContext() override { return nullptr; }

    bool init(sk_sp<const GrCaps> caps) override {
        SkASSERT(caps);
        SkASSERT(fThreadSafeProxy); // should've been set in the ctor

        if (!INHERITED::init(std::move(caps))) {
            return false;
        }

        // DDL contexts/drawing managers always sort the oplists and attempt to reduce opsTask
        // splitting.
        this->setupDrawingManager(true, true);

        SkASSERT(this->caps());

        return true;
    }

    GrAtlasManager* onGetAtlasManager() override {
        SkASSERT(0);   // the DDL Recorders should never invoke this
        return nullptr;
    }

    // Add to the set of unique program infos required by this DDL
    void recordProgramInfo(const GrProgramInfo* programInfo) final {
        if (!programInfo) {
            return;
        }

        const GrCaps* caps = this->caps();

        if (this->backend() == GrBackendApi::kVulkan ||
            this->backend() == GrBackendApi::kMetal ||
            this->backend() == GrBackendApi::kDawn) {
            // Currently, Vulkan, Metal and Dawn require a live renderTarget to
            // compute the key
            return;
        }

        if (programInfo->requestedFeatures() & GrProcessor::CustomFeatures::kSampleLocations) {
            // Sample locations require a live renderTarget to compute the key
            return;
        }

        GrProgramDesc desc = caps->makeDesc(nullptr, *programInfo);
        if (!desc.isValid()) {
            return;
        }

        fProgramInfoMap.add(desc, programInfo);
    }

    void detachProgramData(SkTArray<ProgramData>* dst) final {
        SkASSERT(dst->empty());

        fProgramInfoMap.toArray(dst);
    }


private:
    class ProgramInfoMap : public ::SkNoncopyable {
        typedef const GrProgramDesc  CacheKey;
        typedef const GrProgramInfo* CacheValue;

    public:
        // All the programInfo data should be stored in the record-time arena so there is no
        // need to ref them here or to delete them in the destructor.
        ProgramInfoMap() : fMap(10) {}
        ~ProgramInfoMap() {}

        // TODO: this is doing a lot of reallocating of the ProgramDesc! Once the program descs
        // are allocated in the record-time area there won't be a problem.
        void add(CacheKey& desc, const GrProgramInfo* programInfo) {
            SkASSERT(desc.isValid());

            const CacheValue* preExisting = fMap.find(desc);
            if (preExisting) {
                return;
            }

            fMap.insert(desc, programInfo);
        }

        void toArray(SkTArray<ProgramData>* dst) {
            fMap.foreach([dst](CacheKey* programDesc, CacheValue* programInfo) {
                             // TODO: remove this allocation once the program descs are stored
                             // in the record-time arena.
                             dst->emplace_back(std::make_unique<const GrProgramDesc>(*programDesc),
                                               *programInfo);
                         });
        }

    private:
        struct DescHash {
            uint32_t operator()(CacheKey& desc) const {
                return SkOpts::hash_fn(desc.asKey(), desc.keyLength(), 0);
            }
        };

        SkLRUCache<CacheKey, CacheValue, DescHash> fMap;
    };

    ProgramInfoMap fProgramInfoMap;

    typedef GrContext INHERITED;
};

sk_sp<GrContext> GrContextPriv::MakeDDL(const sk_sp<GrContextThreadSafeProxy>& proxy) {
    sk_sp<GrContext> context(new GrDDLContext(proxy));

    if (!context->init(proxy->priv().refCaps())) {
        return nullptr;
    }
    return context;
}
