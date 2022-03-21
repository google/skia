/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_PipelineDataCache_DEFINED
#define skgpu_PipelineDataCache_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/core/SkPipelineData.h"

#include <unordered_map>
#include <vector>

namespace skgpu {

// Add the block of data (DataBlockT) to the cache and return a unique ID that corresponds to its
// contents. If an identical block of data is already in the cache, that unique ID is returned.
// A DataBlockT must have:
//   uint32_t hash() const;
//   operator==
template<typename DataBlockT>
class PipelineDataCache {
public:
    static constexpr uint32_t kInvalidIndex = 0;

    PipelineDataCache() {
        // kInvalidIndex is reserved
        static_assert(kInvalidIndex == 0);
        fDataBlocks.push_back(nullptr);
        fDataBlockIDs.insert({nullptr, Index()});
    }

    // TODO: For the uniform data version of this cache we should revisit the insert and Make APIs:
    // 1. UniformData::Make requires knowing the data size up front, which involves two invocations
    //    of the UniformManager. Ideally, we could align uniforms on the fly into a dynamic buffer.
    // 2. UniformData stores the offsets for each uniform, but these aren't needed after we've
    //    filled out the buffer. If we remember layout offsets, it should be stored per Combination
    //    or RenderStep that defines the uniform set.
    // 3. UniformCache's ids are only fundamentally limited by the number of draws that can be
    //    recorded into a DrawPass, which means a very large recording with multiple passes could
    //    exceed uint32_t across all the passes.
    // 4. The check to know if a UniformData is present in the cache is practically the same for
    //    checking if the data needs to be uploaded to the GPU, so UniformCache could remember the
    //    associated BufferBindInfos as well.
    // 5. Because UniformCache only cares about the content byte hash/equality, and can memcpy to
    //    the GPU buffer, the cached data contents could all go into a shared byte array, instead of
    //    needing to extend SkRefCnt.
    // 6. insert() as a name can imply that the value is always added, so we may want a better one.
    //    It can be a little less generic if UniformCache returns id and bind buffer info. On the
    //    other hand unordered_map::insert has the same semantics as this insert, so maybe it's fine

    // Simple wrapper around the returned index to keep all the uint32_ts straight
    class Index {
    public:
        Index() : fIndex(kInvalidIndex) {}
        explicit Index(uint32_t index) : fIndex(index) {}

        bool operator==(const Index& that) const { return fIndex == that.fIndex; }
        bool operator!=(const Index& that) const { return !(*this == that); }
        bool isValid() const { return fIndex != kInvalidIndex; }
        uint32_t asUInt() const { return fIndex; }
    private:
        uint32_t fIndex;
    };

    Index insert(const DataBlockT& dataBlock) {
        auto kv = fDataBlockIDs.find(const_cast<DataBlockT*>(&dataBlock));
        if (kv != fDataBlockIDs.end()) {
            return kv->second;
        }

        Index id(SkTo<uint32_t>(fDataBlocks.size()));
        SkASSERT(id.isValid());

        // TODO: switch this over to using an SkArena
        std::unique_ptr<DataBlockT> tmp(new DataBlockT(dataBlock));
        fDataBlockIDs.insert({tmp.get(), id});
        fDataBlocks.push_back(std::move(tmp));
        this->validate();
        return id;
    }

    DataBlockT* lookup(Index uniqueID) {
        SkASSERT(uniqueID.asUInt() < fDataBlocks.size());
        return fDataBlocks[uniqueID.asUInt()].get();
    }

    // The number of unique DataBlockT objects in the cache
    size_t count() const {
        SkASSERT(fDataBlocks.size() == fDataBlockIDs.size() && fDataBlocks.size() > 0);
        return fDataBlocks.size() - 1;  /* -1 to discount the invalidID's entry */
    }

private:
    struct Hash {
        // This hash operator de-references and hashes the data contents
        size_t operator()(DataBlockT* dataBlock) const {
            if (!dataBlock) {
                return 0;
            }

            return dataBlock->hash();
        }
    };
    struct Eq {
        // This equality operator de-references and compares the actual data contents
        bool operator()(DataBlockT *a, DataBlockT *b) const {
            if (!a || !b) {
                return !a && !b;
            }

            return *a == *b;
        }
    };

    // Note: the unique IDs are only unique w/in a Recorder or a Recording _not_ globally
    std::unordered_map<DataBlockT*, Index, Hash, Eq> fDataBlockIDs;
    std::vector<std::unique_ptr<DataBlockT>> fDataBlocks;

    void validate() const {
#ifdef SK_DEBUG
        for (size_t i = 0; i < fDataBlocks.size(); ++i) {
            auto kv = fDataBlockIDs.find(fDataBlocks[i].get());
            SkASSERT(kv != fDataBlockIDs.end());
            SkASSERT(kv->first == fDataBlocks[i].get());
            SkASSERT(SkTo<uint32_t>(i) == kv->second.asUInt());
        }
#endif
    }
};

// A UniformDataCache lives for the entire duration of a Recorder. As such it has a greater
// likelihood of overflowing a uint32_t index.
using UniformDataCache = PipelineDataCache<SkUniformDataBlock>;

// A TextureDataCache only lives for a single Recording. When a Recording is snapped it is pulled
// off of the Recorder and goes with the Recording as a record of the required Textures and
// Samplers.
using TextureDataCache = PipelineDataCache<SkPipelineData::TextureDataBlock>;

} // namespace skgpu

#endif // skgpu_PipelineDataCache_DEFINED
