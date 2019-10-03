/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "samplecode/Sample.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkFont.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSurface.h"

#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"

#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkSpecialImage.h"

#include "tools/ToolUtils.h"

namespace {

struct FilterNode {
    // Pointer to the actual filter in the DAG, so it still contains its input filters and
    // may be used as an input in an earlier node. Null when this represents the "source" input
    sk_sp<SkImageFilter> fFilter;

    // FilterNodes wrapping each of fFilter's inputs. Leaf node when fInputNodes is empty.
    SkTArray<FilterNode> fInputNodes;

    // Distance from root filter
    int fDepth;

    // The source content rect (this is the same for all nodes, but is stored here for convenience)
    SkRect fContent;
    // The portion of the original CTM that is kept as the local matrix/ctm when filtering
    SkMatrix fLocalCTM;
    // The portion of the original CTM that the results should be drawn with (or given current
    // canvas impl., the portion of the CTM that is baked into a new DAG)
    SkMatrix fRemainingCTM;

    // Cached reverse bounds using device-space clip bounds (e.g. SkCanvas::clipRectBounds with
    // null first argument). This represents the layer calculated in SkCanvas for the filtering.
    // FIXME: SkCanvas (and this sample), this is seeded with the device-space clip bounds so that
    // the implicit matrix node's reverse bounds are updated appropriately when it recurses to the
    // original root node.
    SkIRect fLayerBounds;

    // Cached reverse bounds using the local draw bounds (e.g. SkCanvas::clipRectBounds with the
    // draw bounds provided as first argument). For intermediate nodes in a DAG, this is calculated
    // to match what the filter would compute when being evaluated as part of the original DAG
    // (i.e. if the implicit matrix filter node were not inserted at the beginning).
    // fReverseLocalIsolatedBounds is the same, except it represents what would be calculated if
    // only this node were being applied as the image filter.
    SkIRect fReverseLocalBounds;
    SkIRect fReverseLocalIsolatedBounds;

    // Cached forward bounds based on local draw bounds. For intermediate nodes in a DAG, this is
    // calculated to match what the filter computes as part of the whole DAG. fForwardIsolatedBounds
    // is the same but represents what would be calculated if only this node were applied.
    SkIRect fForwardBounds;
    SkIRect fForwardIsolatedBounds;

    // Should be called after the input nodes have been created since this will complete the
    // entire tree.
    void computeBounds() {
        // In normal usage, forward bounds are filter-space bounds of the geometry content, so
        // fContent mapped by the local matrix, since we assume the layer content is made by
        // concat(localCTM) -> clipRect(content) -> drawRect(content).
        // Similarly, in normal usage, reverse bounds are the filter-space bounds of the space to
        // be filled by image filter results. Since the clip rect is set to the same as the content,
        // it's the same bounds forward or reverse in this contrived case.
        SkIRect inputRect;
        fLocalCTM.mapRect(fContent).roundOut(&inputRect);

        this->computeForwardBounds(inputRect);

        // The layer bounds (matching what SkCanvas computes), use the content rect mapped by the
        // entire CTM as its input rect. If this is an implicit matrix node, the computeReverseX
        // functions will switch to using the local-mapped bounds for children in order to simulate
        // what would happen if the last step were done as a draw. When there's no implicit matrix
        // node, this calculated rectangle is the same as inputRect.
        SkIRect deviceRect;
        SkMatrix ctm = SkMatrix::Concat(fRemainingCTM, fLocalCTM);
        ctm.mapRect(fContent).roundOut(&deviceRect);

        SkASSERT(this->isImplicitMatrixNode() || inputRect == deviceRect);
        this->computeReverseLocalIsolatedBounds(deviceRect);
        this->computeReverseBounds(deviceRect, false);
        // Unlike the above two calls, calculating layer bounds will keep the device bounds for
        // intermediate nodes to show the current SkCanvas behavior vs. the ideal
        this->computeReverseBounds(deviceRect, true);
    }

