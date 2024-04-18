/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/MSKPPlayer.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkCanvasVirtualEnforcer.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkSurface.h"
#include "include/docs/SkMultiPictureDocument.h"
#include "include/gpu/GrDirectContext.h"
#include "include/private/base/SkTArray.h"
#include "include/utils/SkNoDrawCanvas.h"
#include "src/base/SkTLazy.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkStringUtils.h"
#include "tools/SkSharingProc.h"

using namespace skia_private;

///////////////////////////////////////////////////////////////////////////////

// Base Cmd struct.
struct MSKPPlayer::Cmd {
    virtual ~Cmd() = default;
    virtual bool isFullRedraw(SkCanvas*) const = 0;
    virtual void draw(SkCanvas* canvas, const LayerMap&, LayerStateMap*) const = 0;
    // If this command draws another layer, return it's ID. Otherwise, -1.
    virtual int layerID() const { return -1; }
};

// Draws a SkPicture.
struct MSKPPlayer::PicCmd : Cmd {
    sk_sp<SkPicture> fContent;
    SkIRect fClipRect = SkIRect::MakeEmpty(); // clip for picture (no clip if empty).

    bool isFullRedraw(SkCanvas* canvas) const override {
        if (fClipRect.isEmpty()) {
            return false;
        }
        return fClipRect.contains(SkIRect::MakeSize(canvas->getBaseLayerSize()));
    }

    void draw(SkCanvas* canvas, const LayerMap&, LayerStateMap*) const override {
        if (!fClipRect.isEmpty()) {
            canvas->save();
            canvas->clipIRect(fClipRect);
        }
        canvas->drawPicture(fContent.get());
        if (!fClipRect.isEmpty()) {
            canvas->restore();
        }
    }
};

// Draws another layer. Stores the ID of the layer to draw and what command index on that
// layer should be current when the layer is drawn. The layer contents are updated to the
// stored command index before the layer is drawn.
struct MSKPPlayer::DrawLayerCmd : Cmd {
    int                         fLayerId;
    size_t                      fLayerCmdCnt;
    SkRect                      fSrcRect;
    SkRect                      fDstRect;
    SkSamplingOptions           fSampling;
    SkCanvas::SrcRectConstraint fConstraint;
    SkTLazy<SkPaint>            fPaint;

    bool isFullRedraw(SkCanvas* canvas) const override { return false; }
    void draw(SkCanvas* canvas, const LayerMap&, LayerStateMap*) const override;
    int layerID() const override { return fLayerId; }
};

void MSKPPlayer::DrawLayerCmd::draw(SkCanvas* canvas,
                                    const LayerMap& layerMap,
                                    LayerStateMap* layerStateMap) const {
    const LayerCmds& layer = layerMap.at(fLayerId);
    LayerState* layerState = &(*layerStateMap)[fLayerId];
    if (!layerState->fSurface) {
        layerState->fCurrCmd = 0;
        layerState->fSurface = MSKPPlayer::MakeSurfaceForLayer(layer, canvas);
        if (!layerState->fSurface) {
            SkDebugf("Couldn't create surface for layer");
            return;
        }
    }
    size_t cmd = layerState->fCurrCmd;
    if (cmd > fLayerCmdCnt) {
        // If the layer contains contents from later commands then replay from the beginning.
        cmd = 0;
    }
    SkCanvas* layerCanvas = layerState->fSurface->getCanvas();
    // Check if there is a full redraw between cmd and fLayerCmdCnt and if so jump to it and ensure
    // we clear the canvas if starting from a full redraw.
    for (size_t checkCmd = fLayerCmdCnt - 1; checkCmd > cmd; --checkCmd) {
        if (layer.fCmds[checkCmd]->isFullRedraw(layerCanvas)) {
            cmd = checkCmd;
            break;
        }
    }
    for (; cmd < fLayerCmdCnt; ++cmd) {
        if (cmd == 0 || layer.fCmds[cmd]->isFullRedraw(layerCanvas)) {
            layerState->fSurface->getCanvas()->clear(SK_ColorTRANSPARENT);
        }
        layer.fCmds[cmd]->draw(layerCanvas, layerMap, layerStateMap);
    }
    layerState->fCurrCmd = fLayerCmdCnt;
    const SkPaint* paint = fPaint.isValid() ? fPaint.get() : nullptr;
    canvas->drawImageRect(layerState->fSurface->makeImageSnapshot(),
                          fSrcRect,
                          fDstRect,
                          fSampling,
                          paint,
                          fConstraint);
}

