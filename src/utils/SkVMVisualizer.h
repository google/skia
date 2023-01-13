/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkVMVisualizer_DEFINED
#define SkVMVisualizer_DEFINED

#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "src/core/SkTHash.h"
#include "src/core/SkVM.h"

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <vector>

class SkWStream;
namespace SkSL { class SkVMDebugTrace; }

namespace skvm::viz {
    enum InstructionFlags : uint8_t {
        kNormal = 0x00,
        kHoisted = 0x01,
        kDead = 0x02,
    };

    struct Instruction {
        InstructionFlags kind = InstructionFlags::kNormal;
        int instructionIndex; // index in the actual instructions list
        int duplicates = 0;   // number of duplicates;
                              // -1 means it's a duplicate itself; 0 - it does not have dups
        skvm::Instruction instruction;
        bool operator == (const Instruction& o) const;
        SkString classes() const;
    };

    struct InstructionHash {
        uint32_t operator()(const Instruction& i) const;
    };

    class Visualizer {
    public:
        explicit Visualizer(SkSL::SkVMDebugTrace* debugInfo);
        ~Visualizer() = default;
        void dump(SkWStream* output);
        void markAsDeadCode(std::vector<bool>& live, const std::vector<int>& newIds);
        void finalize(const std::vector<skvm::Instruction>& all,
                      const std::vector<skvm::OptimizedInstruction>& optimized);
        void addInstructions(std::vector<skvm::Instruction>& program);
        void markAsDuplicate(int origin, int id) {
            ++fInstructions[origin].duplicates;
        }
        void addInstruction(Instruction skvm);
        SkString V(int reg) const;
    private:
        void dumpInstruction(int id) const;
        void dumpHead() const;
        void dumpTail() const;
        void formatVV(const char* op, int v1, int v2) const;
        void formatPV(const char* op, int imm, int v1) const;
        void formatPVV(const char* op, int imm, int v1, int v2) const;
        void formatPVVVV(const char* op, int imm, int v1, int v2, int v3, int v4) const;
        void formatA_(int id, const char* op) const;
        void formatA_P(int id, const char* op, int imm) const;
        void formatA_PH(int id, const char* op, int immA, int immB) const;
        void formatA_PHH(int id, const char* op, int immA, int immB, int immC) const;
        void formatA_PHV(int id, const char* op, int immA, int immB, int v) const;
        void formatA_S(int id, const char* op, int imm) const;
        void formatA_V(int id, const char* op, int v) const;
        void formatA_VV(int id, const char* op, int v1, int v2) const;
        void formatA_VVV(int id, const char* op,  int v1, int v2, int v3) const;
        void formatA_VC(int id, const char* op,  int v, int imm) const;

        void writeText(const char* format, ...) const SK_PRINTF_LIKE(2, 3);
#if defined(SK_ENABLE_SKSL)
        SkSL::SkVMDebugTrace* fDebugInfo;
#endif
        SkTHashMap<Instruction, size_t, InstructionHash> fIndex;
        SkTArray<Instruction> fInstructions;
        SkWStream* fOutput;
        SkTHashMap<int, size_t> fToDisassembler;
    };
} // namespace skvm::viz

namespace sknonstd {
template <typename T> struct is_bitmask_enum;
template <> struct is_bitmask_enum<skvm::viz::InstructionFlags> : std::true_type {};
}  // namespace sknonstd

#endif // SkVMVisualizer_DEFINED
