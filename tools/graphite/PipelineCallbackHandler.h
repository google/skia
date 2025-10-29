/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PipelineCallbackHandler_DEFINED
#define PipelineCallbackHandler_DEFINED

#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "src/base/SkSpinlock.h"
#include "src/core/SkChecksum.h"
#include "src/core/SkTHash.h"

#include <vector>

class SkData;

namespace skiatools::graphite {

// This is intended to be an example of a Precompilation Callback handler.
// For the gr*testprecompile configs it is used to:
//   collect all the Android-style keys that are used by a given source (e.g., gm, or skp)
//   recreate the Pipelines from the collected keys in `resetAndRecreatePipelines`
// For the gr*testtracking configs it is used to:
//   collect all the Pipeline labels for a given source
//   print out a list of the labels at the end using `report`
class PipelineCallBackHandler {
public:
    static void CallBack(void* context,
                         skgpu::graphite::ContextOptions::PipelineCacheOp op,
                         const std::string& label,
                         uint32_t uniqueKeyHash,
                         bool fromPrecompile,
                         sk_sp<SkData> androidStyleKey) {
        PipelineCallBackHandler* handler = reinterpret_cast<PipelineCallBackHandler*>(context);

        handler->add(op, label, uniqueKeyHash, fromPrecompile, std::move(androidStyleKey));
    }

    void add(skgpu::graphite::ContextOptions::PipelineCacheOp op,
             const std::string& label,
             uint32_t uniqueKeyHash,
             bool fromPrecompile,
             sk_sp<SkData> androidStyleKey) SK_EXCLUDES(fSpinLock);

    void retrieveKeys(std::vector<sk_sp<SkData>>* result) SK_EXCLUDES(fSpinLock) {
        SkAutoSpinlock lock{ fSpinLock };

        result->reserve(fMap.count());

        fMap.foreach([result](std::unique_ptr<PipelineData>* data) {
            // Because not all Pipelines are serializable we need to check for nulls here
            if ((*data)->fAndroidStyleKey) {
                result->push_back((*data)->fAndroidStyleKey);
            }
        });
    }

    void reset() SK_EXCLUDES(fSpinLock) {
        SkAutoSpinlock lock{ fSpinLock };

        fMap.reset();
    }

    void report() SK_EXCLUDES(fSpinLock);

private:
    mutable SkSpinlock fSpinLock;

    struct PipelineData {
        PipelineData(const std::string& label,
                     uint32_t uniqueKeyHash,
                     bool fromPrecompile,
                     sk_sp<SkData> androidStyleKey) :
               fLabel(label),
               fAndroidStyleKey(std::move(androidStyleKey)),
               fUniqueKeyHash(uniqueKeyHash),
               fUses(fromPrecompile ? 0 : 1),
               fFromPrecompile(fromPrecompile) {
        }

        std::string   fLabel;
        sk_sp<SkData> fAndroidStyleKey;
        uint32_t      fUniqueKeyHash;
        uint32_t      fUses;
        bool          fFromPrecompile;
    };

    struct PipelineKey {
        const std::string* fLabel;
        uint32_t fUniqueKeyHash;

        static PipelineKey GetKey(const std::unique_ptr<PipelineData>& v) {
            return { &v->fLabel, v->fUniqueKeyHash };
        }
        static uint32_t Hash(const PipelineKey& k) { return k.fUniqueKeyHash; }

        bool operator==(const PipelineKey& other) const {
            return fUniqueKeyHash == other.fUniqueKeyHash && *fLabel == *other.fLabel;
        }
    };

    using Map = skia_private::THashTable<std::unique_ptr<PipelineData>, PipelineKey, PipelineKey>;
    Map fMap SK_GUARDED_BY(fSpinLock);
};

}  // namespace skiatools::graphite

#endif  // PipelineCallbackHandler_DEFINED