///////////////////////////////////////////////////////////////////////////////

class MSKPPlayer::CmdRecordCanvas : public SkCanvasVirtualEnforcer<SkCanvas> {
public:
    CmdRecordCanvas(LayerCmds* dst, LayerMap* offscreenLayers, const SkIRect* clipRect = nullptr)
            : fDst(dst), fOffscreenLayers(offscreenLayers) {
        if (clipRect) {
            fClipRect = *clipRect;
        }
        fRecorder.beginRecording(SkRect::Make(dst->fDimensions));
    }
    ~CmdRecordCanvas() override { this->recordPicCmd(); }

protected:
    void onDrawPaint(const SkPaint& paint) override {
        fRecorder.getRecordingCanvas()->drawPaint(paint);
    }

    void onDrawBehind(const SkPaint& paint) override {
        SkCanvasPriv::DrawBehind(fRecorder.getRecordingCanvas(), paint);
    }

    void onDrawRect(const SkRect& rect, const SkPaint& paint) override {
        fRecorder.getRecordingCanvas()->drawRect(rect, paint);
    }

    void onDrawRRect(const SkRRect& rrect, const SkPaint& paint) override {
        fRecorder.getRecordingCanvas()->drawRRect(rrect, paint);
    }

    void onDrawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) override {
        fRecorder.getRecordingCanvas()->drawDRRect(outer, inner, paint);
    }

    void onDrawOval(const SkRect& rect, const SkPaint& paint) override {
        fRecorder.getRecordingCanvas()->drawOval(rect, paint);
    }

    void onDrawArc(const SkRect& rect,
                   SkScalar startAngle,
                   SkScalar sweepAngle,
                   bool useCenter,
                   const SkPaint& paint) override {
        fRecorder.getRecordingCanvas()->drawArc(rect, startAngle, sweepAngle, useCenter, paint);
    }

    void onDrawPath(const SkPath& path, const SkPaint& paint) override {
        fRecorder.getRecordingCanvas()->drawPath(path, paint);
    }

    void onDrawRegion(const SkRegion& region, const SkPaint& paint) override {
        fRecorder.getRecordingCanvas()->drawRegion(region, paint);
    }

    void onDrawTextBlob(const SkTextBlob* blob,
                        SkScalar x,
                        SkScalar y,
                        const SkPaint& paint) override {
        fRecorder.getRecordingCanvas()->drawTextBlob(blob, x, y, paint);
    }

    void onDrawPatch(const SkPoint cubics[12],
                     const SkColor colors[4],
                     const SkPoint texCoords[4],
                     SkBlendMode mode,
                     const SkPaint& paint) override {
        fRecorder.getRecordingCanvas()->drawPatch(cubics, colors, texCoords, mode, paint);
    }

    void onDrawPoints(SkCanvas::PointMode mode,
                      size_t count,
                      const SkPoint pts[],
                      const SkPaint& paint) override {
        fRecorder.getRecordingCanvas()->drawPoints(mode, count, pts, paint);
    }

    void onDrawImage2(const SkImage* image,
                      SkScalar dx,
                      SkScalar dy,
                      const SkSamplingOptions& sampling,
                      const SkPaint* paint) override {
        fRecorder.getRecordingCanvas()->drawImage(image, dx, dy, sampling, paint);
    }

    void onDrawImageRect2(const SkImage* image,
                          const SkRect& src,
                          const SkRect& dst,
                          const SkSamplingOptions& sampling,
                          const SkPaint* paint,
                          SrcRectConstraint constraint) override {
        if (fNextDrawImageFromLayerID != -1) {
            this->recordPicCmd();
            auto drawLayer = std::make_unique<DrawLayerCmd>();
            drawLayer->fLayerId = fNextDrawImageFromLayerID;
            drawLayer->fLayerCmdCnt = fOffscreenLayers->at(fNextDrawImageFromLayerID).fCmds.size();
            drawLayer->fSrcRect = src;
            drawLayer->fDstRect = dst;
            drawLayer->fSampling = sampling;
            drawLayer->fConstraint = constraint;
            if (paint) {
                drawLayer->fPaint.init(*paint);
            }
            fDst->fCmds.push_back(std::move(drawLayer));
            fNextDrawImageFromLayerID = -1;
            return;
        }
        fRecorder.getRecordingCanvas()->drawImageRect(image, src, dst, sampling, paint, constraint);
    }

    void onDrawImageLattice2(const SkImage* image,
                             const Lattice& lattice,
                             const SkRect& dst,
                             SkFilterMode mode,
                             const SkPaint* paint) override {
        fRecorder.getRecordingCanvas()->drawImageLattice(image, lattice, dst, mode, paint);
    }

    void onDrawAtlas2(const SkImage* image,
                      const SkRSXform rsxForms[],
                      const SkRect src[],
                      const SkColor colors[],
                      int count,
                      SkBlendMode mode,
                      const SkSamplingOptions& sampling,
                      const SkRect* cull,
                      const SkPaint* paint) override {
        fRecorder.getRecordingCanvas()->drawAtlas(image,
                                                  rsxForms,
                                                  src,
                                                  colors,
                                                  count,
                                                  mode,
                                                  sampling,
                                                  cull,
                                                  paint);
    }

    void onDrawEdgeAAImageSet2(const ImageSetEntry imageSet[],
                               int count,
                               const SkPoint dstClips[],
                               const SkMatrix preViewMatrices[],
                               const SkSamplingOptions& sampling,
                               const SkPaint* paint,
                               SrcRectConstraint constraint) override {
        fRecorder.getRecordingCanvas()->experimental_DrawEdgeAAImageSet(imageSet,
                                                                        count,
                                                                        dstClips,
                                                                        preViewMatrices,
                                                                        sampling,
                                                                        paint,
                                                                        constraint);
    }

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    void onDrawEdgeAAQuad(const SkRect& rect,
                          const SkPoint clip[4],
                          SkCanvas::QuadAAFlags aaFlags,
                          const SkColor4f& color,
                          SkBlendMode mode) override {}
