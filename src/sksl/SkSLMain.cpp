/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#define SK_OPTS_NS skslc_standalone
#include "src/opts/SkChecksum_opts.h"

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLDehydrator.h"
#include "src/sksl/SkSLFileOutputStream.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/SkSLStringStream.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/ir/SkSLEnum.h"
#include "src/sksl/ir/SkSLUnresolvedFunction.h"

#include <fstream>
#include <stdarg.h>
#include <stdio.h>

void SkDebugf(const char format[], ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

namespace SkOpts {
    decltype(hash_fn) hash_fn = skslc_standalone::hash_fn;
}

// Given the path to a file (e.g. src/gpu/effects/GrFooFragmentProcessor.fp) and the expected
// filename prefix and suffix (e.g. "Gr" and ".fp"), returns the "base name" of the
// file (in this case, 'FooFragmentProcessor'). If no match, returns the empty string.
static SkSL::String base_name(const char* fpPath, const char* prefix, const char* suffix) {
    SkSL::String result;
    const char* end = fpPath + strlen(fpPath);
    const char* fileName = end;
    // back up until we find a slash
    while (fileName != fpPath && '/' != *(fileName - 1) && '\\' != *(fileName - 1)) {
        --fileName;
    }
    if (!strncmp(fileName, prefix, strlen(prefix)) &&
        !strncmp(end - strlen(suffix), suffix, strlen(suffix))) {
        result.append(fileName + strlen(prefix), end - fileName - strlen(prefix) - strlen(suffix));
    }
    return result;
}

// Given a string containing an SkSL program, searches for a #pragma settings comment, like so:
//    /*#pragma settings Default Sharpen*/
// The passed-in Settings object will be updated accordingly. Any number of options can be provided.
static void detect_shader_settings(const SkSL::String& text, SkSL::Program::Settings* settings) {
    using Factory = SkSL::ShaderCapsFactory;

    // Find a matching comment and isolate the name portion.
    static constexpr char kPragmaSettings[] = "/*#pragma settings ";
    const char* settingsPtr = strstr(text.c_str(), kPragmaSettings);
    if (settingsPtr != nullptr) {
        // Subtract one here in order to preserve the leading space, which is necessary to allow
        // consumeSuffix to find the first item.
        settingsPtr += strlen(kPragmaSettings) - 1;

        const char* settingsEnd = strstr(settingsPtr, "*/");
        if (settingsEnd != nullptr) {
            SkSL::String settingsText{settingsPtr, size_t(settingsEnd - settingsPtr)};

            // Apply settings as requested. Since they can come in any order, repeat until we've
            // consumed them all.
            for (;;) {
                const size_t startingLength = settingsText.length();

                if (settingsText.consumeSuffix(" AddAndTrueToLoopCondition")) {
                    static auto s_addAndTrueCaps = Factory::AddAndTrueToLoopCondition();
                    settings->fCaps = s_addAndTrueCaps.get();
                }
                if (settingsText.consumeSuffix(" CannotUseFractForNegativeValues")) {
                    static auto s_negativeFractCaps = Factory::CannotUseFractForNegativeValues();
                    settings->fCaps = s_negativeFractCaps.get();
                }
                if (settingsText.consumeSuffix(" CannotUseFragCoord")) {
                    static auto s_noFragCoordCaps = Factory::CannotUseFragCoord();
                    settings->fCaps = s_noFragCoordCaps.get();
                }
                if (settingsText.consumeSuffix(" CannotUseMinAndAbsTogether")) {
                    static auto s_minAbsCaps = Factory::CannotUseMinAndAbsTogether();
                    settings->fCaps = s_minAbsCaps.get();
                }
                if (settingsText.consumeSuffix(" Default")) {
                    static auto s_defaultCaps = Factory::Default();
                    settings->fCaps = s_defaultCaps.get();
                }
                if (settingsText.consumeSuffix(" EmulateAbsIntFunction")) {
                    static auto s_emulateAbsIntCaps = Factory::EmulateAbsIntFunction();
                    settings->fCaps = s_emulateAbsIntCaps.get();
                }
                if (settingsText.consumeSuffix(" FragCoordsOld")) {
                    static auto s_fragCoordsOld = Factory::FragCoordsOld();
                    settings->fCaps = s_fragCoordsOld.get();
                }
                if (settingsText.consumeSuffix(" FragCoordsNew")) {
                    static auto s_fragCoordsNew = Factory::FragCoordsNew();
                    settings->fCaps = s_fragCoordsNew.get();
                }
                if (settingsText.consumeSuffix(" GeometryShaderExtensionString")) {
                    static auto s_geometryExtCaps = Factory::GeometryShaderExtensionString();
                    settings->fCaps = s_geometryExtCaps.get();
                }
                if (settingsText.consumeSuffix(" GeometryShaderSupport")) {
                    static auto s_geometryShaderCaps = Factory::GeometryShaderSupport();
                    settings->fCaps = s_geometryShaderCaps.get();
                }
                if (settingsText.consumeSuffix(" GSInvocationsExtensionString")) {
                    static auto s_gsInvocationCaps = Factory::GSInvocationsExtensionString();
                    settings->fCaps = s_gsInvocationCaps.get();
                }
                if (settingsText.consumeSuffix(" IncompleteShortIntPrecision")) {
                    static auto s_incompleteShortIntCaps = Factory::IncompleteShortIntPrecision();
                    settings->fCaps = s_incompleteShortIntCaps.get();
                }
                if (settingsText.consumeSuffix(" MustForceNegatedAtanParamToFloat")) {
                    static auto s_negativeAtanCaps = Factory::MustForceNegatedAtanParamToFloat();
                    settings->fCaps = s_negativeAtanCaps.get();
                }
                if (settingsText.consumeSuffix(" NoGSInvocationsSupport")) {
                    static auto s_noGSInvocations = Factory::NoGSInvocationsSupport();
                    settings->fCaps = s_noGSInvocations.get();
                }
                if (settingsText.consumeSuffix(" RemovePowWithConstantExponent")) {
                    static auto s_powCaps = Factory::RemovePowWithConstantExponent();
                    settings->fCaps = s_powCaps.get();
                }
                if (settingsText.consumeSuffix(" RewriteDoWhileLoops")) {
                    static auto s_rewriteLoopCaps = Factory::RewriteDoWhileLoops();
                    settings->fCaps = s_rewriteLoopCaps.get();
                }
                if (settingsText.consumeSuffix(" ShaderDerivativeExtensionString")) {
                    static auto s_derivativeCaps = Factory::ShaderDerivativeExtensionString();
                    settings->fCaps = s_derivativeCaps.get();
                }
                if (settingsText.consumeSuffix(" UnfoldShortCircuitAsTernary")) {
                    static auto s_ternaryCaps = Factory::UnfoldShortCircuitAsTernary();
                    settings->fCaps = s_ternaryCaps.get();
                }
                if (settingsText.consumeSuffix(" UsesPrecisionModifiers")) {
                    static auto s_precisionCaps = Factory::UsesPrecisionModifiers();
                    settings->fCaps = s_precisionCaps.get();
                }
                if (settingsText.consumeSuffix(" Version110")) {
                    static auto s_version110Caps = Factory::Version110();
                    settings->fCaps = s_version110Caps.get();
                }
                if (settingsText.consumeSuffix(" Version450Core")) {
                    static auto s_version450CoreCaps = Factory::Version450Core();
                    settings->fCaps = s_version450CoreCaps.get();
                }
                if (settingsText.consumeSuffix(" FlipY")) {
                    settings->fFlipY = true;
                }
                if (settingsText.consumeSuffix(" ForceHighPrecision")) {
                    settings->fForceHighPrecision = true;
                }
                if (settingsText.consumeSuffix(" Sharpen")) {
                    settings->fSharpenTextures = true;
                }

                if (settingsText.empty()) {
                    break;
                }
                if (settingsText.length() == startingLength) {
                    printf("Unrecognized #pragma settings: %s\n", settingsText.c_str());
                    exit(3);
                }
            }
        }
    }
}

/**
 * Very simple standalone executable to facilitate testing.
 */
int main(int argc, const char** argv) {
    bool honorSettings = true;
    if (argc == 4) {
        if (0 == strcmp(argv[3], "--settings")) {
            honorSettings = true;
        } else if (0 == strcmp(argv[3], "--nosettings")) {
            honorSettings = false;
        } else {
            printf("unrecognized flag: %s\n", argv[3]);
            exit(1);
        }
    } else if (argc != 3) {
        printf("usage: skslc <input> <output> <flags>\n"
               "\n"
               "Allowed flags:\n"
               "--settings:   honor embedded /*#pragma settings*/ comments.\n"
               "--nosettings: ignore /*#pragma settings*/ comments\n");
        exit(1);
    }

    SkSL::Program::Kind kind;
    SkSL::String input(argv[1]);
    if (input.endsWith(".vert")) {
        kind = SkSL::Program::kVertex_Kind;
    } else if (input.endsWith(".frag") || input.endsWith(".sksl")) {
        kind = SkSL::Program::kFragment_Kind;
    } else if (input.endsWith(".geom")) {
        kind = SkSL::Program::kGeometry_Kind;
    } else if (input.endsWith(".fp")) {
        kind = SkSL::Program::kFragmentProcessor_Kind;
    } else if (input.endsWith(".stage")) {
        kind = SkSL::Program::kPipelineStage_Kind;
    } else {
        printf("input filename must end in '.vert', '.frag', '.geom', '.fp', '.stage', or "
               "'.sksl'\n");
        exit(1);
    }

    std::ifstream in(argv[1]);
    SkSL::String text((std::istreambuf_iterator<char>(in)),
                       std::istreambuf_iterator<char>());
    if (in.rdstate()) {
        printf("error reading '%s'\n", argv[1]);
        exit(2);
    }

    SkSL::Program::Settings settings;
    if (honorSettings) {
        detect_shader_settings(text, &settings);
    }
    SkSL::String name(argv[2]);
    if (name.endsWith(".spirv")) {
        SkSL::FileOutputStream out(argv[2]);
        SkSL::Compiler compiler;
        if (!out.isValid()) {
            printf("error writing '%s'\n", argv[2]);
            exit(4);
        }
        std::unique_ptr<SkSL::Program> program = compiler.convertProgram(kind, text, settings);
        if (!program || !compiler.toSPIRV(*program, out)) {
            printf("%s", compiler.errorText().c_str());
            exit(3);
        }
        if (!out.close()) {
            printf("error writing '%s'\n", argv[2]);
            exit(4);
        }
    } else if (name.endsWith(".glsl")) {
        SkSL::FileOutputStream out(argv[2]);
        SkSL::Compiler compiler;
        if (!out.isValid()) {
            printf("error writing '%s'\n", argv[2]);
            exit(4);
        }
        std::unique_ptr<SkSL::Program> program = compiler.convertProgram(kind, text, settings);
        if (!program || !compiler.toGLSL(*program, out)) {
            printf("%s", compiler.errorText().c_str());
            exit(3);
        }
        if (!out.close()) {
            printf("error writing '%s'\n", argv[2]);
            exit(4);
        }
    } else if (name.endsWith(".metal")) {
        SkSL::FileOutputStream out(argv[2]);
        SkSL::Compiler compiler;
        if (!out.isValid()) {
            printf("error writing '%s'\n", argv[2]);
            exit(4);
        }
        std::unique_ptr<SkSL::Program> program = compiler.convertProgram(kind, text, settings);
        if (!program || !compiler.toMetal(*program, out)) {
            printf("%s", compiler.errorText().c_str());
            exit(3);
        }
        if (!out.close()) {
            printf("error writing '%s'\n", argv[2]);
            exit(4);
        }
    } else if (name.endsWith(".h")) {
        SkSL::FileOutputStream out(argv[2]);
        SkSL::Compiler compiler(SkSL::Compiler::kPermitInvalidStaticTests_Flag);
        if (!out.isValid()) {
            printf("error writing '%s'\n", argv[2]);
            exit(4);
        }
        settings.fReplaceSettings = false;
        std::unique_ptr<SkSL::Program> program = compiler.convertProgram(kind, text, settings);
        if (!program || !compiler.toH(*program, base_name(argv[1], "Gr", ".fp"), out)) {
            printf("%s", compiler.errorText().c_str());
            exit(3);
        }
        if (!out.close()) {
            printf("error writing '%s'\n", argv[2]);
            exit(4);
        }
    } else if (name.endsWith(".cpp")) {
        SkSL::FileOutputStream out(argv[2]);
        SkSL::Compiler compiler(SkSL::Compiler::kPermitInvalidStaticTests_Flag);
        if (!out.isValid()) {
            printf("error writing '%s'\n", argv[2]);
            exit(4);
        }
        settings.fReplaceSettings = false;
        std::unique_ptr<SkSL::Program> program = compiler.convertProgram(kind, text, settings);
        if (!program || !compiler.toCPP(*program, base_name(argv[1], "Gr", ".fp"), out)) {
            printf("%s", compiler.errorText().c_str());
            exit(3);
        }
        if (!out.close()) {
            printf("error writing '%s'\n", argv[2]);
            exit(4);
        }
    } else if (name.endsWith(".dehydrated.sksl")) {
        SkSL::FileOutputStream out(argv[2]);
        SkSL::Compiler compiler;
        if (!out.isValid()) {
            printf("error writing '%s'\n", argv[2]);
            exit(4);
        }
        std::shared_ptr<SkSL::SymbolTable> symbols;
        std::vector<std::unique_ptr<SkSL::ProgramElement>> elements;
        compiler.processIncludeFile(kind, argv[1], nullptr, &elements, &symbols);
        SkSL::Dehydrator dehydrator;
        dehydrator.write(*symbols);
        dehydrator.write(elements);
        SkSL::String baseName = base_name(argv[1], "", ".sksl");
        SkSL::StringStream buffer;
        dehydrator.finish(buffer);
        const SkSL::String& data = buffer.str();
        out.printf("static constexpr size_t SKSL_INCLUDE_%s_LENGTH = %d;\n", baseName.c_str(),
                   (int) data.length());
        out.printf("static uint8_t SKSL_INCLUDE_%s[%d] = {", baseName.c_str(), (int) data.length());
        for (size_t i = 0; i < data.length(); ++i) {
            out.printf("%d,", (uint8_t) data[i]);
        }
        out.printf("};\n");
        if (!out.close()) {
            printf("error writing '%s'\n", argv[2]);
            exit(4);
        }
    } else {
        printf("expected output filename to end with '.spirv', '.glsl', '.cpp', '.h', or '.metal'");
        exit(1);
    }
}