    bool isImplicitMatrixNode() const {
        // In the future we wish to replace the implicit matrix node with direct draws to the final
        // destination (instead of using an SkMatrixImageFilter). Visualizing the DAG correctly
        // requires handling these nodes differently since it has part of the canvas CTM built in.
        return fDepth == 1 && !fRemainingCTM.isIdentity();
    }

private:
    void computeForwardBounds(const SkIRect srcRect) {
        if (fFilter) {
            // For forward filtering, the leaves of the DAG are evaluated first and are then
            // propagated to the root. This means that every filter's filterBounds() function sees
            // the original src rect. It is never dependent on the parent node (unlike reverse
            // filtering), so calling filterBounds() on an intermediate node gives us the correct
            // intermediate values.
            fForwardBounds = fFilter->filterBounds(
                    srcRect, fLocalCTM, SkImageFilter::kForward_MapDirection, nullptr);

            // For isolated forward filtering, it uses the same input but should not be propagated
            // to the inputs, so get the filter node bounds directly.
            fForwardIsolatedBounds = as_IFB(fFilter)->filterNodeBounds(
                    srcRect, fLocalCTM, SkImageFilter::kForward_MapDirection, nullptr);
        } else {
            fForwardBounds = srcRect;
            fForwardIsolatedBounds = srcRect;
        }

        // Fill in children
        for (int i = 0; i < fInputNodes.count(); ++i) {
            fInputNodes[i].computeForwardBounds(srcRect);
        }
    }

    void computeReverseLocalIsolatedBounds(const SkIRect& srcRect) {
        if (fFilter) {
            fReverseLocalIsolatedBounds = as_IFB(fFilter)->filterNodeBounds(
                    srcRect, fLocalCTM, SkImageFilter::kReverse_MapDirection, &srcRect);
        } else {
            fReverseLocalIsolatedBounds = srcRect;
        }

        SkIRect childSrcRect = srcRect;
        if (this->isImplicitMatrixNode()) {
            // Switch srcRect from the device-space bounds to what would be used when the draw is
            // the final step of filtering, as if the implicit node weren't needed
            fLocalCTM.mapRect(fContent).roundOut(&childSrcRect);
        }

        // Fill in children. Unlike regular reverse bounds mapping, the input nodes see the original
        // bounds. Normally, the bounds that the child nodes see have already been mapped processed
        // by this node.
        for (int i = 0; i < fInputNodes.count(); ++i) {
            fInputNodes[i].computeReverseLocalIsolatedBounds(childSrcRect);
        }
    }

    // fReverseLocalBounds and fLayerBounds are computed the same, except they differ in what the
    // initial bounding rectangle was. It is assumed that the 'srcRect' has already been processed
    // by the parent node's onFilterNodeBounds() function, as in SkImageFilter::filterBounds().
    void computeReverseBounds(const SkIRect& srcRect, bool writeToLayerBounds) {
        SkIRect reverseBounds = srcRect;

        if (fFilter) {
            // Since srcRect has been through parent's onFilterNodeBounds(), calling filterBounds()
            // directly on this node will calculate the same rectangle that this filter would report
            // during the parent node's onFilterBounds() recursion.
            reverseBounds = fFilter->filterBounds(
                    srcRect, fLocalCTM, SkImageFilter::kReverse_MapDirection, &srcRect);

            SkIRect nextSrcRect;
            if (this->isImplicitMatrixNode() && !writeToLayerBounds) {
                // When not writing to the layer bounds, and we're the implicit matrix node
                // we reset the src rect to be what it should be if no implicit node was necessary.
                fLocalCTM.mapRect(fContent).roundOut(&nextSrcRect);
            } else {
                // To calculate the appropriate intermediate reverse bounds for the children, we
                // need this node's onFilterNodeBounds() results based on its parents' bounds (the
                // current 'srcRect').
                nextSrcRect = as_IFB(fFilter)->filterNodeBounds(
                    srcRect, fLocalCTM, SkImageFilter::kReverse_MapDirection, &srcRect);
            }

            // Fill in the children. The union of these bounds should equal the value calculated
            // for reverseBounds already.
            SkDEBUGCODE(SkIRect netReverseBounds = SkIRect::MakeEmpty();)
            for (int i = 0; i < fInputNodes.count(); ++i) {
                fInputNodes[i].computeReverseBounds(nextSrcRect, writeToLayerBounds);
                SkDEBUGCODE(netReverseBounds.join(
                        writeToLayerBounds ? fInputNodes[i].fLayerBounds
                                           : fInputNodes[i].fReverseLocalBounds);)
            }
            // Because of the resetting done when not computing layer bounds for the implicit
            // matrix node, this assertion doesn't hold in that particular scenario.
            SkASSERT(netReverseBounds == reverseBounds ||
                     (this->isImplicitMatrixNode() && !writeToLayerBounds));
        }

        if (writeToLayerBounds) {
            fLayerBounds = reverseBounds;
        } else {
            fReverseLocalBounds = reverseBounds;
        }
    }
};

} // anonymous namespace