#else
    void onDrawEdgeAAQuad(const SkRect& rect,
                          const SkPoint clip[4],
                          SkCanvas::QuadAAFlags aaFlags,
                          const SkColor4f& color,
                          SkBlendMode mode) override {
        fRecorder.getRecordingCanvas()->experimental_DrawEdgeAAQuad(rect,
                                                                    clip,
                                                                    aaFlags,
                                                                    color,
                                                                    mode);
    }
#endif

    void onDrawAnnotation(const SkRect& rect, const char key[], SkData* value) override {
        static constexpr char kOffscreenLayerDraw[] = "OffscreenLayerDraw";
        static constexpr char kSurfaceID[] = "SurfaceID";
        TArray<SkString> tokens;
        SkStrSplit(key, "|", kStrict_SkStrSplitMode, &tokens);
        if (tokens.size() == 2) {
            if (tokens[0].equals(kOffscreenLayerDraw)) {
                // Indicates that the next drawPicture command contains the SkPicture to render
                // to the layer identified by the ID. 'rect' indicates the dirty area to update
                // (and indicates the layer size if this command is introducing a new layer id).
                fNextDrawPictureToLayerID = std::stoi(tokens[1].c_str());
                fNextDrawPictureToLayerClipRect = rect.roundOut();
                if (fOffscreenLayers->find(fNextDrawPictureToLayerID) == fOffscreenLayers->end()) {
                    SkASSERT(fNextDrawPictureToLayerClipRect.left() == 0 &&
                             fNextDrawPictureToLayerClipRect.top()  == 0);
                    (*fOffscreenLayers)[fNextDrawPictureToLayerID].fDimensions =
                            fNextDrawPictureToLayerClipRect.size();
                }
                // The next draw picture will notice that fNextDrawPictureToLayerID is set and
                // redirect the picture to the offscreen layer.
                return;
            } else if (tokens[0].equals(kSurfaceID)) {
                // Indicates that the following drawImageRect should draw an offscreen layer
                // to this layer.
                fNextDrawImageFromLayerID = std::stoi(tokens[1].c_str());
                return;
            }
        }
    }

    void onDrawShadowRec(const SkPath& path, const SkDrawShadowRec& rec) override {
        fRecorder.getRecordingCanvas()->private_draw_shadow_rec(path, rec);
    }

    void onDrawDrawable(SkDrawable* drawable, const SkMatrix* matrix) override {
        fRecorder.getRecordingCanvas()->drawDrawable(drawable, matrix);
    }

    void onDrawPicture(const SkPicture* picture,
                       const SkMatrix* matrix,
                       const SkPaint* paint) override {
        if (fNextDrawPictureToLayerID != -1) {
            SkASSERT(!matrix);
            SkASSERT(!paint);
            LayerCmds* layer = &fOffscreenLayers->at(fNextDrawPictureToLayerID);
            CmdRecordCanvas sc(layer, fOffscreenLayers, &fNextDrawPictureToLayerClipRect);
            picture->playback(&sc);
            fNextDrawPictureToLayerID = -1;
            fNextDrawPictureToLayerClipRect = SkIRect::MakeEmpty();
            return;
        }
        if (paint) {
            this->saveLayer(nullptr, paint);
        }
        if (matrix) {
            this->save();
            this->concat(*matrix);
        }

        picture->playback(this);

        if (matrix) {
            this->restore();
        }
        if (paint) {
            this->restore();
        }
    }

