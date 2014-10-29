/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProgramDesc_DEFINED
#define GrProgramDesc_DEFINED

#include "GrBackendProcessorFactory.h"
#include "GrColor.h"
#include "GrTypesPriv.h"
#include "SkChecksum.h"

class GrGpuGL;

/** This class describes a program to generate. It also serves as a program cache key. Very little
    of this is GL-specific. The GL-specific parts could be factored out into a subclass. */
class GrProgramDesc {
public:
    // Creates an uninitialized key that must be populated by GrGpu::buildProgramDesc()
    GrProgramDesc() {}

    // Returns this as a uint32_t array to be used as a key in the program cache.
    const uint32_t* asKey() const {
        return reinterpret_cast<const uint32_t*>(fKey.begin());
    }

    // Gets the number of bytes in asKey(). It will be a 4-byte aligned value. When comparing two
    // keys the size of either key can be used with memcmp() since the lengths themselves begin the
    // keys and thus the memcmp will exit early if the keys are of different lengths.
    uint32_t keyLength() const { return *this->atOffset<uint32_t, kLengthOffset>(); }

    // Gets the a checksum of the key. Can be used as a hash value for a fast lookup in a cache.
    uint32_t getChecksum() const { return *this->atOffset<uint32_t, kChecksumOffset>(); }

    GrProgramDesc& operator= (const GrProgramDesc& other) {
        size_t keyLength = other.keyLength();
        fKey.reset(keyLength);
        memcpy(fKey.begin(), other.fKey.begin(), keyLength);
        return *this;
    }

    bool operator== (const GrProgramDesc& other) const {
        // The length is masked as a hint to the compiler that the address will be 4 byte aligned.
        return 0 == memcmp(this->asKey(), other.asKey(), this->keyLength() & ~0x3);
    }

    bool operator!= (const GrProgramDesc& other) const {
        return !(*this == other);
    }

    static bool Less(const GrProgramDesc& a, const GrProgramDesc& b) {
        return memcmp(a.asKey(), b.asKey(), a.keyLength() & ~0x3) < 0;
    }


    ///////////////////////////////////////////////////////////////////////////
    /// @name Stage Output Types
    ////

    enum PrimaryOutputType {
        // Modulate color and coverage, write result as the color output.
        kModulate_PrimaryOutputType,
        // Combines the coverage, dst, and color as coverage * color + (1 - coverage) * dst. This
        // can only be set if fDstReadKey is non-zero.
        kCombineWithDst_PrimaryOutputType,

        kPrimaryOutputTypeCnt,
    };

    enum SecondaryOutputType {
        // There is no secondary output
        kNone_SecondaryOutputType,
        // Writes coverage as the secondary output. Only set if dual source blending is supported
        // and primary output is kModulate.
        kCoverage_SecondaryOutputType,
        // Writes coverage * (1 - colorA) as the secondary output. Only set if dual source blending
        // is supported and primary output is kModulate.
        kCoverageISA_SecondaryOutputType,
        // Writes coverage * (1 - colorRGBA) as the secondary output. Only set if dual source
        // blending is supported and primary output is kModulate.
        kCoverageISC_SecondaryOutputType,

        kSecondaryOutputTypeCnt,
    };

    // Specifies where the initial color comes from before the stages are applied.
    enum ColorInput {
        kAllOnes_ColorInput,
        kAttribute_ColorInput,
        kUniform_ColorInput,

        kColorInputCnt
    };

    struct KeyHeader {
        uint8_t                     fDstReadKey;   // set by GrGLShaderBuilder if there
                                                   // are effects that must read the dst.
                                                   // Otherwise, 0.
        uint8_t                     fFragPosKey;   // set by GrGLShaderBuilder if there are
                                                   // effects that read the fragment position.
                                                   // Otherwise, 0.

        SkBool8                     fEmitsPointSize;

        ColorInput                  fColorInput : 8;
        ColorInput                  fCoverageInput : 8;

        PrimaryOutputType           fPrimaryOutputType : 8;
        SecondaryOutputType         fSecondaryOutputType : 8;

        int8_t                      fPositionAttributeIndex;
        int8_t                      fLocalCoordAttributeIndex;
        int8_t                      fColorAttributeIndex;
        int8_t                      fCoverageAttributeIndex;

        SkBool8                     fHasGeometryProcessor;
        int8_t                      fColorEffectCnt;
        int8_t                      fCoverageEffectCnt;
    };


    bool hasGeometryProcessor() const {
        return SkToBool(this->header().fHasGeometryProcessor);
    }

    int numColorEffects() const {
        return this->header().fColorEffectCnt;
    }

    int numCoverageEffects() const {
        return this->header().fCoverageEffectCnt;
    }

    int numTotalEffects() const { return this->numColorEffects() + this->numCoverageEffects(); }

    // This should really only be used internally, base classes should return their own headers
    const KeyHeader& header() const { return *this->atOffset<KeyHeader, kHeaderOffset>(); }

    /** Used to provide effects' keys to their emitCode() function. */
    class ProcKeyProvider {
    public:
        enum ProcessorType {
            kGeometry_ProcessorType,
            kFragment_ProcessorType,
        };