static FilterNode build_dag(const SkMatrix& local, const SkMatrix& remainder, const SkRect& rect,
                            const SkImageFilter* filter, int depth) {
    FilterNode node;
    node.fFilter = sk_ref_sp(filter);
    node.fDepth = depth;
    node.fContent = rect;

    node.fLocalCTM = local;
    node.fRemainingCTM = remainder;

    if (node.fFilter) {
        if (depth > 0) {
            // We don't visit children when at the root because the real child filters are replaced
            // with the internalSaveLayer decomposition emulation, which then cycles back to the
            // original filter but with an updated matrix (and then we process the children).
            node.fInputNodes.reserve(node.fFilter->countInputs());
            for (int i = 0; i < node.fFilter->countInputs(); ++i) {
                node.fInputNodes.push_back() =
                        build_dag(local, remainder, rect, node.fFilter->getInput(i), depth + 1);
            }
        }
    }

    return node;
}

static FilterNode build_dag(const SkMatrix& ctm, const SkRect& rect,
                            const SkImageFilter* rootFilter) {
    // Emulate SkCanvas::internalSaveLayer's decomposition of the CTM.
    SkMatrix local;
    sk_sp<SkImageFilter> finalFilter = as_IFB(rootFilter)->applyCTM(ctm, &local);

    // In ApplyCTMToFilter, the CTM is decomposed such that CTM = remainder * local. The matrix
    // that is embedded in 'finalFilter' is actually local^-1*remainder*local to account for
    // how SkMatrixImageFilter is specified, but we want the true remainder since it is what should
    // transform the results to put in the correct place after filtering.
    SkMatrix invLocal, remaining;
    if (as_IFB(rootFilter)->uniqueID() != as_IFB(finalFilter)->uniqueID()) {
        remaining = SkMatrix::Concat(ctm, invLocal);
    } else {
        remaining = SkMatrix::I();
    }

    // Create a root node that represents the full result
    FilterNode rootNode = build_dag(ctm, SkMatrix::I(), rect, rootFilter, 0);
    // Set its only child as the modified DAG that handles the CTM decomposition
    rootNode.fInputNodes.push_back() =
            build_dag(local, remaining, rect, finalFilter.get(), 1);
    // Fill in bounds information that requires the entire node DAG to have been extracted first.
    rootNode.fInputNodes[0].computeBounds();
    return rootNode;
}

static void draw_node(SkCanvas* canvas, const FilterNode& node) {
    canvas->clear(SK_ColorTRANSPARENT);

    SkPaint filterPaint;
    filterPaint.setImageFilter(node.fFilter);

    SkPaint paint;
    static const SkColor kColors[2] = {SK_ColorGREEN, SK_ColorWHITE};
    SkPoint points[2] = { {node.fContent.fLeft + 15.f, node.fContent.fTop + 15.f},
                          {node.fContent.fRight - 15.f, node.fContent.fBottom - 15.f} };
    paint.setShader(SkGradientShader::MakeLinear(points, kColors, nullptr, SK_ARRAY_COUNT(kColors),
                                                 SkTileMode::kRepeat));

    SkPaint line;
    line.setStrokeWidth(0.f);
    line.setStyle(SkPaint::kStroke_Style);

    if (node.fDepth == 0) {
        // The root node, so draw this one the canonical way through SkCanvas to show current
        // net behavior. Will not include bounds visualization.
        canvas->save();
        canvas->concat(node.fLocalCTM);
        SkASSERT(node.fRemainingCTM.isIdentity());

        canvas->clipRect(node.fContent, /* aa */ true);
        canvas->saveLayer(nullptr, &filterPaint);
        canvas->drawRect(node.fContent, paint);
        canvas->restore(); // Completes the image filter
        canvas->restore(); // Undoes matrix and clip

        // Draw content rect (no clipping)
        canvas->save();
        canvas->concat(node.fLocalCTM);
        line.setColor(SK_ColorBLACK);
        canvas->drawRect(node.fContent, line);
        canvas->restore();
    } else {
        canvas->save();
        if (!node.isImplicitMatrixNode()) {
            canvas->concat(node.fRemainingCTM);
        }
        canvas->concat(node.fLocalCTM);

        canvas->saveLayer(nullptr, &filterPaint);
        canvas->drawRect(node.fContent, paint);
        canvas->restore(); // Completes the image filter

        // Draw content-rect bounds
        line.setColor(SK_ColorBLACK);
        if (node.isImplicitMatrixNode()) {
            canvas->setMatrix(SkMatrix::Concat(node.fRemainingCTM, node.fLocalCTM));
        }
        canvas->drawRect(node.fContent, line);
        canvas->restore(); // Undoes the matrix

        // Bounding boxes have all been mapped by the local matrix already, so drawing them with
        // the remaining CTM should align everything to the already drawn filter outputs. The
        // exception is forward bounds of the implicit matrix node, which also have been mapped
        // by the remainder matrix.
        canvas->save();
        canvas->concat(node.fRemainingCTM);

        // The bounds of the layer saved for the filtering as currently implemented
        line.setColor(SK_ColorRED);
        canvas->drawRect(SkRect::Make(node.fLayerBounds).makeOutset(5.f, 5.f), line);
        // The bounds of the layer that could be saved if the last step were a draw
        line.setColor(SK_ColorMAGENTA);
        canvas->drawRect(SkRect::Make(node.fReverseLocalBounds).makeOutset(4.f, 4.f), line);

        // Dashed lines for the isolated shapes
        static const SkScalar kDashParams[] = {6.f, 12.f};
        line.setPathEffect(SkDashPathEffect::Make(kDashParams, 2, 0.f));
        // The bounds of the layer if it were the only filter in the DAG
        canvas->drawRect(SkRect::Make(node.fReverseLocalIsolatedBounds).makeOutset(3.f, 3.f), line);

        if (node.isImplicitMatrixNode()) {
            canvas->resetMatrix();
        }
        // The output bounds calculated as if the node were the only filter in the DAG
        line.setColor(SK_ColorBLUE);
        canvas->drawRect(SkRect::Make(node.fForwardIsolatedBounds).makeOutset(1.f, 1.f), line);

        // The output bounds calculated for the node
        line.setPathEffect(nullptr);
        canvas->drawRect(SkRect::Make(node.fForwardBounds).makeOutset(2.f, 2.f), line);

        canvas->restore();
    }
}