private:
    void recordPicCmd() {
        auto cmd = std::make_unique<PicCmd>();
        cmd->fContent = fRecorder.finishRecordingAsPicture();
        cmd->fClipRect = fClipRect;
        if (cmd->fContent) {
            fDst->fCmds.push_back(std::move(cmd));
        }
        // Set up to accumulate again.
        fRecorder.beginRecording(SkRect::Make(fDst->fDimensions));
    }

    SkPictureRecorder fRecorder; // accumulates draws until we draw an offscreen into this layer.
    LayerCmds*        fDst                            = nullptr;
    SkIRect           fClipRect                       = SkIRect::MakeEmpty();
    int               fNextDrawPictureToLayerID       = -1;
    SkIRect           fNextDrawPictureToLayerClipRect = SkIRect::MakeEmpty();
    int               fNextDrawImageFromLayerID       = -1;
    LayerMap*         fOffscreenLayers                = nullptr;
};

///////////////////////////////////////////////////////////////////////////////

std::unique_ptr<MSKPPlayer> MSKPPlayer::Make(SkStreamSeekable* stream) {
    auto deserialContext = std::make_unique<SkSharingDeserialContext>();
    SkDeserialProcs procs;
    procs.fImageProc = SkSharingDeserialContext::deserializeImage;
    procs.fImageCtx = deserialContext.get();

    int pageCount = SkMultiPictureDocument::ReadPageCount(stream);
    if (!pageCount) {
        return nullptr;
    }
    std::vector<SkDocumentPage> pages(pageCount);
    if (!SkMultiPictureDocument::Read(stream, pages.data(), pageCount, &procs)) {
        return nullptr;
    }
    std::unique_ptr<MSKPPlayer> result(new MSKPPlayer);
    result->fRootLayers.reserve(pages.size());
    for (const auto& page : pages) {
        SkISize dims = {SkScalarCeilToInt(page.fSize.width()),
                        SkScalarCeilToInt(page.fSize.height())};
        result->fRootLayers.emplace_back();
        result->fRootLayers.back().fDimensions = dims;
        result->fMaxDimensions.fWidth  = std::max(dims.width() , result->fMaxDimensions.width() );
        result->fMaxDimensions.fHeight = std::max(dims.height(), result->fMaxDimensions.height());
        CmdRecordCanvas sc(&result->fRootLayers.back(), &result->fOffscreenLayers);
        page.fPicture->playback(&sc);
    }
    return result;
}

