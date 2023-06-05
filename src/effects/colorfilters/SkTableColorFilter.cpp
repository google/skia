/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/effects/colorfilters/SkTableColorFilter.h"

#include "include/core/SkColorFilter.h"
#include "include/core/SkColorTable.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "src/effects/colorfilters/SkColorFilterBase.h"

#include <cstdint>
#include <utility>

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

bool SkTableColorFilter::appendStages(const SkStageRec& rec, bool shaderIsOpaque) const {
    SkRasterPipeline* p = rec.fPipeline;
    if (!shaderIsOpaque) {
        p->append(SkRasterPipelineOp::unpremul);
    }

    SkRasterPipeline_TablesCtx* tables = rec.fAlloc->make<SkRasterPipeline_TablesCtx>();
    tables->a = fTable->alphaTable();
    tables->r = fTable->redTable();
    tables->g = fTable->greenTable();
    tables->b = fTable->blueTable();
    p->append(SkRasterPipelineOp::byte_tables, tables);

    bool definitelyOpaque = shaderIsOpaque && tables->a[0xff] == 0xff;
    if (!definitelyOpaque) {
        p->append(SkRasterPipelineOp::premul);
    }
    return true;
}

#if defined(SK_ENABLE_SKVM)
skvm::Color SkTableColorFilter::onProgram(skvm::Builder* p,
                                          skvm::Color c,
                                          const SkColorInfo& dst,
                                          skvm::Uniforms* uniforms,
                                          SkArenaAlloc*) const {
    auto apply_table_to_component = [&](skvm::F32 c, const uint8_t* bytePtr) -> skvm::F32 {
        skvm::I32 index = to_unorm(8, clamp01(c));
        skvm::Uniform table = uniforms->pushPtr(bytePtr);
        return from_unorm(8, gather8(table, index));
    };

    c = unpremul(c);
    c.a = apply_table_to_component(c.a, fTable->alphaTable());
    c.r = apply_table_to_component(c.r, fTable->redTable());
    c.g = apply_table_to_component(c.g, fTable->greenTable());
    c.b = apply_table_to_component(c.b, fTable->blueTable());
    return premul(c);
}
#endif

void SkTableColorFilter::flatten(SkWriteBuffer& buffer) const {
    fTable->flatten(buffer);
}

sk_sp<SkFlattenable> SkTableColorFilter::CreateProc(SkReadBuffer& buffer) {
    return SkColorFilters::Table(SkColorTable::Deserialize(buffer));
}

#if defined(SK_GRAPHITE)

void SkTableColorFilter::addToKey(const skgpu::graphite::KeyContext& keyContext,
                                  skgpu::graphite::PaintParamsKeyBuilder* builder,
                                  skgpu::graphite::PipelineDataGatherer* gatherer) const {
    using namespace skgpu::graphite;

    sk_sp<TextureProxy> proxy = RecorderPriv::CreateCachedProxy(keyContext.recorder(),
                                                                this->bitmap());
    if (!proxy) {
        SKGPU_LOG_W("Couldn't create TableColorFilter's table");

        // Return the input color as-is.
        PriorOutputBlock::BeginBlock(keyContext, builder, gatherer);
        builder->endBlock();
        return;
    }

    TableColorFilterBlock::TableColorFilterData data(std::move(proxy));

    TableColorFilterBlock::BeginBlock(keyContext, builder, gatherer, data);
    builder->endBlock();
}

#endif

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkColorFilter> SkColorFilters::Table(const uint8_t table[256]) {
    return SkColorFilters::Table(SkColorTable::Make(table));
}

sk_sp<SkColorFilter> SkColorFilters::TableARGB(const uint8_t tableA[256],
                                               const uint8_t tableR[256],
                                               const uint8_t tableG[256],
                                               const uint8_t tableB[256]) {
    return SkColorFilters::Table(SkColorTable::Make(tableA, tableR, tableG, tableB));
}

sk_sp<SkColorFilter> SkColorFilters::Table(sk_sp<SkColorTable> table) {
    if (!table) {
        return nullptr;
    }
    return sk_make_sp<SkTableColorFilter>(std::move(table));
}

void SkRegisterTableColorFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkTableColorFilter);
    // Previous name
    SkFlattenable::Register("SkTable_ColorFilter", SkTableColorFilter::CreateProc);
}
