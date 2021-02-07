/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawIndirectCmdWriter_DEFINED
#define GrDrawIndirectCmdWriter_DEFINED

#include "src/gpu/GrCaps.h"
#include <array>

/**
 * Helper for writing commands to an indirect draw buffer. Usage:
 *
 *    GrDrawIndirectCmdWriter cmds(target->makeDrawIndirectSpace(...));
 *    cmds.write(...);
 *    cmds.write(...);
 */
struct GrDrawIndirectCmdWriter {
public:
    using Cmd = std::array<uint32_t, 4>;

    GrDrawIndirectCmdWriter(void* ptr) : fCmds(static_cast<Cmd*>(ptr)) {}
    GrDrawIndirectCmdWriter() = default;

    bool isValid() const { return fCmds != nullptr; }

    // inline void write(uint32_t vertexCount, uint32_t instanceCount, uint32_t baseVertex,
    //                   uint32_t baseInstance, const GrCaps&) {
    inline void write(uint32_t instanceCount, uint32_t baseInstance, uint32_t vertexCount,
                      uint32_t baseVertex, const GrCaps&) {
        *fCmds++ = {vertexCount, instanceCount, baseVertex, baseInstance};
    }

private:
    Cmd* fCmds = nullptr;
};

/**
 * Helper for writing commands to an indexed-indirect draw buffer. Usage:
 *
 *    GrDrawIndirectCmdWriter cmds(target->makeDrawIndirectSpace(...));
 *    cmds.write(...);
 *    cmds.write(...);
 */
struct GrDrawIndexedIndirectCmdWriter {
public:
    using Cmd = std::array<uint32_t, 5>;

    GrDrawIndexedIndirectCmdWriter(void* ptr) : fCmds(static_cast<Cmd*>(ptr)) {}
    GrDrawIndexedIndirectCmdWriter() = default;

    bool isValid() const { return fCmds != nullptr; }
    // inline void writeIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t baseIndex,
    //                          uint32_t baseVertex, uint32_t baseInstance, const GrCaps&) {
    inline void writeIndexed(uint32_t indexCount, uint32_t baseIndex, uint32_t instanceCount,
                             uint32_t baseInstance, uint32_t baseVertex, const GrCaps&) {
        *fCmds++ = {indexCount, instanceCount, baseIndex, baseVertex, baseInstance};
    }

private:
    Cmd* fCmds = nullptr;
};

#endif
