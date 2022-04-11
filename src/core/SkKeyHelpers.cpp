/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkKeyHelpers.h"

#include "src/core/SkDebugUtils.h"
#include "src/core/SkKeyContext.h"
#include "src/core/SkPaintParamsKey.h"
#include "src/core/SkPipelineData.h"
#include "src/core/SkShaderCodeDictionary.h"
#include "src/core/SkUniform.h"
#include "src/shaders/SkShaderBase.h"

#ifdef SK_GRAPHITE_ENABLED
#include "src/gpu/Blend.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/UniformManager.h"
#endif

#define VALIDATE_UNIFORMS(gatherer, dict, codeSnippetID) \
    SkDEBUGCODE(UniformExpectationsValidator uev( \
        gatherer, dict->getUniforms(SkBuiltInCodeSnippetID::codeSnippetID));)

constexpr SkPMColor4f kErrorColor = { 1, 0, 0, 1 };

namespace {

// This can be used to catch errors in blocks that have a fixed, known block data size
void validate_block_header(const SkPaintParamsKeyBuilder* builder,
                           SkBuiltInCodeSnippetID codeSnippetID,
                           int blockDataSize) {
    SkDEBUGCODE(int fullBlockSize = SkPaintParamsKey::kBlockHeaderSizeInBytes + blockDataSize;)
    SkDEBUGCODE(int headerOffset = builder->sizeInBytes() - fullBlockSize;)
    SkASSERT(builder->byte(headerOffset) == static_cast<int>(codeSnippetID));
    SkASSERT(builder->byte(headerOffset+SkPaintParamsKey::kBlockSizeOffsetInBytes) ==
             fullBlockSize);
}

#ifdef SK_GRAPHITE_ENABLED
void add_blendmode_to_key(SkPaintParamsKeyBuilder* builder, SkBlendMode bm) {
    static_assert(SkTFitsIn<uint8_t>(static_cast<int>(SkBlendMode::kLastMode)));
    builder->addByte(static_cast<uint8_t>(bm));
}

#endif // SK_GRAPHITE_ENABLED

} // anonymous namespace

//--------------------------------------------------------------------------------------------------
namespace DepthStencilOnlyBlock {

static const int kBlockDataSize = 0;

void AddToKey(const SkKeyContext& /* keyContext */,
              SkPaintParamsKeyBuilder* builder,
              SkPipelineDataGatherer* /* gatherer */) {
    builder->beginBlock(SkBuiltInCodeSnippetID::kDepthStencilOnlyDraw);
    builder->endBlock();

    validate_block_header(builder,
                          SkBuiltInCodeSnippetID::kDepthStencilOnlyDraw,
                          kBlockDataSize);
}

} // namespace DepthStencilOnlyBlock

//--------------------------------------------------------------------------------------------------
namespace SolidColorShaderBlock {

namespace {

#ifdef SK_GRAPHITE_ENABLED
static const int kBlockDataSize = 0;

void add_solid_uniform_data(const SkShaderCodeDictionary* dict,
                            const SkPMColor4f& premulColor,
                            SkPipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, kSolidColorShader)
    gatherer->write(premulColor);
}
#endif // SK_GRAPHITE_ENABLED

} // anonymous namespace

void AddToKey(const SkKeyContext& keyContext,
              SkPaintParamsKeyBuilder* builder,
              SkPipelineDataGatherer* gatherer,
              const SkPMColor4f& premulColor) {

#ifdef SK_GRAPHITE_ENABLED
    if (builder->backend() == SkBackend::kGraphite) {
        auto dict = keyContext.dict();

        builder->beginBlock(SkBuiltInCodeSnippetID::kSolidColorShader);
        builder->endBlock();

        validate_block_header(builder,
                              SkBuiltInCodeSnippetID::kSolidColorShader,
                              kBlockDataSize);

        if (gatherer) {
            add_solid_uniform_data(dict, premulColor, gatherer);
        }
        return;
    }
#endif // SK_GRAPHITE_ENABLED

    if (builder->backend() == SkBackend::kSkVM || builder->backend() == SkBackend::kGanesh) {
        // TODO: add implementation of other backends
    }

}

} // namespace SolidColorShaderBlock