        ProcKeyProvider(const GrProgramDesc* desc, ProcessorType type, int effectOffset)
            : fDesc(desc), fBaseIndex(0), fEffectOffset(effectOffset) {
            switch (type) {
                case kGeometry_ProcessorType:
                    // there can be only one
                    fBaseIndex = 0;
                    break;
                case kFragment_ProcessorType:
                    fBaseIndex = desc->hasGeometryProcessor() ? 1 : 0;
                    break;
            }
        }

        GrProcessorKey get(int index) const {
            const uint16_t* offsetsAndLengths = reinterpret_cast<const uint16_t*>(
                fDesc->fKey.begin() + fEffectOffset);
            // We store two uint16_ts per effect, one for the offset to the effect's key and one for
            // its length. Here we just need the offset.
            uint16_t offset = offsetsAndLengths[2 * (fBaseIndex + index) + 0];
            uint16_t length = offsetsAndLengths[2 * (fBaseIndex + index) + 1];
            // Currently effects must add to the key in units of uint32_t.
            SkASSERT(0 == (length % sizeof(uint32_t)));
            return GrProcessorKey(reinterpret_cast<const uint32_t*>(fDesc->fKey.begin() + offset),
                               length / sizeof(uint32_t));
        }
    private:
        const GrProgramDesc*  fDesc;
        int                   fBaseIndex;
        int                   fEffectOffset;
    };

    // A struct to communicate descriptor information to the program descriptor builder
    struct DescInfo {
        int positionAttributeIndex() const {
            return fFixedFunctionVertexAttribIndices[kPosition_GrVertexAttribBinding];
        }
        int localCoordAttributeIndex() const {
            return fFixedFunctionVertexAttribIndices[kLocalCoord_GrVertexAttribBinding];
        }
        int colorVertexAttributeIndex() const {
            return fFixedFunctionVertexAttribIndices[kColor_GrVertexAttribBinding];
        }
        int coverageVertexAttributeIndex() const {
            return fFixedFunctionVertexAttribIndices[kCoverage_GrVertexAttribBinding];
        }
        bool hasLocalCoordAttribute() const {
            return -1 != fFixedFunctionVertexAttribIndices[kLocalCoord_GrVertexAttribBinding];
        }
        bool hasColorVertexAttribute() const {
            return -1 != fFixedFunctionVertexAttribIndices[kColor_GrVertexAttribBinding];
        }
        bool hasCoverageVertexAttribute() const {
            return -1 != fFixedFunctionVertexAttribIndices[kCoverage_GrVertexAttribBinding];
        }

        int fFixedFunctionVertexAttribIndices[kGrFixedFunctionVertexAttribBindingCnt];

        // These flags are needed to protect the code from creating an unused uniform color/coverage
        // which will cause shader compiler errors.
        bool            fInputColorIsUsed;
        bool            fInputCoverageIsUsed;

        // These flags give aggregated info on the processor stages that are used when building
        // programs.
        bool            fReadsDst;
        bool            fReadsFragPosition;
        bool            fRequiresLocalCoordAttrib;

        // Fragment shader color outputs
        GrProgramDesc::PrimaryOutputType  fPrimaryOutputType : 8;
        GrProgramDesc::SecondaryOutputType  fSecondaryOutputType : 8;
    };

private:
    template<typename T, size_t OFFSET> T* atOffset() {
        return reinterpret_cast<T*>(reinterpret_cast<intptr_t>(fKey.begin()) + OFFSET);
    }

    template<typename T, size_t OFFSET> const T* atOffset() const {
        return reinterpret_cast<const T*>(reinterpret_cast<intptr_t>(fKey.begin()) + OFFSET);
    }

    void finalize() {
        int keyLength = fKey.count();
        SkASSERT(0 == (keyLength % 4));
        *(this->atOffset<uint32_t, GrProgramDesc::kLengthOffset>()) = SkToU32(keyLength);

        uint32_t* checksum = this->atOffset<uint32_t, GrProgramDesc::kChecksumOffset>();
        *checksum = 0;
        *checksum = SkChecksum::Compute(reinterpret_cast<uint32_t*>(fKey.begin()), keyLength);
    }

    // The key, stored in fKey, is composed of four parts:
    // 1. uint32_t for total key length.
    // 2. uint32_t for a checksum.
    // 3. Header struct defined above.  Also room for extensions to the header
    // 4. A Backend specific payload.  Room is preallocated for this
    enum KeyOffsets {
        // Part 1.
        kLengthOffset = 0,
        // Part 2.
        kChecksumOffset = kLengthOffset + sizeof(uint32_t),
        // Part 3.
        kHeaderOffset = kChecksumOffset + sizeof(uint32_t),
        kHeaderSize = SkAlign4(2 * sizeof(KeyHeader)),
    };

    enum {
        kMaxPreallocProcessors = 8,
        kIntsPerProcessor      = 4,    // This is an overestimate of the average effect key size.
        kPreAllocSize = kHeaderOffset + kHeaderSize +
                        kMaxPreallocProcessors * sizeof(uint32_t) * kIntsPerProcessor,
    };

    SkSTArray<kPreAllocSize, uint8_t, true> fKey;

    friend class GrGLProgramDescBuilder;
};

#endif