static constexpr float kLineHeight = 16.f;
static constexpr float kLineInset = 8.f;

static float print_matrix(SkCanvas* canvas, const char* prefix, const SkMatrix& matrix,
                         float x, float y, const SkFont& font, const SkPaint& paint) {
    canvas->drawString(prefix, x, y, font, paint);
    y += kLineHeight;
    for (int i = 0; i < 3; ++i) {
        SkString row;
        row.appendf("[%.2f %.2f %.2f]",
                    matrix.get(i * 3), matrix.get(i * 3 + 1), matrix.get(i * 3 + 2));
        canvas->drawString(row, x, y, font, paint);
        y += kLineHeight;
    }
    return y;
}

static float print_size(SkCanvas* canvas, const char* prefix, const SkIRect& rect,
                       float x, float y, const SkFont& font, const SkPaint& paint) {
    canvas->drawString(prefix, x, y, font, paint);
    y += kLineHeight;
    SkString sz;
    sz.appendf("%d x %d", rect.width(), rect.height());
    canvas->drawString(sz, x, y, font, paint);
    return y + kLineHeight;
}

static float print_info(SkCanvas* canvas, const FilterNode& node) {
    SkFont font(nullptr, 12);
    SkPaint text;
    text.setAntiAlias(true);

    float y = kLineHeight;
    if (node.fDepth == 0) {
        canvas->drawString("Final Results", kLineInset, y, font, text);
        // The actual interesting matrices are in the root node's first child
        y = print_matrix(canvas, "Local", node.fInputNodes[0].fLocalCTM,
                     kLineInset, y + kLineHeight, font, text);
        y = print_matrix(canvas, "Embedded", node.fInputNodes[0].fRemainingCTM,
                     kLineInset, y, font, text);
    } else if (node.fFilter) {
        canvas->drawString(node.fFilter->getTypeName(), kLineInset, y, font, text);
        print_size(canvas, "Layer Size", node.fLayerBounds, kLineInset, y + kLineHeight,
                   font, text);
        y = print_size(canvas, "Ideal Size", node.fReverseLocalBounds, 10 * kLineInset,
                       y + kLineHeight, font, text);
    } else {
        canvas->drawString("Source Input", kLineInset, kLineHeight, font, text);
        y += kLineHeight;
    }

    return y;
}