//--------------------------------------------------------------------------------------------------
namespace GradientShaderBlocks {

namespace {

#ifdef SK_GRAPHITE_ENABLED
static const int kBlockDataSize = 1;

void add_linear_gradient_uniform_data(const SkShaderCodeDictionary* dict,
                                      const GradientData& gradData,
                                      SkPipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, kLinearGradientShader)
    gatherer->write(gradData.fColor4fs, GradientData::kMaxStops);
    gatherer->write(gradData.fOffsets, GradientData::kMaxStops);
    gatherer->write(gradData.fPoints[0]);
    gatherer->write(gradData.fPoints[1]);
    gatherer->write(gradData.fRadii[0]);        // unused
    gatherer->write(gradData.fRadii[1]);        // unused
    gatherer->write(SkPoint::Make(0.0f, 0.0f)); // padding
};

void add_radial_gradient_uniform_data(const SkShaderCodeDictionary* dict,
                                      const GradientData& gradData,
                                      SkPipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, kRadialGradientShader)
    gatherer->write(gradData.fColor4fs, GradientData::kMaxStops);
    gatherer->write(gradData.fOffsets, GradientData::kMaxStops);
    gatherer->write(gradData.fPoints[0]);
    gatherer->write(gradData.fPoints[1]);       // unused
    gatherer->write(gradData.fRadii[0]);
    gatherer->write(gradData.fRadii[1]);        // unused
    gatherer->write(SkPoint::Make(0.0f, 0.0f)); // padding
};

void add_sweep_gradient_uniform_data(const SkShaderCodeDictionary* dict,
                                     const GradientData& gradData,
                                     SkPipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, kSweepGradientShader)
    gatherer->write(gradData.fColor4fs, GradientData::kMaxStops);
    gatherer->write(gradData.fOffsets, GradientData::kMaxStops);
    gatherer->write(gradData.fPoints[0]);
    gatherer->write(gradData.fPoints[1]);       // unused
    gatherer->write(gradData.fRadii[0]);        // unused
    gatherer->write(gradData.fRadii[1]);        // unused
    gatherer->write(SkPoint::Make(0.0f, 0.0f)); // padding
};

void add_conical_gradient_uniform_data(const SkShaderCodeDictionary* dict,
                                       const GradientData& gradData,
                                       SkPipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, kConicalGradientShader)
    gatherer->write(gradData.fColor4fs, GradientData::kMaxStops);
    gatherer->write(gradData.fOffsets, GradientData::kMaxStops);
    gatherer->write(gradData.fPoints[0]);
    gatherer->write(gradData.fPoints[1]);
    gatherer->write(gradData.fRadii[0]);
    gatherer->write(gradData.fRadii[1]);
    gatherer->write(SkPoint::Make(0.0f, 0.0f)); // padding
};

#endif // SK_GRAPHITE_ENABLED

} // anonymous namespace

GradientData::GradientData(SkShader::GradientType type,
                           SkTileMode tm,
                           int numStops)
        : fType(type)
        , fPoints{{0.0f, 0.0f}, {0.0f, 0.0f}}
        , fRadii{0.0f, 0.0f}
        , fTM(tm)
        , fNumStops(numStops) {
    sk_bzero(fColor4fs, sizeof(fColor4fs));
    sk_bzero(fOffsets, sizeof(fOffsets));
}

