/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkRecordOpts.h"

#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkMath.h"
#include "include/private/base/SkTemplates.h"
#include "src/core/SkRecord.h"
#include "src/core/SkRecordPattern.h"
#include "src/core/SkRecords.h"

#include <cstdint>
#include <optional>

using namespace SkRecords;

// Most of the optimizations in this file are pattern-based.  These are all defined as structs with:
//   - a Match typedef
//   - a bool onMatch(SkRceord*, Match*, int begin, int end) method,
//     which returns true if it made changes and false if not.

// Run a pattern-based optimization once across the SkRecord, returning true if it made any changes.
// It looks for spans which match Pass::Match, and when found calls onMatch() with that pattern,
// record, and [begin,end) span of the commands that matched.
template <typename Pass>
static bool apply(Pass* pass, SkRecord* record) {
    typename Pass::Match match;
    bool changed = false;
    int begin, end = 0;

    while (match.search(record, &begin, &end)) {
        changed |= pass->onMatch(record, &match, begin, end);
    }
    return changed;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// Turns the logical NoOp Save and Restore in Save-Draw*-Restore patterns into actual NoOps.
struct SaveOnlyDrawsRestoreNooper {
    typedef Pattern<Is<Save>,
                    Greedy<Or<Is<NoOp>, IsDraw>>,
                    Is<Restore>>
        Match;

    bool onMatch(SkRecord* record, Match*, int begin, int end) {
        record->replace<NoOp>(begin);  // Save
        record->replace<NoOp>(end-1);  // Restore
        return true;
    }
};

static bool fold_opacity_layer_color_to_paint(const SkPaint* layerPaint,
                                              bool isSaveLayer,
                                              SkPaint* paint) {
    // We assume layerPaint is always from a saveLayer.  If isSaveLayer is
    // true, we assume paint is too.

    // The alpha folding can proceed if the filter layer paint does not have properties which cause
    // the resulting filter layer to be "blended" in complex ways to the parent layer.
    // TODO: most likely only some xfer modes are the hard constraints
    if (!paint->isSrcOver()) {
        return false;
    }

    if (!isSaveLayer && paint->getImageFilter()) {
        // For normal draws, the paint color is used as one input for the color for the draw. Image
        // filter will operate on the result, and thus we can not change the input.
        // For layer saves, the image filter is applied to the layer contents. The layer is then
        // modulated with the paint color, so it's fine to proceed with the fold for saveLayer
        // paints with image filters.
        return false;
    }

    if (paint->getColorFilter()) {
        // Filter input depends on the paint color.

        // Here we could filter the color if we knew the draw is going to be uniform color.  This
        // should be detectable as drawPath/drawRect/.. without a shader being uniform, while
        // drawBitmap/drawSprite or a shader being non-uniform. However, current matchers don't
        // give the type out easily, so just do not optimize that at the moment.
        return false;
    }

    if (layerPaint) {
        const uint32_t layerColor = layerPaint->getColor();
        // The layer paint color must have only alpha component.
        if (SK_ColorTRANSPARENT != SkColorSetA(layerColor, SK_AlphaTRANSPARENT)) {
            return false;
        }

        // The layer paint can not have any effects.
        if (layerPaint->getPathEffect()  ||
            layerPaint->getShader()      ||
            !layerPaint->isSrcOver()     ||
            layerPaint->getMaskFilter()  ||
            layerPaint->getColorFilter() ||
            layerPaint->getImageFilter()) {
            return false;
        }
        paint->setAlpha(SkMulDiv255Round(paint->getAlpha(), SkColorGetA(layerColor)));
    }

    return true;
}

// Turns logical no-op Save-[non-drawing command]*-Restore patterns into actual no-ops.
struct SaveNoDrawsRestoreNooper {
    // Greedy matches greedily, so we also have to exclude Save and Restore.
    // Nested SaveLayers need to be excluded, or we'll match their Restore!
    typedef Pattern<Is<Save>,
                    Greedy<Not<Or<Is<Save>,
                                  Is<SaveLayer>,
                                  Is<Restore>,
                                  IsDraw>>>,
                    Is<Restore>>
        Match;

    bool onMatch(SkRecord* record, Match*, int begin, int end) {
        // The entire span between Save and Restore (inclusively) does nothing.
        for (int i = begin; i < end; i++) {
            record->replace<NoOp>(i);
        }
        return true;
    }
};
void SkRecordNoopSaveRestores(SkRecord* record) {
    SaveOnlyDrawsRestoreNooper onlyDraws;
    SaveNoDrawsRestoreNooper noDraws;

    // Run until they stop changing things.
    while (apply(&onlyDraws, record) || apply(&noDraws, record));
}

#ifndef SK_BUILD_FOR_ANDROID_FRAMEWORK
static bool effectively_srcover(const SkPaint* paint) {
    if (!paint || paint->isSrcOver()) {
        return true;
    }
    // src-mode with opaque and no effects (which might change opaqueness) is ok too.
    return !paint->getShader() && !paint->getColorFilter() && !paint->getImageFilter() &&
           0xFF == paint->getAlpha() && paint->asBlendMode() == SkBlendMode::kSrc;
}

// For some SaveLayer-[drawing command]-Restore patterns, merge the SaveLayer's alpha into the
// draw, and no-op the SaveLayer and Restore.
struct SaveLayerDrawRestoreNooper {
    // Note that we use IsSingleDraw here, to avoid matching drawAtlas, drawVertices, etc...
    // Those operations (can) draw multiple, overlapping primitives that blend with each other.
    // Applying this operation to them changes their behavior. (skbug.com/14554)
    typedef Pattern<Is<SaveLayer>, IsSingleDraw, Is<Restore>> Match;

    bool onMatch(SkRecord* record, Match* match, int begin, int end) {
        if (match->first<SaveLayer>()->backdrop) {
            // can't throw away the layer if we have a backdrop
            return false;
        }

        if (!match->first<SaveLayer>()->filters.empty()) {
            // Our optimizations don't handle the filter list correctly - don't bother trying
            return false;
        }

        // A SaveLayer's bounds field is just a hint, so we should be free to ignore it.
        SkPaint* layerPaint = match->first<SaveLayer>()->paint;
        SkPaint* drawPaint = match->second<SkPaint>();

        if (nullptr == layerPaint && effectively_srcover(drawPaint)) {
            // There wasn't really any point to this SaveLayer at all.
            return KillSaveLayerAndRestore(record, begin);
        }

        if (drawPaint == nullptr) {
            // We can just give the draw the SaveLayer's paint.
            // TODO(mtklein): figure out how to do this clearly
            return false;
        }

        if (!fold_opacity_layer_color_to_paint(layerPaint, false /*isSaveLayer*/, drawPaint)) {
            return false;
        }

        return KillSaveLayerAndRestore(record, begin);
    }

    static bool KillSaveLayerAndRestore(SkRecord* record, int saveLayerIndex) {
        record->replace<NoOp>(saveLayerIndex);    // SaveLayer
        record->replace<NoOp>(saveLayerIndex+2);  // Restore
        return true;
    }
};
void SkRecordNoopSaveLayerDrawRestores(SkRecord* record) {
    SaveLayerDrawRestoreNooper pass;
    apply(&pass, record);
}
#endif

/* For SVG generated:
  SaveLayer (non-opaque, typically for CSS opacity)
    Save
      ClipRect
      SaveLayer (typically for SVG filter)
      Restore
    Restore
  Restore
*/
struct SvgOpacityAndFilterLayerMergePass {
    typedef Pattern<Is<SaveLayer>, Is<Save>, Is<ClipRect>, Is<SaveLayer>,
                    Is<Restore>, Is<Restore>, Is<Restore>> Match;

    bool onMatch(SkRecord* record, Match* match, int begin, int end) {
        if (match->first<SaveLayer>()->backdrop) {
            // can't throw away the layer if we have a backdrop
            return false;
        }

        if (!match->first<SaveLayer>()->filters.empty() ||
            !match->fourth<SaveLayer>()->filters.empty()) {
            // Our optimizations don't handle the filter list correctly - don't bother trying
            return false;
        }

        SkPaint* opacityPaint = match->first<SaveLayer>()->paint;
        if (nullptr == opacityPaint) {
            // There wasn't really any point to this SaveLayer at all.
            return KillSaveLayerAndRestore(record, begin);
        }

        // This layer typically contains a filter, but this should work for layers with for other
        // purposes too.
        SkPaint* filterLayerPaint = match->fourth<SaveLayer>()->paint;
        if (filterLayerPaint == nullptr) {
            // We can just give the inner SaveLayer the paint of the outer SaveLayer.
            // TODO(mtklein): figure out how to do this clearly
            return false;
        }

        if (!fold_opacity_layer_color_to_paint(opacityPaint, true /*isSaveLayer*/,
                                               filterLayerPaint)) {
            return false;
        }

        return KillSaveLayerAndRestore(record, begin);
    }

    static bool KillSaveLayerAndRestore(SkRecord* record, int saveLayerIndex) {
        record->replace<NoOp>(saveLayerIndex);     // SaveLayer
        record->replace<NoOp>(saveLayerIndex + 6); // Restore
        return true;
    }
};

void SkRecordMergeSvgOpacityAndFilterLayers(SkRecord* record) {
    SvgOpacityAndFilterLayerMergePass pass;
    apply(&pass, record);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void SkRecordOptimize(SkRecord* record) {
    // This might be useful  as a first pass in the future if we want to weed
    // out junk for other optimization passes.  Right now, nothing needs it,
    // and the bounding box hierarchy will do the work of skipping no-op
    // Save-NoDraw-Restore sequences better than we can here.
    // As there is a known problem with this peephole and drawAnnotation, disable this.
    // If we want to enable this we must first fix this bug:
    //     https://bugs.chromium.org/p/skia/issues/detail?id=5548
//    SkRecordNoopSaveRestores(record);

    // Turn off this optimization completely for Android framework
    // because it makes the following Android CTS test fail:
    // android.uirendering.cts.testclasses.LayerTests#testSaveLayerClippedWithAlpha
#ifndef SK_BUILD_FOR_ANDROID_FRAMEWORK
    SkRecordNoopSaveLayerDrawRestores(record);
#endif
    SkRecordMergeSvgOpacityAndFilterLayers(record);

    record->defrag();
}
