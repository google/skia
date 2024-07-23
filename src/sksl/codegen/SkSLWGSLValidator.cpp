/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/codegen/SkSLWGSLValidator.h"

#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLPosition.h"

#include "src/tint/lang/wgsl/extension.h"
#include "src/tint/lang/wgsl/reader/options.h"
#include "tint/tint.h"

namespace SkSL {

static bool validate_wgsl(ErrorReporter& reporter,
                          std::string_view wgsl,
                          bool appendError,
                          std::string* warnings) {
    // Enable the WGSL optional features that Skia might rely on.
    tint::wgsl::reader::Options options;
    for (auto extension : {tint::wgsl::Extension::kChromiumExperimentalPixelLocal,
                           tint::wgsl::Extension::kDualSourceBlending}) {
        options.allowed_features.extensions.insert(extension);
    }
    options.allowed_features.features.insert(
            tint::wgsl::LanguageFeature::kUnrestrictedPointerParameters);

    // Verify that the WGSL we produced is valid.
    tint::Source::File srcFile("", wgsl);
    tint::Program program(tint::wgsl::reader::Parse(&srcFile, options));

    if (program.Diagnostics().ContainsErrors()) {
        // The program isn't valid WGSL.
        if (appendError) {
            // Report the error via SkDEBUGFAIL and append the generated program for
            // ease of debugging. We don't do this for our golden test output because
            // it can change too often
            tint::diag::Formatter diagFormatter;
            std::string diagOutput = diagFormatter.Format(program.Diagnostics()).Plain();
            diagOutput += "\n";
            diagOutput += wgsl;
            SkDEBUGFAILF("%s", diagOutput.c_str());
        } else {
            reporter.error(Position(),
                           std::string("Tint compilation failed.\n\n") + std::string(wgsl));
        }
        return false;
    }

    if (!program.Diagnostics().empty()) {
        // The program contains warnings. Report them as-is.
        tint::diag::Formatter diagFormatter;
        *warnings = diagFormatter.Format(program.Diagnostics()).Plain();
    }
    return true;
}

bool ValidateWGSL(ErrorReporter& reporter, std::string_view wgsl, std::string* warnings) {
    return validate_wgsl(reporter, wgsl, false, warnings);
}

bool ValidateWGSLVerbose(ErrorReporter& reporter, std::string_view wgsl, std::string* warnings) {
    return validate_wgsl(reporter, wgsl, true, warnings);
}

}  // namespace SkSL