GradientData::GradientData(SkShader::GradientType type,
                           SkPoint point0, SkPoint point1,
                           float radius0, float radius1,
                           SkTileMode tm,
                           int numStops,
                           SkColor4f* color4fs,
                           float* offsets)
        : fType(type)
        , fTM(tm)
        , fNumStops(std::min(numStops, kMaxStops)) {
    SkASSERT(fNumStops >= 1);

    fPoints[0] = point0;
    fPoints[1] = point1;
    fRadii[0] = radius0;
    fRadii[1] = radius1;
    memcpy(fColor4fs, color4fs, fNumStops * sizeof(SkColor4f));
    if (offsets) {
        memcpy(fOffsets, offsets, fNumStops * sizeof(float));
    } else {
        for (int i = 0; i < fNumStops; ++i) {
            fOffsets[i] = SkIntToFloat(i) / (fNumStops-1);
        }
    }

    // Extend the colors and offset, if necessary, to fill out the arrays
    // TODO: this should be done later when the actual code snippet has been selected!!
    for (int i = fNumStops ; i < kMaxStops; ++i) {
        fColor4fs[i] = fColor4fs[fNumStops-1];
        fOffsets[i] = fOffsets[fNumStops-1];
    }
}

void AddToKey(const SkKeyContext& keyContext,
              SkPaintParamsKeyBuilder *builder,
              SkPipelineDataGatherer* gatherer,
              const GradientData& gradData) {

#ifdef SK_GRAPHITE_ENABLED
    if (builder->backend() == SkBackend::kGraphite) {
        auto dict = keyContext.dict();
        SkBuiltInCodeSnippetID codeSnippetID = SkBuiltInCodeSnippetID::kSolidColorShader;
        switch (gradData.fType) {
            case SkShader::kLinear_GradientType:
                codeSnippetID = SkBuiltInCodeSnippetID::kLinearGradientShader;
                if (gatherer) {
                    add_linear_gradient_uniform_data(dict, gradData, gatherer);
                }
                break;
            case SkShader::kRadial_GradientType:
                codeSnippetID = SkBuiltInCodeSnippetID::kRadialGradientShader;
                if (gatherer) {
                    add_radial_gradient_uniform_data(dict, gradData, gatherer);
                }
                break;
            case SkShader::kSweep_GradientType:
                codeSnippetID = SkBuiltInCodeSnippetID::kSweepGradientShader;
                if (gatherer) {
                    add_sweep_gradient_uniform_data(dict, gradData, gatherer);
                }
                break;
            case SkShader::GradientType::kConical_GradientType:
                codeSnippetID = SkBuiltInCodeSnippetID::kConicalGradientShader;
                if (gatherer) {
                    add_conical_gradient_uniform_data(dict, gradData, gatherer);
                }
                break;
            case SkShader::GradientType::kColor_GradientType:
            case SkShader::GradientType::kNone_GradientType:
            default:
                SkASSERT(0);
                break;
        }

        builder->beginBlock(codeSnippetID);

        SkASSERT(static_cast<int>(gradData.fTM) <= std::numeric_limits<uint8_t>::max());
        builder->addByte(static_cast<uint8_t>(gradData.fTM));

        builder->endBlock();

        validate_block_header(builder, codeSnippetID, kBlockDataSize);
        return;
    }
#endif // SK_GRAPHITE_ENABLED

    if (builder->backend() == SkBackend::kSkVM || builder->backend() == SkBackend::kGanesh) {
        // TODO: add implementation of other backends
        SolidColorShaderBlock::AddToKey(keyContext, builder, gatherer, kErrorColor);
    }
}

} // namespace GradientShaderBlocks

//--------------------------------------------------------------------------------------------------
namespace ImageShaderBlock {

namespace {

#ifdef SK_GRAPHITE_ENABLED

void add_image_uniform_data(const SkShaderCodeDictionary* dict,
                            const ImageData& imgData,
                            SkPipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, kImageShader)
    gatherer->write(imgData.fSubset);
}

#endif // SK_GRAPHITE_ENABLED

} // anonymous namespace

