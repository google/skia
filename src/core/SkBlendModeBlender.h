/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlendModeBlender_DEFINED
#define SkBlendModeBlender_DEFINED

#include "include/core/SkFlattenable.h"
#include "src/core/SkBlenderBase.h"

#include <optional>

class SkReadBuffer;
class SkWriteBuffer;
enum class SkBlendMode;
struct SkStageRec;

class SkBlendModeBlender : public SkBlenderBase {
public:
    SkBlendModeBlender(SkBlendMode mode) : fMode(mode) {}

    BlenderType type() const override { return BlenderType::kBlendMode; }
    SkBlendMode mode() const { return fMode; }

    SK_FLATTENABLE_HOOKS(SkBlendModeBlender)

private:
    using INHERITED = SkBlenderBase;

    std::optional<SkBlendMode> asBlendMode() const final { return fMode; }

    void flatten(SkWriteBuffer& buffer) const override;

    bool onAppendStages(const SkStageRec& rec) const override;

    SkBlendMode fMode;
};

#endif
