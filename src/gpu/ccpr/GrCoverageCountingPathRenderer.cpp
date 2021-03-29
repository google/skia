/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ccpr/GrCoverageCountingPathRenderer.h"

#include "include/pathops/SkPathOps.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrFixedClip.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/ccpr/GrCCClipProcessor.h"
#include "src/gpu/ops/GrFillRectOp.h"

bool GrCoverageCountingPathRenderer::IsSupported(const GrCaps& caps) {
    const GrShaderCaps& shaderCaps = *caps.shaderCaps();
    GrBackendFormat defaultA8Format = caps.getDefaultBackendFormat(GrColorType::kAlpha_8,
                                                                   GrRenderable::kYes);
    if (caps.driverDisableMSAAClipAtlas() || !shaderCaps.integerSupport() ||
        !caps.drawInstancedSupport() || !shaderCaps.floatIs32Bits() ||
        !defaultA8Format.isValid() || // This checks both texturable and renderable
        !caps.halfFloatVertexAttributeSupport()) {
        return false;
    }

    if (caps.internalMultisampleCount(defaultA8Format) > 1 &&
        caps.sampleLocationsSupport() &&
        shaderCaps.sampleMaskSupport()) {
        return true;
    }

    return false;
}

std::unique_ptr<GrCoverageCountingPathRenderer> GrCoverageCountingPathRenderer::CreateIfSupported(
        const GrCaps& caps) {
    if (IsSupported(caps)) {
        return std::make_unique<GrCoverageCountingPathRenderer>();
    }
    return nullptr;
}

GrCCPerOpsTaskPaths* GrCoverageCountingPathRenderer::lookupPendingPaths(uint32_t opsTaskID) {
    auto it = fPendingPaths.find(opsTaskID);
    if (fPendingPaths.end() == it) {
        sk_sp<GrCCPerOpsTaskPaths> paths = sk_make_sp<GrCCPerOpsTaskPaths>();
        it = fPendingPaths.insert(std::make_pair(opsTaskID, std::move(paths))).first;
    }
    return it->second.get();
}

GrFPResult GrCoverageCountingPathRenderer::makeClipProcessor(
        std::unique_ptr<GrFragmentProcessor> inputFP, uint32_t opsTaskID,
        const SkPath& deviceSpacePath, const SkIRect& accessRect, const GrCaps& caps) {
#ifdef SK_DEBUG
    SkASSERT(!fFlushing);
    SkIRect pathIBounds;
    deviceSpacePath.getBounds().roundOut(&pathIBounds);
    SkIRect maskBounds;
    if (maskBounds.intersect(accessRect, pathIBounds)) {
        SkASSERT(maskBounds.height64() * maskBounds.width64() <= kMaxClipPathArea);
    }
#endif

    if (deviceSpacePath.isEmpty() ||
        !SkIRect::Intersects(accessRect, deviceSpacePath.getBounds().roundOut())) {
        // "Intersect" draws that don't intersect the clip can be dropped.
        return deviceSpacePath.isInverseFillType() ? GrFPSuccess(nullptr) : GrFPFailure(nullptr);
    }

    uint32_t key = deviceSpacePath.getGenerationID();
    key = (key << 1) | (uint32_t)GrFillRuleForSkPath(deviceSpacePath);
    sk_sp<GrCCClipPath>& clipPath = this->lookupPendingPaths(opsTaskID)->fClipPaths[key];
    if (!clipPath) {
        // This the first time we've accessed this clip path key in the map.
        clipPath = sk_make_sp<GrCCClipPath>(deviceSpacePath, accessRect, caps);
    } else {
        clipPath->addAccess(accessRect);
    }

    auto mustCheckBounds = GrCCClipProcessor::MustCheckBounds(
            !clipPath->pathDevIBounds().contains(accessRect));
    return GrFPSuccess(std::make_unique<GrCCClipProcessor>(std::move(inputFP), caps, clipPath,
                                                           mustCheckBounds));
}

namespace {

// Iterates all GrCCClipPaths in an array of non-empty maps.
class ClipMapsIter {
public:
    ClipMapsIter(const sk_sp<GrCCPerOpsTaskPaths>* mapsList) : fMapsList(mapsList) {}

    bool operator!=(const ClipMapsIter& that) {
        if (fMapsList != that.fMapsList) {
            return true;
        }
        // fPerOpsTaskClipPaths will be null when we are on the first element.
        if (fPerOpsTaskClipPaths != that.fPerOpsTaskClipPaths) {
            return true;
        }
        return fPerOpsTaskClipPaths && fClipPathsIter != that.fClipPathsIter;
    }