ImageData::ImageData(const SkSamplingOptions& sampling,
                     SkTileMode tileModeX,
                     SkTileMode tileModeY,
                     SkRect subset)
    : fSampling(sampling)
    , fTileModes{tileModeX, tileModeY}
    , fSubset(subset) {
}

void AddToKey(const SkKeyContext& keyContext,
              SkPaintParamsKeyBuilder* builder,
              SkPipelineDataGatherer* gatherer,
              const ImageData& imgData) {

#ifdef SK_GRAPHITE_ENABLED
    if (builder->backend() == SkBackend::kGraphite) {
        // TODO: allow through lazy proxies
        if (gatherer && !imgData.fTextureProxy) {
            // We're dropping the ImageShader here. This could be an instance of trying to draw
            // a raster-backed image w/ a Graphite-backed canvas.
            // TODO: At some point the pre-compile path should also be creating a texture
            // proxy (i.e., we can remove the 'pipelineData' in the above test).
            SolidColorShaderBlock::AddToKey(keyContext, builder, gatherer, kErrorColor);
            return;
        }

        auto dict = keyContext.dict();
        builder->beginBlock(SkBuiltInCodeSnippetID::kImageShader);

        // TODO: bytes are overkill for just tilemodes. We could add smaller/bit-width
        // types.
        static_assert(SkTFitsIn<uint8_t>(SkTileMode::kLastTileMode));
        builder->addByte(static_cast<uint8_t>(imgData.fTileModes[0]));
        builder->addByte(static_cast<uint8_t>(imgData.fTileModes[1]));

        builder->endBlock();

        if (gatherer) {
            gatherer->add(imgData.fSampling,
                          imgData.fTileModes,
                          std::move(imgData.fTextureProxy));

            add_image_uniform_data(dict, imgData, gatherer);
        }

        return;
    }
#endif // SK_GRAPHITE_ENABLED

    if (builder->backend() == SkBackend::kSkVM || builder->backend() == SkBackend::kGanesh) {
        // TODO: add implementation for other backends
        SolidColorShaderBlock::AddToKey(keyContext, builder, gatherer, kErrorColor);
    }
}

} // namespace ImageShaderBlock

//--------------------------------------------------------------------------------------------------
namespace BlendShaderBlock {

namespace {

#ifdef SK_GRAPHITE_ENABLED

void add_blendshader_uniform_data(const SkShaderCodeDictionary* dict,
                                  SkBlendMode bm,
                                  SkPipelineDataGatherer* gatherer) {
    VALIDATE_UNIFORMS(gatherer, dict, kBlendShader)
    gatherer->write(SkTo<int>(bm));
    gatherer->write(0); // padding - remove
    gatherer->write(0); // padding - remove
    gatherer->write(0); // padding - remove
}

#endif // SK_GRAPHITE_ENABLED

} // anonymous namespace

void AddToKey(const SkKeyContext& keyContext,
              SkPaintParamsKeyBuilder *builder,
              SkPipelineDataGatherer* gatherer,
              const BlendData& blendData) {

#ifdef SK_GRAPHITE_ENABLED
    if (builder->backend() == SkBackend::kGraphite) {
        auto dict = keyContext.dict();
        // When extracted into SkShaderInfo::SnippetEntries the children will appear after their
        // parent. Thus, the parent's uniform data must appear in the uniform block before the
        // uniform data of the children.
        if (gatherer) {
            add_blendshader_uniform_data(dict, blendData.fBM, gatherer);
        }

        builder->beginBlock(SkBuiltInCodeSnippetID::kBlendShader);

        // Child blocks always go right after the parent block's header
        // TODO: add startChild/endChild entry points to SkPaintParamsKeyBuilder. They could be
        // used to compute and store the number of children w/in a block's header.
        int start = builder->sizeInBytes();
        as_SB(blendData.fDst)->addToKey(keyContext, builder, gatherer);
        int firstShaderSize = builder->sizeInBytes() - start;

        start = builder->sizeInBytes();
        as_SB(blendData.fSrc)->addToKey(keyContext, builder, gatherer);
        int secondShaderSize = builder->sizeInBytes() - start;

        builder->endBlock();

        int expectedBlockSize = firstShaderSize + secondShaderSize;
        validate_block_header(builder,
                              SkBuiltInCodeSnippetID::kBlendShader,
                              expectedBlockSize);
        return;
    }
#endif // SK_GRAPHITE_ENABLED

    if (builder->backend() == SkBackend::kSkVM || builder->backend() == SkBackend::kGanesh) {
        // TODO: add implementation for other backends
        SolidColorShaderBlock::AddToKey(keyContext, builder, gatherer, kErrorColor);
    }
}

} // namespace BlendShaderBlock

