/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawIndirectCmd_DEFINED
#define GrDrawIndirectCmd_DEFINED

#include "src/gpu/GrCaps.h"
#include <array>

// Draw commands on the GPU are simple tuples of uint32_t. The ordering is backend-specific.
using GrDrawIndirectCmd = std::array<uint32_t, 4>;
using GrDrawIndexedIndirectCmd = std::array<uint32_t, 5>;

// Helper for writing commands to an indirect draw buffer. Usage:
//
//    GrDrawIndirectCmdWriter indirectWriter = target->makeDrawIndirectSpace(...);
//    indirectWriter.write(...);
//    indirectWriter.write(...);
struct GrDrawIndirectCmdWriter {
public:
    GrDrawIndirectCmdWriter(void* ptr) : fCmds(static_cast<GrDrawIndirectCmd*>(ptr)) {}
    GrDrawIndirectCmdWriter() = default;

    bool isValid() const { return fCmds != nullptr; }

    inline void write(uint32_t instanceCount, uint32_t baseInstance, uint32_t vertexCount,
                      uint32_t baseVertex, const GrCaps&) {
        *fCmds++ = {vertexCount, instanceCount, baseVertex, baseInstance};
    }

private:
    GrDrawIndirectCmd* fCmds = nullptr;
};

// Helper for writing commands to an indexed-indirect draw buffer. Usage:
//
//    GrDrawIndexedIndirectCmdWriter indirectWriter = target->makeDrawIndexedIndirectSpace(...);
//    indirectWriter.writeIndexed(...);
//    indirectWriter.writeIndexed(...);
struct GrDrawIndexedIndirectCmdWriter {
public:
    GrDrawIndexedIndirectCmdWriter(void* ptr)
            : fCmds(static_cast<GrDrawIndexedIndirectCmd*>(ptr)) {}
    GrDrawIndexedIndirectCmdWriter() = default;

    bool isValid() const { return fCmds != nullptr; }

    inline void writeIndexed(uint32_t indexCount, uint32_t baseIndex, uint32_t instanceCount,
                             uint32_t baseInstance, uint32_t baseVertex, const GrCaps&) {
        *fCmds++ = {indexCount, instanceCount, baseIndex, baseVertex, baseInstance};
    }

private:
    GrDrawIndexedIndirectCmd* fCmds = nullptr;
};

#endif
