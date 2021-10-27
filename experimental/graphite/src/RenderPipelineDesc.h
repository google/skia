/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_RenderPipelineDesc_DEFINED
#define skgpu_RenderPipelineDesc_DEFINED

#include "include/core/SkTypes.h"

#include "experimental/graphite/include/private/GraphiteTypesPriv.h"
#include "include/private/SkTArray.h"

namespace skgpu {

class RenderPipelineDesc {
public:
    RenderPipelineDesc();

    /** Describes a vertex or instance attribute. */
    class Attribute {
    public:
        constexpr Attribute() = default;
        constexpr Attribute(const char* name,
                            VertexAttribType cpuType,
                            SLType gpuType)
                : fName(name), fCPUType(cpuType), fGPUType(gpuType) {
            SkASSERT(name && gpuType != SLType::kVoid);
        }
        constexpr Attribute(const Attribute&) = default;

        Attribute& operator=(const Attribute&) = default;

        constexpr bool isInitialized() const { return fGPUType != SLType::kVoid; }

        constexpr const char* name() const { return fName; }
        constexpr VertexAttribType cpuType() const { return fCPUType; }
        constexpr SLType           gpuType() const { return fGPUType; }

        inline constexpr size_t size() const;
        constexpr size_t sizeAlign4() const { return SkAlign4(this->size()); }

    private:
        const char* fName = nullptr;
        VertexAttribType fCPUType = VertexAttribType::kFloat;
        SLType fGPUType = SLType::kVoid;
    };

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
        friend class RenderPipelineDesc;
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

    bool operator==(const RenderPipelineDesc& that) const {
        return this->fKey == that.fKey;
    }

    bool operator!=(const RenderPipelineDesc& other) const {
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

//////////////////////////////////////////////////////////////////////////////

/**
 * Returns the size of the attrib type in bytes.
 * Placed here in service of Skia dependents that build with C++11.
 */
static constexpr inline size_t VertexAttribTypeSize(VertexAttribType type) {
    switch (type) {
        case VertexAttribType::kFloat:
            return sizeof(float);
        case VertexAttribType::kFloat2:
            return 2 * sizeof(float);
        case VertexAttribType::kFloat3:
            return 3 * sizeof(float);
        case VertexAttribType::kFloat4:
            return 4 * sizeof(float);
        case VertexAttribType::kHalf:
            return sizeof(uint16_t);
        case VertexAttribType::kHalf2:
            return 2 * sizeof(uint16_t);
        case VertexAttribType::kHalf4:
            return 4 * sizeof(uint16_t);
        case VertexAttribType::kInt2:
            return 2 * sizeof(int32_t);
        case VertexAttribType::kInt3:
            return 3 * sizeof(int32_t);
        case VertexAttribType::kInt4:
            return 4 * sizeof(int32_t);
        case VertexAttribType::kByte:
            return 1 * sizeof(char);
        case VertexAttribType::kByte2:
            return 2 * sizeof(char);
        case VertexAttribType::kByte4:
            return 4 * sizeof(char);
        case VertexAttribType::kUByte:
            return 1 * sizeof(char);
        case VertexAttribType::kUByte2:
            return 2 * sizeof(char);
        case VertexAttribType::kUByte4:
            return 4 * sizeof(char);
        case VertexAttribType::kUByte_norm:
            return 1 * sizeof(char);
        case VertexAttribType::kUByte4_norm:
            return 4 * sizeof(char);
        case VertexAttribType::kShort2:
            return 2 * sizeof(int16_t);
        case VertexAttribType::kShort4:
            return 4 * sizeof(int16_t);
        case VertexAttribType::kUShort2: // fall through
        case VertexAttribType::kUShort2_norm:
            return 2 * sizeof(uint16_t);
        case VertexAttribType::kInt:
            return sizeof(int32_t);
        case VertexAttribType::kUint:
            return sizeof(uint32_t);
        case VertexAttribType::kUShort_norm:
            return sizeof(uint16_t);
        case VertexAttribType::kUShort4_norm:
            return 4 * sizeof(uint16_t);
    }
    // GCC fails because SK_ABORT evaluates to non constexpr. clang and cl.exe think this is
    // unreachable and don't complain.
#if defined(__clang__) || !defined(__GNUC__)
    SK_ABORT("Unsupported type conversion");
#endif
    return 0;
}

constexpr size_t RenderPipelineDesc::Attribute::size() const {
    return VertexAttribTypeSize(fCPUType);
}

} // namespace skgpu

#endif // skgpu_RenderPipelineDesc_DEFINED
