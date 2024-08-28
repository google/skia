/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/gpu/ganesh/GrContextThreadSafeProxy.h"
#include "include/gpu/ganesh/GrRecordingContext.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkNoncopyable.h"
#include "include/private/base/SkTArray.h"
#include "src/core/SkChecksum.h"
#include "src/core/SkLRUCache.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrProgramDesc.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"

#include <cstdint>
#include <memory>
#include <utility>

class GrProgramInfo;

using namespace skia_private;

/**
 * The DDL Context is the one in effect during DDL Recording. It isn't backed by a GrGPU and
 * cannot allocate any GPU resources.
 */
class GrDDLContext final : public GrRecordingContext {
public:
    GrDDLContext(sk_sp<GrContextThreadSafeProxy> proxy)
        : INHERITED(std::move(proxy), true) {
    }

    ~GrDDLContext() override {}

    void abandonContext() override {
        SkASSERT(0); // abandoning in a DDL Recorder doesn't make a whole lot of sense
        INHERITED::abandonContext();
    }

private:
    // Add to the set of unique program infos required by this DDL
    void recordProgramInfo(const GrProgramInfo* programInfo) final {
        if (!programInfo) {
            return;
        }

        const GrCaps* caps = this->caps();

        if (this->backend() == GrBackendApi::kMetal || this->backend() == GrBackendApi::kDirect3D) {
            // Currently Metal and Direct3D require a live renderTarget to compute the key
            return;
        }

        GrProgramDesc desc = caps->makeDesc(nullptr, *programInfo);
        if (!desc.isValid()) {
            return;
        }

        fProgramInfoMap.add(desc, programInfo);
    }

    void detachProgramData(TArray<ProgramData>* dst) final {
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

        void toArray(TArray<ProgramData>* dst) {
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
                return SkChecksum::Hash32(desc.asKey(), desc.keyLength());
            }
        };

        SkLRUCache<CacheKey, CacheValue, DescHash> fMap;
    };

    ProgramInfoMap fProgramInfoMap;

    using INHERITED = GrRecordingContext;
};

sk_sp<GrRecordingContext> GrRecordingContextPriv::MakeDDL(sk_sp<GrContextThreadSafeProxy> proxy) {
    sk_sp<GrRecordingContext> context(new GrDDLContext(std::move(proxy)));

    if (!context->init()) {
        return nullptr;
    }
    return context;
}
