/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_GraphicsPipelineDesc_DEFINED
#define skgpu_GraphicsPipelineDesc_DEFINED

#include "include/core/SkTypes.h"

#include "experimental/graphite/src/Attribute.h"
#include "experimental/graphite/src/DrawTypes.h"
#include "include/private/SkTArray.h"

namespace skgpu {

/**
 * GraphicsPipelineDesc represents the state needed to create a backend specific GraphicsPipeline,
 * minus the target-specific properties that can be inferred from the DrawPass and RenderPassTask.
 */
class GraphicsPipelineDesc {
public:
    GraphicsPipelineDesc();

    // TODO: Iter and AttributeSet go away when the RenderStep is part of the GraphicsPipelineDesc.
    class Iter {
    public:
        Iter() : fCurr(nullptr), fRemaining(0) {}
        Iter(const Iter& iter) : fCurr(iter.fCurr), fRemaining(iter.fRemaining) {}
        Iter& operator= (const Iter& iter) {
            fCurr = iter.fCurr;
            fRemaining = iter.fRemaining;
            return *this;
        }
        Iter(const Attribute* attrs, int count) : fCurr(attrs), fRemaining(count) {
            this->skipUninitialized();
        }

        bool operator!=(const Iter& that) const { return fCurr != that.fCurr; }
        const Attribute& operator*() const { return *fCurr; }
        void operator++() {
            if (fRemaining) {
                fRemaining--;
                fCurr++;
                this->skipUninitialized();
            }
        }

    private:
        void skipUninitialized() {
            if (!fRemaining) {
                fCurr = nullptr;
            } else {
                while (!fCurr->isInitialized()) {
                    ++fCurr;
                }
            }
        }

        const Attribute* fCurr;
        int fRemaining;
    };

    class AttributeSet {
    public:
        Iter begin() const { return Iter(fAttributes, fCount); }
        Iter end() const { return Iter(); }

        int count() const { return fCount; }
        size_t stride() const { return fStride; }

    private:
        friend class GraphicsPipelineDesc;
        void init(const Attribute* attrs, int count) {
            fAttributes = attrs;
            fRawCount = count;
            fCount = 0;
            fStride = 0;
            for (int i = 0; i < count; ++i) {
                if (attrs[i].isInitialized()) {
                    fCount++;
                    fStride += attrs[i].sizeAlign4();
                }
            }
        }

        const Attribute* fAttributes = nullptr;
        int              fRawCount = 0;
        int              fCount = 0;
        size_t           fStride = 0;
    };

    // Returns this as a uint32_t array to be used as a key in the pipeline cache.
    // TODO: Do we want to do anything here with a tuple or an SkSpan?
    const uint32_t* asKey() const {
        return fKey.data();
    }

    // Gets the number of bytes in asKey(). It will be a 4-byte aligned value.
    uint32_t keyLength() const {
        return fKey.size() * sizeof(uint32_t);
    }

    bool operator==(const GraphicsPipelineDesc& that) const {
        return this->fKey == that.fKey;
    }

    bool operator!=(const GraphicsPipelineDesc& other) const {
        return !(*this == other);
    }

    // TODO: remove this once we have something real working
    void setTestingOnlyShaderIndex(int index) {
        fTestingOnlyShaderIndex = index;
        if (fKey.count() >= 1) {
            fKey[0] = index;
        } else {
            fKey.push_back(index);
        }
    }
    int testingOnlyShaderIndex() const {
        return fTestingOnlyShaderIndex;
    }

    void setVertexAttributes(const Attribute* attrs, int attrCount) {
        fVertexAttributes.init(attrs, attrCount);
    }
    void setInstanceAttributes(const Attribute* attrs, int attrCount) {
        SkASSERT(attrCount >= 0);
        fInstanceAttributes.init(attrs, attrCount);
    }

    int numVertexAttributes() const { return fVertexAttributes.fCount; }
    const AttributeSet& vertexAttributes() const { return fVertexAttributes; }
    int numInstanceAttributes() const { return fInstanceAttributes.fCount; }
    const AttributeSet& instanceAttributes() const { return fInstanceAttributes; }

    bool hasVertexAttributes() const { return SkToBool(fVertexAttributes.fCount); }
    bool hasInstanceAttributes() const { return SkToBool(fInstanceAttributes.fCount); }

    /**
     * A common practice is to populate the the vertex/instance's memory using an implicit array of
     * structs. In this case, it is best to assert that:
     *     stride == sizeof(struct)
     */
    size_t vertexStride() const { return fVertexAttributes.fStride; }
    size_t instanceStride() const { return fInstanceAttributes.fStride; }

private:
    // Estimate of max expected key size
    // TODO: flesh this out
    inline static constexpr int kPreAllocSize = 1;

    SkSTArray<kPreAllocSize, uint32_t, true> fKey;

    int fTestingOnlyShaderIndex;

    AttributeSet fVertexAttributes;
    AttributeSet fInstanceAttributes;
};

} // namespace skgpu

#endif // skgpu_GraphicsPipelineDesc_DEFINED