    void operator++() {
        // fPerOpsTaskClipPaths is null when we are on the first element.
        if (!fPerOpsTaskClipPaths) {
            fPerOpsTaskClipPaths = &(*fMapsList)->fClipPaths;
            SkASSERT(!fPerOpsTaskClipPaths->empty());  // We don't handle empty lists.
            fClipPathsIter = fPerOpsTaskClipPaths->begin();
        }
        if ((++fClipPathsIter) == fPerOpsTaskClipPaths->end()) {
            ++fMapsList;
            fPerOpsTaskClipPaths = nullptr;
        }
    }

    GrCCClipPath* get() {
        // fPerOpsTaskClipPaths is null when we are on the first element.
        const auto& it = (!fPerOpsTaskClipPaths) ? (*fMapsList)->fClipPaths.begin()
                                                 : fClipPathsIter;
        return it->second.get();
    }

    GrCCClipPath* operator->() {
        return this->get();
    }

private:
    const sk_sp<GrCCPerOpsTaskPaths>* fMapsList;
    std::map<uint32_t, sk_sp<GrCCClipPath>>* fPerOpsTaskClipPaths = nullptr;
    std::map<uint32_t, sk_sp<GrCCClipPath>>::iterator fClipPathsIter;
};

// Transfers MSAA stencil winding counts to A8 coverage values in the color buffer.
static void transfer_stencil_to_coverage(GrOnFlushResourceProvider* onFlushRP,
                                         GrSurfaceDrawContext* surfaceDrawContext, SkISize extent) {
    SkRect rect = SkRect::MakeSize(SkSize::Make(extent));
    GrAAType aaType = GrAAType::kMSAA;
    auto fillRectFlags = GrSimpleMeshDrawOpHelper::InputFlags::kNone;

    // This will be the final op in the surfaceDrawContext. So if Ganesh is planning to discard the
    // stencil values anyway, then we might not actually need to reset the stencil values back to 0.
    bool mustResetStencil = !onFlushRP->caps()->discardStencilValuesAfterRenderPass();

    if (surfaceDrawContext->numSamples() == 1) {
        // We are mixed sampled. We need to either enable conservative raster (preferred) or disable
        // MSAA in order to avoid double blend artifacts. (Even if we disable MSAA for the cover
        // geometry, the stencil test is still multisampled and will still produce smooth results.)
        if (onFlushRP->caps()->conservativeRasterSupport()) {
            fillRectFlags |= GrSimpleMeshDrawOpHelper::InputFlags::kConservativeRaster;
        } else {
            aaType = GrAAType::kNone;
        }
        mustResetStencil = true;
    }

    const GrUserStencilSettings* stencil;
    if (mustResetStencil) {
        constexpr static GrUserStencilSettings kTestAndResetStencil(
            GrUserStencilSettings::StaticInit<
                0x0000,
                GrUserStencilTest::kNotEqual,
                0xffff,
                GrUserStencilOp::kZero,
                GrUserStencilOp::kKeep,
                0xffff>());

        // Outset the cover rect in case there are T-junctions in the path bounds.
        rect.outset(1, 1);
        stencil = &kTestAndResetStencil;
    } else {
        constexpr static GrUserStencilSettings kTestStencil(
            GrUserStencilSettings::StaticInit<
                0x0000,
                GrUserStencilTest::kNotEqual,
                0xffff,
                GrUserStencilOp::kKeep,
                GrUserStencilOp::kKeep,
                0xffff>());

        stencil = &kTestStencil;
    }

    GrPaint paint;
    paint.setColor4f(SK_PMColor4fWHITE);
    GrQuad coverQuad(rect);
    DrawQuad drawQuad{coverQuad, coverQuad, GrQuadAAFlags::kAll};
    auto coverOp = GrFillRectOp::Make(surfaceDrawContext->recordingContext(), std::move(paint),
                                      aaType, &drawQuad, stencil, fillRectFlags);
    surfaceDrawContext->addDrawOp(nullptr, std::move(coverOp));
}

class AtlasBuilder {
public:
    AtlasBuilder(const GrCCAtlas::Specs& specs,
                 const SkTArray<sk_sp<GrCCPerOpsTaskPaths>>& flushingPaths)
            : fAtlasSpecs(specs)
            , fClipPathIter(flushingPaths.begin())
            , fEnd(flushingPaths.end())
            , fFirstUnassignedAtlas(fClipPathIter) {
    }