MSKPPlayer::~MSKPPlayer() = default;

SkISize MSKPPlayer::frameDimensions(int i) const {
    if (i < 0 || i >= this->numFrames()) {
        return {-1, -1};
    }
    return fRootLayers[i].fDimensions;
}

bool MSKPPlayer::playFrame(SkCanvas* canvas, int i) {
    if (i < 0 || i >= this->numFrames()) {
        return false;
    }

    // Find the first offscreen layer that has a valid surface. If it's recording context
    // differs from the passed canvas's then reset all the layers. Playback will
    // automatically allocate new surfaces for offscreen layers as they're encountered.
    for (const auto& ols : fOffscreenLayerStates) {
        const LayerState& state = ols.second;
        if (state.fSurface) {
            if (state.fSurface->recordingContext() != canvas->recordingContext()) {
                this->resetLayers();
            }
            break;
        }
    }

    // Replay all the commands for this frame to the caller's canvas.
    const LayerCmds& layer = fRootLayers[i];
    for (const auto& cmd : layer.fCmds) {
        cmd->draw(canvas, fOffscreenLayers, &fOffscreenLayerStates);
    }
    return true;
}

sk_sp<SkSurface> MSKPPlayer::MakeSurfaceForLayer(const LayerCmds& layer, SkCanvas* rootCanvas) {
    // Assume layer has same surface props and info as this (mskp doesn't currently record this
    // data).
    SkSurfaceProps props;
    rootCanvas->getProps(&props);
    return rootCanvas->makeSurface(rootCanvas->imageInfo().makeDimensions(layer.fDimensions),
                                   &props);
}

void MSKPPlayer::resetLayers() { fOffscreenLayerStates.clear(); }

void MSKPPlayer::rewindLayers() {
    for (auto& [id, state] : fOffscreenLayerStates) {
        state.fCurrCmd = -1;
    }
}

void MSKPPlayer::allocateLayers(SkCanvas* canvas) {
    // Iterate over layers not states as states are lazily created in playback but here we want to
    // create any that don't already exist.
    for (auto& [id, layer] : fOffscreenLayers) {
        LayerState& state = fOffscreenLayerStates[id];
        if (!state.fSurface || state.fSurface->recordingContext() != canvas->recordingContext()) {
            state.fCurrCmd = -1;
            state.fSurface = MakeSurfaceForLayer(fOffscreenLayers[id], canvas);
        }
    }
}

std::vector<int> MSKPPlayer::layerIDs(int frame) const {
    std::vector<int> result;
    if (frame < 0) {
        result.reserve(fOffscreenLayers.size());
        for (auto& [id, _] : fOffscreenLayers) {
            result.push_back(id);
        }
        return result;
    }
    if (frame < static_cast<int>(fRootLayers.size())) {
        this->collectReferencedLayers(fRootLayers[frame], &result);
    }
    return result;
}

sk_sp<SkImage> MSKPPlayer::layerSnapshot(int layerID) const {
    auto iter = fOffscreenLayerStates.find(layerID);
    if (iter == fOffscreenLayerStates.end() || !iter->second.fSurface) {
        return nullptr;
    }
    return iter->second.fSurface->makeImageSnapshot();
}

void MSKPPlayer::collectReferencedLayers(const LayerCmds& layer, std::vector<int>* out) const {
    for (const auto& cmd : layer.fCmds) {
        if (int id = cmd->layerID(); id >= 0) {
            // Linear, but we'd need to have a lot of layers to actually care.
            if (std::find(out->begin(), out->end(), id) == out->end()) {
                out->push_back(id);
                auto iter = fOffscreenLayers.find(id);
                SkASSERT(iter != fOffscreenLayers.end());
                this->collectReferencedLayers(iter->second, out);
            }
        }
    }
}
