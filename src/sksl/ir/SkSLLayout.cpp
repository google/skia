/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSLLayout.h"
#include "include/private/SkSLString.h"

namespace SkSL {

std::string Layout::description() const {
    std::string result;
    auto separator = SkSL::String::Separator();
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
        result += separator() + "input_attachment_index = " +
                  std::to_string(fInputAttachmentIndex);
    }
    if (fFlags & kOriginUpperLeft_Flag) {
        result += separator() + "origin_upper_left";
    }
    if (fFlags & kBlendSupportAllEquations_Flag) {
        result += separator() + "blend_support_all_equations";
    }
    if (fFlags & kPushConstant_Flag) {
        result += separator() + "push_constant";
    }
    if (fFlags & kColor_Flag) {
        result += separator() + "color";
    }
    if (result.size() > 0) {
        result = "layout (" + result + ")";
    }
    return result;
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
           fInputAttachmentIndex == other.fInputAttachmentIndex;
}

}  // namespace SkSL