//--------------------------------------------------------------------------------------------------
#ifdef SK_GRAPHITE_ENABLED
namespace {

constexpr SkPipelineDataGatherer::BlendInfo make_simple_blendInfo(skgpu::BlendCoeff srcCoeff,
                                                                  skgpu::BlendCoeff dstCoeff) {
    return { skgpu::BlendEquation::kAdd,
             srcCoeff,
             dstCoeff,
             SK_PMColor4fTRANSPARENT,
             skgpu::BlendModifiesDst(skgpu::BlendEquation::kAdd, srcCoeff, dstCoeff) };
}

static constexpr int kNumCoeffModes = (int)SkBlendMode::kLastCoeffMode + 1;
/*>> No coverage, input color unknown <<*/
static constexpr SkPipelineDataGatherer::BlendInfo gBlendTable[kNumCoeffModes] = {
        /* clear */      make_simple_blendInfo(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kZero),
        /* src */        make_simple_blendInfo(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kZero),
        /* dst */        make_simple_blendInfo(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kOne),
        /* src-over */   make_simple_blendInfo(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kISA),
        /* dst-over */   make_simple_blendInfo(skgpu::BlendCoeff::kIDA,  skgpu::BlendCoeff::kOne),
        /* src-in */     make_simple_blendInfo(skgpu::BlendCoeff::kDA,   skgpu::BlendCoeff::kZero),
        /* dst-in */     make_simple_blendInfo(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kSA),
        /* src-out */    make_simple_blendInfo(skgpu::BlendCoeff::kIDA,  skgpu::BlendCoeff::kZero),
        /* dst-out */    make_simple_blendInfo(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kISA),
        /* src-atop */   make_simple_blendInfo(skgpu::BlendCoeff::kDA,   skgpu::BlendCoeff::kISA),
        /* dst-atop */   make_simple_blendInfo(skgpu::BlendCoeff::kIDA,  skgpu::BlendCoeff::kSA),
        /* xor */        make_simple_blendInfo(skgpu::BlendCoeff::kIDA,  skgpu::BlendCoeff::kISA),
        /* plus */       make_simple_blendInfo(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kOne),
        /* modulate */   make_simple_blendInfo(skgpu::BlendCoeff::kZero, skgpu::BlendCoeff::kSC),
        /* screen */     make_simple_blendInfo(skgpu::BlendCoeff::kOne,  skgpu::BlendCoeff::kISC)
};

const SkPipelineDataGatherer::BlendInfo& get_blend_info(SkBlendMode bm) {
    if (bm <= SkBlendMode::kLastCoeffMode) {
        return gBlendTable[(int) bm];
    }

    return gBlendTable[(int) SkBlendMode::kSrc];
}

} // anonymous namespace
#endif // SK_GRAPHITE_ENABLED