    void makeAtlases(GrOnFlushResourceProvider* onFlushRP) {
        for (; fClipPathIter != fEnd; ++fClipPathIter) {
            GrCCClipPath* clipPath = fClipPathIter.get();

            // Intersect the path and access rect.
            SkASSERT(!clipPath->deviceSpacePath().isEmpty());
            GrScissorTest enableScissorInAtlas;
            SkIRect clippedPathIBounds;
            if (clipPath->accessRect().contains(clipPath->pathDevIBounds())) {
                clippedPathIBounds = clipPath->pathDevIBounds();
                enableScissorInAtlas = GrScissorTest::kDisabled;
            } else {
                SkAssertResult(clippedPathIBounds.intersect(clipPath->accessRect(),
                                                            clipPath->pathDevIBounds()));
                enableScissorInAtlas = GrScissorTest::kEnabled;
            }

            // Allocate a spot in the atlas for the path mask.
            SkIPoint16 location;
            if (!fAtlas ||
                !fAtlas->addRect(clippedPathIBounds.width(), clippedPathIBounds.height(),
                                 &location)) {
                // The current atlas is out of room and can't grow any bigger.
                if (fAtlas) {
                    this->renderAtlasAndInstantiateProxies(onFlushRP);
                }
                fAtlas = std::make_unique<GrCCAtlas>(fAtlasSpecs, *onFlushRP->caps());
                SkASSERT(clippedPathIBounds.width() <= fAtlasSpecs.fMinWidth);
                SkASSERT(clippedPathIBounds.height() <= fAtlasSpecs.fMinHeight);
                SkAssertResult(fAtlas->addRect(clippedPathIBounds.width(),
                                               clippedPathIBounds.height(), &location));
            }
            clipPath->setAtlasTranslate(location.x() - clippedPathIBounds.left(),
                                        location.y() - clippedPathIBounds.top());

            // Add the path to our atlas layout.
            SkMatrix atlasMatrix = SkMatrix::Translate(clipPath->atlasTranslate().fX,
                                                       clipPath->atlasTranslate().fY);
            SkPath* atlasPath;
            if (enableScissorInAtlas == GrScissorTest::kDisabled) {
                atlasPath = &fAtlasPaths[(int)clipPath->fillRule()].fUberPath;
            } else {
                auto& [scissoredPath, scissor] =
                        fAtlasPaths[(int)clipPath->fillRule()].fScissoredPaths.push_back();
                scissor = clippedPathIBounds.makeOffset(clipPath->atlasTranslate());
                atlasPath = &scissoredPath;
            }
            auto origin = clippedPathIBounds.topLeft() + clipPath->atlasTranslate();
            atlasPath->moveTo(origin.fX, origin.fY);  // Implicit moveTo(0,0).
            atlasPath->addPath(clipPath->deviceSpacePath(), atlasMatrix);
        }

        if (fAtlas) {
            this->renderAtlasAndInstantiateProxies(onFlushRP);
        }

#ifdef SK_DEBUG
        // These paths should have been rendered and reset to empty by this point.
        for (size_t i = 0; i < SK_ARRAY_COUNT(fAtlasPaths); ++i) {
            SkASSERT(fAtlasPaths[i].fUberPath.isEmpty());
            SkASSERT(fAtlasPaths[i].fScissoredPaths.empty());
        }
        SkASSERT(!(fEnd != fClipPathIter));
#endif
    }

private:
    // Renders all enqueued paths into the given atlas and clears our path queue.
    void renderAtlasAndInstantiateProxies(GrOnFlushResourceProvider* onFlushRP) {
        SkASSERT(fAtlas);
        auto surfaceDrawContext = fAtlas->instantiate(onFlushRP);
        if (!surfaceDrawContext) {
            for (int i = 0; i < (int)SK_ARRAY_COUNT(fAtlasPaths); ++i) {
                fAtlasPaths[i].fUberPath.reset();
                fAtlasPaths[i].fScissoredPaths.reset();
            }
            return;
        }

        for (int i = 0; i < (int)SK_ARRAY_COUNT(fAtlasPaths); ++i) {
            SkPathFillType fillType = (i == (int)GrFillRule::kNonzero) ? SkPathFillType::kWinding
                                                                       : SkPathFillType::kEvenOdd;
            SkPath& uberPath = fAtlasPaths[i].fUberPath;
            if (!uberPath.isEmpty()) {
                uberPath.setIsVolatile(true);
                uberPath.setFillType(fillType);
                surfaceDrawContext->stencilPath(nullptr, GrAA::kYes, SkMatrix::I(), uberPath);
                uberPath.reset();
            }
            for (auto& [scissoredPath, scissor] : fAtlasPaths[i].fScissoredPaths) {
                GrFixedClip fixedClip(
                        surfaceDrawContext->asRenderTargetProxy()->backingStoreDimensions(),
                        scissor);
                scissoredPath.setIsVolatile(true);
                scissoredPath.setFillType(fillType);
                surfaceDrawContext->stencilPath(&fixedClip, GrAA::kYes, SkMatrix::I(),
                                                scissoredPath);
            }
            fAtlasPaths[i].fScissoredPaths.reset();
        }

        transfer_stencil_to_coverage(onFlushRP, surfaceDrawContext.get(), fAtlas->drawBounds());

        if (surfaceDrawContext->asSurfaceProxy()->requiresManualMSAAResolve()) {
            onFlushRP->addTextureResolveTask(sk_ref_sp(surfaceDrawContext->asTextureProxy()),
                                             GrSurfaceProxy::ResolveFlags::kMSAA);
        }

        // Assign the texture we just rendered to all pending atlas proxies.
        if (GrTexture* texture = surfaceDrawContext->asTextureProxy()->peekTexture()) {
            for (; fFirstUnassignedAtlas != fClipPathIter; ++fFirstUnassignedAtlas) {
                fFirstUnassignedAtlas->assignAtlasTexture(sk_ref_sp(texture));
            }
        }
    }

