/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawIndirectCommand_DEFINED
#define GrDrawIndirectCommand_DEFINED

#include "src/gpu/GrCaps.h"
#include <array>

// Draw commands on the GPU are simple tuples of uint32_t. The ordering is backend-specific.
using GrDrawIndirectCommand = std::array<uint32_t, 4>;
using GrDrawIndexedIndirectCommand = std::array<uint32_t, 5>;

// Helper for writing commands to an indirect draw buffer. Usage:
//
//    GrDrawIndirectWriter indirectWriter = target->makeDrawIndirectSpace(...);
//    indirectWriter.write(...);
//    indirectWriter.write(...);
struct GrDrawIndirectWriter {
public:
    GrDrawIndirectWriter() = default;
    GrDrawIndirectWriter(void* data) : fData(static_cast<GrDrawIndirectCommand*>(data)) {}
    GrDrawIndirectWriter(const GrDrawIndirectWriter&) = delete;
    GrDrawIndirectWriter(GrDrawIndirectWriter&& that) { *this = std::move(that); }

    GrDrawIndirectWriter& operator=(const GrDrawIndirectWriter&) = delete;
    GrDrawIndirectWriter& operator=(GrDrawIndirectWriter&& that) {
        fData = that.fData;
        that.fData = nullptr;
        return *this;
    }

    bool isValid() const { return fData != nullptr; }

    inline void write(uint32_t instanceCount, uint32_t baseInstance, uint32_t vertexCount,
                      uint32_t baseVertex, const GrCaps&) {
        *fData++ = {vertexCount, instanceCount, baseVertex, baseInstance};
    }

private:
    GrDrawIndirectCommand* fData;
};

// Helper for writing commands to an indexed indirect draw buffer. Usage:
//
//    GrDrawIndexedIndirectWriter indirectWriter = target->makeDrawIndexedIndirectSpace(...);
//    indirectWriter.writeIndexed(...);
//    indirectWriter.writeIndexed(...);
struct GrDrawIndexedIndirectWriter {
public:
    GrDrawIndexedIndirectWriter() = default;
    GrDrawIndexedIndirectWriter(void* data)
            : fData(static_cast<GrDrawIndexedIndirectCommand*>(data)) {}
    GrDrawIndexedIndirectWriter(const GrDrawIndexedIndirectWriter&) = delete;
    GrDrawIndexedIndirectWriter(GrDrawIndexedIndirectWriter&& that) { *this = std::move(that); }

    GrDrawIndexedIndirectWriter& operator=(const GrDrawIndexedIndirectWriter&) = delete;
    GrDrawIndexedIndirectWriter& operator=(GrDrawIndexedIndirectWriter&& that) {
        fData = that.fData;
        that.fData = nullptr;
        return *this;
    }

    bool isValid() const { return fData != nullptr; }

    inline void writeIndexed(uint32_t indexCount, uint32_t baseIndex, uint32_t instanceCount,
                             uint32_t baseInstance, uint32_t baseVertex, const GrCaps&) {
        *fData++ = {indexCount, instanceCount, baseIndex, baseVertex, baseInstance};
    }

private:
    GrDrawIndexedIndirectCommand* fData;
};

#endif
