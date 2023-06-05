/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkTableColorFilter_DEFINED
#define SkTableColorFilter_DEFINED

#include "include/core/SkColorTable.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "src/effects/colorfilters/SkColorFilterBase.h"

class SkBitmap;
class SkReadBuffer;
class SkWriteBuffer;
struct SkStageRec;

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/RecorderPriv.h"

namespace skgpu::graphite {
class PipelineDataGatherer;
}
#endif

#if defined(SK_ENABLE_SKSL) && defined(SK_ENABLE_SKVM)
#include "src/core/SkVM.h"
#endif

class SkTableColorFilter final : public SkColorFilterBase {
public:
    SkTableColorFilter(sk_sp<SkColorTable> table) : fTable(table) {
        SkASSERT(fTable);
    }

    SkColorFilterBase::Type type() const override { return SkColorFilterBase::Type::kTable; }

#if defined(SK_GRAPHITE)
    void addToKey(const skgpu::graphite::KeyContext&,
                  skgpu::graphite::PaintParamsKeyBuilder*,
                  skgpu::graphite::PipelineDataGatherer*) const override;
#endif

    bool appendStages(const SkStageRec& rec, bool shaderIsOpaque) const override;

#if defined(SK_ENABLE_SKVM)
    skvm::Color onProgram(skvm::Builder* p,
                          skvm::Color c,
                          const SkColorInfo& dst,
                          skvm::Uniforms* uniforms,
                          SkArenaAlloc*) const override;
#endif

    void flatten(SkWriteBuffer& buffer) const override;

    const SkBitmap& bitmap() const { return fTable->bitmap(); }

private:
    friend void ::SkRegisterTableColorFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkTableColorFilter)

    sk_sp<SkColorTable> fTable;
};

#endif