    const GrCCAtlas::Specs fAtlasSpecs;
    ClipMapsIter fClipPathIter;
    ClipMapsIter fEnd;
    ClipMapsIter fFirstUnassignedAtlas;

    // Atlas we are currently building.
    std::unique_ptr<GrCCAtlas> fAtlas;

    // Paths to be rendered in the atlas we are currently building.
    struct AtlasPaths {
        SkPath fUberPath;  // Contains all contours from all non-scissored paths.
        SkSTArray<32, std::tuple<SkPath, SkIRect>> fScissoredPaths;
    };
    static_assert((int)GrFillRule::kNonzero == 0);
    static_assert((int)GrFillRule::kEvenOdd == 1);
    AtlasPaths fAtlasPaths[2];  // One for "nonzero" fill rule and one for "even-odd".
};

}  // namespace

void GrCoverageCountingPathRenderer::preFlush(
        GrOnFlushResourceProvider* onFlushRP, SkSpan<const uint32_t> taskIDs) {
    SkASSERT(!fFlushing);
    SkDEBUGCODE(fFlushing = true);

    if (fPendingPaths.empty()) {
        return;  // Nothing to draw.
    }

    GrCCAtlas::Specs specs;
    int maxPreferredRTSize = onFlushRP->caps()->maxPreferredRenderTargetSize();
    specs.fMaxPreferredTextureSize = maxPreferredRTSize;
    specs.fMinTextureSize = std::min(512, maxPreferredRTSize);

    // Move the per-opsTask paths that are about to be flushed from fPendingPaths to flushingPaths,
    // and count them up so we can preallocate buffers.
    SkSTArray<8, sk_sp<GrCCPerOpsTaskPaths>> flushingPaths;
    flushingPaths.reserve_back(taskIDs.count());
    for (uint32_t taskID : taskIDs) {
        auto iter = fPendingPaths.find(taskID);
        if (fPendingPaths.end() == iter) {
            continue;  // No paths on this opsTask.
        }

        flushingPaths.push_back(std::move(iter->second));
        fPendingPaths.erase(iter);

        for (const auto& clipsIter : flushingPaths.back()->fClipPaths) {
            clipsIter.second->accountForOwnPath(&specs);
        }
    }

    AtlasBuilder atlasBuilder(specs, flushingPaths);
    atlasBuilder.makeAtlases(onFlushRP);
}

void GrCoverageCountingPathRenderer::postFlush(GrDeferredUploadToken,
                                               SkSpan<const uint32_t> /* taskIDs */) {
    SkASSERT(fFlushing);
    SkDEBUGCODE(fFlushing = false);
}
