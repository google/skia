/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkBlendModeColorFilter_DEFINED
#define SkBlendModeColorFilter_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkFlattenable.h"
#include "src/effects/colorfilters/SkColorFilterBase.h"

class SkReadBuffer;
class SkWriteBuffer;
enum class SkBlendMode;
struct SkStageRec;

class SkBlendModeColorFilter final : public SkColorFilterBase {
public:
    SkBlendModeColorFilter(const SkColor4f& color, SkBlendMode mode);

    bool appendStages(const SkStageRec& rec, bool shaderIsOpaque) const override;

    bool onIsAlphaUnchanged() const override;

    SkColorFilterBase::Type type() const override { return SkColorFilterBase::Type::kBlendMode; }

    SkColor4f color() const { return fColor; }
    SkBlendMode mode() const { return fMode; }

private:
    friend void ::SkRegisterModeColorFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkBlendModeColorFilter)

    void flatten(SkWriteBuffer&) const override;
    bool onAsAColorMode(SkColor*, SkBlendMode*) const override;

    SkColor4f fColor;  // always stored in sRGB
    SkBlendMode fMode;
};

#endif