// Returns bottom edge in pixels that the subtree reached in canvas
static float draw_dag(SkCanvas* canvas, SkSurface* nodeSurface, const FilterNode& node) {
    // First capture the results of the node, into nodeSurface
    draw_node(nodeSurface->getCanvas(), node);
    sk_sp<SkImage> nodeResults = nodeSurface->makeImageSnapshot();

    // Fill in background of the filter node with a checkerboard
    canvas->save();
    canvas->clipRect(SkRect::MakeWH(nodeResults->width(), nodeResults->height()));
    ToolUtils::draw_checkerboard(canvas, SK_ColorGRAY, SK_ColorLTGRAY, 10);
    canvas->restore();

    // Display filtered results in current canvas' location (assumed CTM is set for this node)
    canvas->drawImage(nodeResults, 0, 0);

    SkPaint line;
    line.setAntiAlias(true);
    line.setStyle(SkPaint::kStroke_Style);
    line.setStrokeWidth(3.f);

    // Text info
    canvas->save();
    canvas->translate(0, nodeResults->height());
    float textHeight = print_info(canvas, node);
    canvas->restore();

    // Border around filtered results + text info
    canvas->drawRect(SkRect::MakeWH(nodeResults->width(), nodeResults->height() + textHeight),
                     line);

    static const float kPad = 20.f;
    float x = nodeResults->width() + kPad;
    float y = 0;
    for (int i = 0; i < node.fInputNodes.count(); ++i) {
        // Line connecting this node to its child
        canvas->drawLine(nodeResults->width(), 0.5f * nodeResults->height(), // right of node
                         x, y + 0.5f * nodeResults->height(), line);         // left of child
        canvas->save();
        canvas->translate(x, y);
        y = draw_dag(canvas, nodeSurface, node.fInputNodes[i]);
        canvas->restore();
    }
    return SkMaxScalar(y, nodeResults->height() + textHeight + kPad);
}

static void draw_dag(SkCanvas* canvas, sk_sp<SkImageFilter> filter,
                     const SkRect& rect, const SkISize& surfaceSize) {
    // Get the current CTM, which includes all the viewer's UI modifications, which we want to
    // pass into our mock canvases for each DAG node.
    SkMatrix ctm = canvas->getTotalMatrix();

    canvas->save();
    // Reset the matrix so that the DAG layout and instructional text is fixed to the window.
    canvas->resetMatrix();

    // Process the image filter DAG to display intermediate results later on, which will apply the
    // provided CTM during draw_node calls.
    FilterNode dag = build_dag(ctm, rect, filter.get());

    sk_sp<SkSurface> nodeSurface =
            canvas->makeSurface(canvas->imageInfo().makeDimensions(surfaceSize));
    draw_dag(canvas, nodeSurface.get(), dag);

    canvas->restore();
}

class ImageFilterDAGSample : public Sample {
public:
    ImageFilterDAGSample() {}

    void onDrawContent(SkCanvas* canvas) override {
        static const SkRect kFilterRect = SkRect::MakeXYWH(20.f, 20.f, 60.f, 60.f);
        static const SkISize kFilterSurfaceSize = SkISize::Make(
                2 * (kFilterRect.fRight + kFilterRect.fLeft),
                2 * (kFilterRect.fBottom + kFilterRect.fTop));

        // Somewhat clunky, but we want to use the viewer calculated CTM in the mini surfaces used
        // per DAG node. The rotation matrix viewer calculates is based on the sample size so trick
        // it into calculating the right matrix for us w/ 1 frame latency.
        this->setSize(kFilterSurfaceSize.width(), kFilterSurfaceSize.height());

        // Make a large DAG
        //        /--- Color Filter <---- Blur <--- Offset
        // Merge <
        //        \--- Blur <--- Drop Shadow
        sk_sp<SkImageFilter> drop2 = SkImageFilters::DropShadow(
                10.f, 5.f, 3.f, 3.f, SK_ColorBLACK, nullptr);
        sk_sp<SkImageFilter> blur1 = SkImageFilters::Blur(2.f, 2.f, std::move(drop2));

        sk_sp<SkImageFilter> offset3 = SkImageFilters::Offset(-5.f, -5.f, nullptr);
        sk_sp<SkImageFilter> blur2 = SkImageFilters::Blur(4.f, 4.f, std::move(offset3));
        sk_sp<SkImageFilter> cf1 = SkImageFilters::ColorFilter(
                SkColorFilters::Blend(SK_ColorGRAY, SkBlendMode::kModulate), std::move(blur2));

        sk_sp<SkImageFilter> merge0 = SkImageFilters::Merge(std::move(blur1), std::move(cf1));

        draw_dag(canvas, std::move(merge0), kFilterRect, kFilterSurfaceSize);
    }

    SkString name() override { return SkString("ImageFilterDAG"); }

private:

    typedef Sample INHERITED;
};

DEF_SAMPLE(return new ImageFilterDAGSample();)