namespace BlendModeBlock {

#ifdef SK_GRAPHITE_ENABLED
static const int kFixedFunctionBlockDataSize = 1;
static const int kShaderBasedBlockDataSize = 1;
#endif

void AddToKey(const SkKeyContext& keyContext,
              SkPaintParamsKeyBuilder *builder,
              SkPipelineDataGatherer* gatherer,
              SkBlendMode bm) {

#ifdef SK_GRAPHITE_ENABLED
    if (builder->backend() == SkBackend::kGraphite) {
        if (bm <= SkBlendMode::kLastCoeffMode) {
            builder->beginBlock(SkBuiltInCodeSnippetID::kFixedFunctionBlender);
            add_blendmode_to_key(builder, bm);
            builder->endBlock();

            validate_block_header(builder,
                                  SkBuiltInCodeSnippetID::kFixedFunctionBlender,
                                  kFixedFunctionBlockDataSize);

            if (gatherer) {
                gatherer->setBlendInfo(get_blend_info(bm));
            }
        } else {
            builder->beginBlock(SkBuiltInCodeSnippetID::kShaderBasedBlender);
            add_blendmode_to_key(builder, bm);
            builder->endBlock();

            validate_block_header(builder,
                                  SkBuiltInCodeSnippetID::kShaderBasedBlender,
                                  kShaderBasedBlockDataSize);

            if (gatherer) {
                // TODO: set up the correct blend info
                gatherer->setBlendInfo(SkPipelineDataGatherer::BlendInfo());
            }
        }
        return;
    }
#endif// SK_GRAPHITE_ENABLED

    if (builder->backend() == SkBackend::kSkVM || builder->backend() == SkBackend::kGanesh) {
        // TODO: add implementation for other backends
        SolidColorShaderBlock::AddToKey(keyContext, builder, gatherer, kErrorColor);
    }
}

} // namespace BlendModeBlock

//--------------------------------------------------------------------------------------------------
#ifdef SK_GRAPHITE_ENABLED
SkUniquePaintParamsID CreateKey(const SkKeyContext& keyContext,
                                SkPaintParamsKeyBuilder* builder,
                                skgpu::graphite::ShaderCombo::ShaderType s,
                                SkTileMode tm,
                                SkBlendMode bm) {
    SkDEBUGCODE(builder->checkReset());

    switch (s) {
        case skgpu::graphite::ShaderCombo::ShaderType::kNone:
            DepthStencilOnlyBlock::AddToKey(keyContext, builder, nullptr);
            break;
        case skgpu::graphite::ShaderCombo::ShaderType::kSolidColor:
            SolidColorShaderBlock::AddToKey(keyContext, builder, nullptr, kErrorColor);
            break;
        case skgpu::graphite::ShaderCombo::ShaderType::kLinearGradient:
            GradientShaderBlocks::AddToKey(keyContext, builder, nullptr,
                                           { SkShader::kLinear_GradientType, tm, 0 });
            break;
        case skgpu::graphite::ShaderCombo::ShaderType::kRadialGradient:
            GradientShaderBlocks::AddToKey(keyContext, builder, nullptr,
                                           { SkShader::kRadial_GradientType, tm, 0 });
            break;
        case skgpu::graphite::ShaderCombo::ShaderType::kSweepGradient:
            GradientShaderBlocks::AddToKey(keyContext, builder, nullptr,
                                           { SkShader::kSweep_GradientType, tm, 0 });
            break;
        case skgpu::graphite::ShaderCombo::ShaderType::kConicalGradient:
            GradientShaderBlocks::AddToKey(keyContext, builder, nullptr,
                                           { SkShader::kConical_GradientType, tm, 0 });
            break;
    }

    // TODO: the blendInfo should be filled in by BlendModeBlock::AddToKey
    SkPipelineDataGatherer::BlendInfo blendInfo = get_blend_info(bm);
    BlendModeBlock::AddToKey(keyContext, builder, /* pipelineData*/ nullptr, bm);
    SkPaintParamsKey key = builder->lockAsKey();

    auto dict = keyContext.dict();

    auto entry = dict->findOrCreate(key, blendInfo);

    return  entry->uniqueID();
}
#endif
