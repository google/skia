/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_ASTLAYOUT
#define SKSL_ASTLAYOUT

#include "SkSLASTNode.h"
#include "SkSLUtil.h"

namespace SkSL {

/**
 * Represents a layout block appearing before a variable declaration, as in:
 *
 * layout (location = 0) int x;
 */
struct ASTLayout : public ASTNode {
    // These are used by images in GLSL. We only support a subset of what GL supports.
    enum class Format {
        kUnspecified = -1,
        kRGBA32F,
        kR32F,
        kRGBA16F,
        kR16F,
        kRGBA8,
        kR8,
        kRGBA8I,
        kR8I,
    };

    static const char* FormatToStr(Format format) {
        switch (format) {
            case Format::kUnspecified:  return "";
            case Format::kRGBA32F:      return "rgba32f";
            case Format::kR32F:         return "r32f";
            case Format::kRGBA16F:      return "rgba16f";
            case Format::kR16F:         return "r16f";
            case Format::kRGBA8:        return "rgba8";
            case Format::kR8:           return "r8";
            case Format::kRGBA8I:       return "rgba8i";
            case Format::kR8I:          return "r8i";
        }
        SkFAIL("Unexpected format");
        return "";
    }

    static bool ReadFormat(SkString str, Format* format) {
        if (str == "rgba32f") {
            *format = Format::kRGBA32F;
            return true;
        } else if (str == "r32f") {
            *format = Format::kR32F;
            return true;
        } else if (str == "rgba16f") {
            *format = Format::kRGBA16F;
            return true;
        } else if (str == "r16f") {
            *format = Format::kR16F;
            return true;
        } else if (str == "rgba8") {
            *format = Format::kRGBA8;
            return true;
        } else if (str == "r8") {
            *format = Format::kR8;
            return true;
        } else if (str == "rgba8i") {
            *format = Format::kRGBA8I;
            return true;
        } else if (str == "r8i") {
            *format = Format::kR8I;
            return true;
        }
        return false;
    }

    // For int parameters, a -1 means no value
    ASTLayout(int location, int binding, int index, int set, int builtin, bool originUpperLeft,
              bool overrideCoverage, bool blendSupportAllEquations, Format format)
    : fLocation(location)
    , fBinding(binding)
    , fIndex(index)
    , fSet(set)
    , fBuiltin(builtin)
    , fOriginUpperLeft(originUpperLeft)
    , fOverrideCoverage(overrideCoverage)
    , fBlendSupportAllEquations(blendSupportAllEquations)
    , fFormat(format) {}

    SkString description() const {
        SkString result;
        SkString separator;
        if (fLocation >= 0) {
            result += separator + "location = " + to_string(fLocation);
            separator = ", ";
        }
        if (fBinding >= 0) {
            result += separator + "binding = " + to_string(fBinding);
            separator = ", ";
        }
        if (fIndex >= 0) {
            result += separator + "index = " + to_string(fIndex);
            separator = ", ";
        }
        if (fSet >= 0) {
            result += separator + "set = " + to_string(fSet);
            separator = ", ";
        }
        if (fBuiltin >= 0) {
            result += separator + "builtin = " + to_string(fBuiltin);
            separator = ", ";
        }
        if (fOriginUpperLeft) {
            result += separator + "origin_upper_left";
            separator = ", ";
        }
        if (fOverrideCoverage) {
            result += separator + "override_coverage";
            separator = ", ";
        }
        if (fBlendSupportAllEquations) {
            result += separator + "blend_support_all_equations";
            separator = ", ";
        }
        if (fFormat != Format::kUnspecified) {
            result += separator + FormatToStr(fFormat);
            separator = ", ";
        }
        if (result.size() > 0) {
            result = "layout (" + result + ")";
        }
        return result;
    }

    const int fLocation;
    const int fBinding;
    const int fIndex;
    const int fSet;
    const int fBuiltin;
    const bool fOriginUpperLeft;
    const bool fOverrideCoverage;
    const bool fBlendSupportAllEquations;
    const Format fFormat;
};

} // namespace

#endif
