/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/effects/SkTableColorFilter.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkColorFilterBase.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

#include <cstdint>

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

SkTableColorFilter::SkTableColorFilter(const uint8_t tableA[],
                                       const uint8_t tableR[],
                                       const uint8_t tableG[],
                                       const uint8_t tableB[]) {
    fBitmap.allocPixels(SkImageInfo::MakeA8(256, 4));
    uint8_t *a = fBitmap.getAddr8(0, 0), *r = fBitmap.getAddr8(0, 1), *g = fBitmap.getAddr8(0, 2),
            *b = fBitmap.getAddr8(0, 3);
    for (int i = 0; i < 256; i++) {
        a[i] = tableA ? tableA[i] : i;
        r[i] = tableR ? tableR[i] : i;
        g[i] = tableG ? tableG[i] : i;
        b[i] = tableB ? tableB[i] : i;
    }
    fBitmap.setImmutable();
}

bool SkTableColorFilter::appendStages(const SkStageRec& rec, bool shaderIsOpaque) const {
    SkRasterPipeline* p = rec.fPipeline;
    if (!shaderIsOpaque) {
        p->append(SkRasterPipelineOp::unpremul);
    }

    SkRasterPipeline_TablesCtx* tables = rec.fAlloc->make<SkRasterPipeline_TablesCtx>();
    tables->a = fBitmap.getAddr8(0, 0);
    tables->r = fBitmap.getAddr8(0, 1);
    tables->g = fBitmap.getAddr8(0, 2);
    tables->b = fBitmap.getAddr8(0, 3);
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
    c.a = apply_table_to_component(c.a, fBitmap.getAddr8(0, 0));
    c.r = apply_table_to_component(c.r, fBitmap.getAddr8(0, 1));
    c.g = apply_table_to_component(c.g, fBitmap.getAddr8(0, 2));
    c.b = apply_table_to_component(c.b, fBitmap.getAddr8(0, 3));
    return premul(c);
}
#endif

void SkTableColorFilter::flatten(SkWriteBuffer& buffer) const {
    buffer.writeByteArray(fBitmap.getAddr8(0, 0), 4 * 256);
}

sk_sp<SkFlattenable> SkTableColorFilter::CreateProc(SkReadBuffer& buffer) {
    uint8_t argb[4*256];
    if (buffer.readByteArray(argb, sizeof(argb))) {
        return SkColorFilters::TableARGB(argb+0*256, argb+1*256, argb+2*256, argb+3*256);
    }
    return nullptr;
}

#if defined(SK_GRAPHITE)

void SkTableColorFilter::addToKey(const skgpu::graphite::KeyContext& keyContext,
                                  skgpu::graphite::PaintParamsKeyBuilder* builder,
                                  skgpu::graphite::PipelineDataGatherer* gatherer) const {
    using namespace skgpu::graphite;

    sk_sp<TextureProxy> proxy = RecorderPriv::CreateCachedProxy(keyContext.recorder(), fBitmap);
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
    return sk_make_sp<SkTableColorFilter>(table, table, table, table);
}

sk_sp<SkColorFilter> SkColorFilters::TableARGB(const uint8_t tableA[256],
                                               const uint8_t tableR[256],
                                               const uint8_t tableG[256],
                                               const uint8_t tableB[256]) {
    if (!tableA && !tableR && !tableG && !tableB) {
        return nullptr;
    }

    return sk_make_sp<SkTableColorFilter>(tableA, tableR, tableG, tableB);
}

void SkRegisterTableColorFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkTableColorFilter);
    // Previous name
    SkFlattenable::Register("SkTable_ColorFilter", SkTableColorFilter::CreateProc);
}
