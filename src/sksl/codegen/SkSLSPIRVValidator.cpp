/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/codegen/SkSLSPIRVValidator.h"

#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLPosition.h"

#include "spirv-tools/libspirv.hpp"

namespace SkSL {

static bool validate_spirv(ErrorReporter& reporter, std::string_view program, bool disassemble) {
    SkASSERT(0 == program.size() % 4);
    const uint32_t* programData = reinterpret_cast<const uint32_t*>(program.data());
    size_t programSize = program.size() / 4;

    spvtools::SpirvTools tools(SPV_ENV_VULKAN_1_0);
    std::string errors;
    auto msgFn = [&errors](spv_message_level_t, const char*, const spv_position_t&, const char* m) {
        errors += "SPIR-V validation error: ";
        errors += m;
        errors += '\n';
    };
    tools.SetMessageConsumer(msgFn);

    bool result = tools.Validate(programData, programSize);
    if (result) {
        return true;
    }
    // If disassemble is set, we won't abort but instead append the errors. This allows
    // us to keep going and show all the errors, which is used by skslc to make sure
    // expected validation errors fail as expected..
    if (disassemble) {
        // Send the message, plus the entire disassembled SPIR-V (for easier context & debugging)
        // as if were an SkSL compile error message.
        std::string disassembly;
        uint32_t options = spvtools::SpirvTools::kDefaultDisassembleOption;
        options |= SPV_BINARY_TO_TEXT_OPTION_INDENT;
        if (tools.Disassemble(programData, programSize, &disassembly, options)) {
            errors.append(disassembly);
        }
        reporter.error(Position(), errors);
    } else {
        // Normal validation mode - abort() in a debug build with an error message.
        SkDEBUGFAILF("%s", errors.c_str());
    }
    return false;
}

bool ValidateSPIRV(ErrorReporter& reporter, std::string_view program) {
    return validate_spirv(reporter, program, false);
}

bool ValidateSPIRVAndDissassemble(ErrorReporter& reporter, std::string_view program) {
    return validate_spirv(reporter, program, true);
}

}  // namespace SkSL
