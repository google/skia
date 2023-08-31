/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLLayout.h"

#include "include/private/base/SkAssert.h"
#include "src/base/SkMathPriv.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/SkSLString.h"

namespace SkSL {

std::string Layout::paddedDescription() const {
    std::string result;
    auto separator = SkSL::String::Separator();
    if (fFlags & LayoutFlag::kVulkan) {
        result += separator() + "vulkan";
    }
    if (fFlags & LayoutFlag::kMetal) {
        result += separator() + "metal";
    }
    if (fFlags & LayoutFlag::kWebGPU) {
        result += separator() + "webgpu";
    }
    if (fFlags & LayoutFlag::kDirect3D) {
        result += separator() + "direct3d";
    }
    if (fFlags & LayoutFlag::kRGBA8) {
        result += separator() + "rgba8";
    }
    if (fFlags & LayoutFlag::kRGBA32F) {
        result += separator() + "rgba32f";
    }
    if (fFlags & LayoutFlag::kR32F) {
        result += separator() + "r32f";
    }
    if (fLocation >= 0) {
        result += separator() + "location = " + std::to_string(fLocation);
    }
    if (fOffset >= 0) {
        result += separator() + "offset = " + std::to_string(fOffset);
    }
    if (fBinding >= 0) {
        result += separator() + "binding = " + std::to_string(fBinding);
    }
    if (fTexture >= 0) {
        result += separator() + "texture = " + std::to_string(fTexture);
    }
    if (fSampler >= 0) {
        result += separator() + "sampler = " + std::to_string(fSampler);
    }
    if (fIndex >= 0) {
        result += separator() + "index = " + std::to_string(fIndex);
    }
    if (fSet >= 0) {
        result += separator() + "set = " + std::to_string(fSet);
    }
    if (fBuiltin >= 0) {
        result += separator() + "builtin = " + std::to_string(fBuiltin);
    }
    if (fInputAttachmentIndex >= 0) {
        result += separator() + "input_attachment_index = " + std::to_string(fInputAttachmentIndex);
    }
    if (fFlags & LayoutFlag::kOriginUpperLeft) {
        result += separator() + "origin_upper_left";
    }
    if (fFlags & LayoutFlag::kBlendSupportAllEquations) {
        result += separator() + "blend_support_all_equations";
    }
    if (fFlags & LayoutFlag::kPushConstant) {
        result += separator() + "push_constant";
    }
    if (fFlags & LayoutFlag::kColor) {
        result += separator() + "color";
    }
    if (fLocalSizeX >= 0) {
        result += separator() + "local_size_x = " + std::to_string(fLocalSizeX);
    }
    if (fLocalSizeY >= 0) {
        result += separator() + "local_size_y = " + std::to_string(fLocalSizeY);
    }
    if (fLocalSizeZ >= 0) {
        result += separator() + "local_size_z = " + std::to_string(fLocalSizeZ);
    }
    if (result.size() > 0) {
        result = "layout (" + result + ") ";
    }
    return result;
}

std::string Layout::description() const {
    std::string s = this->paddedDescription();
    if (!s.empty()) {
        s.pop_back();
    }
    return s;
}

bool Layout::checkPermittedLayout(const Context& context,
                                  Position pos,
                                  LayoutFlags permittedLayoutFlags) const {
    static constexpr struct { LayoutFlag flag; const char* name; } kLayoutFlags[] = {
        { LayoutFlag::kOriginUpperLeft,          "origin_upper_left"},
        { LayoutFlag::kPushConstant,             "push_constant"},
        { LayoutFlag::kBlendSupportAllEquations, "blend_support_all_equations"},
        { LayoutFlag::kColor,                    "color"},
        { LayoutFlag::kLocation,                 "location"},
        { LayoutFlag::kOffset,                   "offset"},
        { LayoutFlag::kBinding,                  "binding"},
        { LayoutFlag::kTexture,                  "texture"},
        { LayoutFlag::kSampler,                  "sampler"},
        { LayoutFlag::kIndex,                    "index"},
        { LayoutFlag::kSet,                      "set"},
        { LayoutFlag::kBuiltin,                  "builtin"},
        { LayoutFlag::kInputAttachmentIndex,     "input_attachment_index"},
        { LayoutFlag::kVulkan,                   "vulkan"},
        { LayoutFlag::kMetal,                    "metal"},
        { LayoutFlag::kWebGPU,                   "webgpu"},
        { LayoutFlag::kDirect3D,                 "direct3d"},
        { LayoutFlag::kRGBA8,                    "rgba8"},
        { LayoutFlag::kRGBA32F,                  "rgba32f"},
        { LayoutFlag::kR32F,                     "r32f"},
        { LayoutFlag::kLocalSizeX,               "local_size_x"},
        { LayoutFlag::kLocalSizeY,               "local_size_y"},
        { LayoutFlag::kLocalSizeZ,               "local_size_z"},
    };

    bool success = true;
    LayoutFlags layoutFlags = fFlags;

    LayoutFlags backendFlags = layoutFlags & LayoutFlag::kAllBackends;
    if (SkPopCount(backendFlags.value()) > 1) {
        context.fErrors->error(pos, "only one backend qualifier can be used");
        success = false;
    }

    LayoutFlags pixelFormatFlags = layoutFlags & LayoutFlag::kAllPixelFormats;
    if (SkPopCount(pixelFormatFlags.value()) > 1) {
        context.fErrors->error(pos, "only one pixel format qualifier can be used");
        success = false;
    }

    if ((layoutFlags & (LayoutFlag::kTexture | LayoutFlag::kSampler)) &&
        layoutFlags & LayoutFlag::kBinding) {
        context.fErrors->error(pos, "'binding' modifier cannot coexist with 'texture'/'sampler'");
        success = false;
    }
    // The `texture` and `sampler` flags are only allowed when targeting Metal, WebGPU or Direct3D.
    if (!(layoutFlags & (LayoutFlag::kMetal | LayoutFlag::kWebGPU | LayoutFlag::kDirect3D))) {
        permittedLayoutFlags &= ~LayoutFlag::kTexture;
        permittedLayoutFlags &= ~LayoutFlag::kSampler;
    }
    // The `push_constant` flag is only allowed when targeting Vulkan or WebGPU.
    if (!(layoutFlags & (LayoutFlag::kVulkan | LayoutFlag::kWebGPU))) {
        permittedLayoutFlags &= ~LayoutFlag::kPushConstant;
    }
    // The `set` flag is not allowed when explicitly targeting Metal.
    if (layoutFlags & LayoutFlag::kMetal) {
        permittedLayoutFlags &= ~LayoutFlag::kSet;
    }

    for (const auto& lf : kLayoutFlags) {
        if (layoutFlags & lf.flag) {
            if (!(permittedLayoutFlags & lf.flag)) {
                context.fErrors->error(pos, "layout qualifier '" + std::string(lf.name) +
                                            "' is not permitted here");
                success = false;
            }
            layoutFlags &= ~lf.flag;
        }
    }
    SkASSERT(layoutFlags == LayoutFlag::kNone);
    return success;
}

bool Layout::operator==(const Layout& other) const {
    return fFlags                == other.fFlags &&
           fLocation             == other.fLocation &&
           fOffset               == other.fOffset &&
           fBinding              == other.fBinding &&
           fTexture              == other.fTexture &&
           fSampler              == other.fSampler &&
           fIndex                == other.fIndex &&
           fSet                  == other.fSet &&
           fBuiltin              == other.fBuiltin &&
           fInputAttachmentIndex == other.fInputAttachmentIndex &&
           fLocalSizeX           == other.fLocalSizeX &&
           fLocalSizeY           == other.fLocalSizeY &&
           fLocalSizeZ           == other.fLocalSizeZ;
}

}  // namespace SkSL
