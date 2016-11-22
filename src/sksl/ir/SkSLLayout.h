/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_LAYOUT
#define SKSL_LAYOUT

namespace SkSL {

/**
 * Represents a layout block appearing before a variable declaration, as in:
 *
 * layout (location = 0) int x;
 */
struct Layout {
    Layout(const ASTLayout& layout)
    : fLocation(layout.fLocation)
    , fBinding(layout.fBinding)
    , fIndex(layout.fIndex)
    , fSet(layout.fSet)
    , fBuiltin(layout.fBuiltin)
    , fInputAttachmentIndex(layout.fInputAttachmentIndex)
    , fOriginUpperLeft(layout.fOriginUpperLeft)
    , fOverrideCoverage(layout.fOverrideCoverage)
    , fBlendSupportAllEquations(layout.fBlendSupportAllEquations)
    , fFormat(layout.fFormat)
    , fPushConstant(layout.fPushConstant) {}

    Layout(int location, int binding, int index, int set, int builtin, int inputAttachmentIndex,
           bool originUpperLeft, bool overrideCoverage, bool blendSupportAllEquations,
           ASTLayout::Format format, bool pushconstant)
    : fLocation(location)
    , fBinding(binding)
    , fIndex(index)
    , fSet(set)
    , fBuiltin(builtin)
    , fInputAttachmentIndex(inputAttachmentIndex)
    , fOriginUpperLeft(originUpperLeft)
    , fOverrideCoverage(overrideCoverage)
    , fBlendSupportAllEquations(blendSupportAllEquations)
    , fFormat(format)
    , fPushConstant(pushconstant) {}

    Layout()
    : fLocation(-1)
    , fBinding(-1)
    , fIndex(-1)
    , fSet(-1)
    , fBuiltin(-1)
    , fInputAttachmentIndex(-1)
    , fOriginUpperLeft(false)
    , fOverrideCoverage(false)
    , fBlendSupportAllEquations(false)
    , fFormat(ASTLayout::Format::kUnspecified)
    , fPushConstant(false) {}

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
        if (fInputAttachmentIndex >= 0) {
            result += separator + "input_attachment_index = " + to_string(fBuiltin);
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
        if (ASTLayout::Format::kUnspecified != fFormat) {
            result += separator + ASTLayout::FormatToStr(fFormat);
            separator = ", ";
        }
        if (fPushConstant) {
            result += separator + "push_constant";
            separator = ", ";
        }
        if (result.size() > 0) {
            result = "layout (" + result + ")";
        }
        return result;
    }

    bool operator==(const Layout& other) const {
        return fLocation                 == other.fLocation &&
               fBinding                  == other.fBinding &&
               fIndex                    == other.fIndex &&
               fSet                      == other.fSet &&
               fBuiltin                  == other.fBuiltin &&
               fInputAttachmentIndex     == other.fInputAttachmentIndex &&
               fOriginUpperLeft          == other.fOriginUpperLeft &&
               fOverrideCoverage         == other.fOverrideCoverage &&
               fBlendSupportAllEquations == other.fBlendSupportAllEquations &&
               fFormat                   == other.fFormat;
    }

    bool operator!=(const Layout& other) const {
        return !(*this == other);
    }

    int fLocation;
    int fBinding;
    int fIndex;
    int fSet;
    // builtin comes from SPIR-V and identifies which particular builtin value this object
    // represents.
    int fBuiltin;
    // input_attachment_index comes from Vulkan/SPIR-V to connect a shader variable to the a
    // corresponding attachment on the subpass in which the shader is being used.
    int fInputAttachmentIndex;
    bool fOriginUpperLeft;
    bool fOverrideCoverage;
    bool fBlendSupportAllEquations;
    ASTLayout::Format fFormat;
    bool fPushConstant;
};

} // namespace

#endif
