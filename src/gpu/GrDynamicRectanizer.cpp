/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrDynamicRectanizer.h"

#include "src/core/SkIPoint16.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrOnFlushResourceProvider.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRectanizerPow2.h"
#include "src/gpu/GrRectanizerSkyline.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrSurfaceProxyView.h"

// Each Node covers a sub-rectangle of the final atlas. When a GrDynamicRectanizer runs out of room,
// we create a new Node the same size as all combined nodes in the atlas as-is, and then place the
// new Node immediately below or beside the others (thereby doubling the size of the
// GyDynamicAtlas).
class GrDynamicRectanizer::Node {
public:
    Node(Node* previous, GrRectanizer* rectanizer, int x, int y)
            : fPrevious(previous), fRectanizer(rectanizer), fX(x), fY(y) {}

    Node* previous() const { return fPrevious; }

    bool addRect(int w, int h, SkIPoint16* loc) {
        // Pad all paths except those that are expected to take up an entire physical texture.
        if (w < fRectanizer->width()) {
            w = std::min(w + kPadding, fRectanizer->width());
        }
        if (h < fRectanizer->height()) {
            h = std::min(h + kPadding, fRectanizer->height());
        }
        if (!fRectanizer->addRect(w, h, loc)) {
            return false;
        }
        loc->fX += fX;
        loc->fY += fY;
        return true;
    }

private:
    Node* const fPrevious;
    GrRectanizer* const fRectanizer;
    const int fX, fY;
};

void GrDynamicRectanizer::reset(SkISize initialSize) {
    fNodeAllocator.reset();
    fWidth = std::min(SkNextPow2(initialSize.width()), fMaxAtlasSize);
    fHeight = std::min(SkNextPow2(initialSize.height()), fMaxAtlasSize);
    fTopNode = nullptr;
    fDrawBounds.setEmpty();
}

GrDynamicRectanizer::Node* GrDynamicRectanizer::makeNode(
        Node* previous, int l, int t, int r, int b) {
    int width = r - l;
    int height = b - t;
    GrRectanizer* rectanizer =
            (fRectanizerAlgorithm == RectanizerAlgorithm::kSkyline)
                    ? (GrRectanizer*)fNodeAllocator.make<GrRectanizerSkyline>(width, height)
                    : fNodeAllocator.make<GrRectanizerPow2>(width, height);
    return fNodeAllocator.make<Node>(previous, rectanizer, l, t);
}

bool GrDynamicRectanizer::addRect(int width, int height, SkIPoint16* location) {
    if (!this->internalPlaceRect(width, height, location)) {
        return false;
    }

    fDrawBounds.fWidth = std::max(fDrawBounds.width(), location->x() + width);
    fDrawBounds.fHeight = std::max(fDrawBounds.height(), location->y() + height);
    return true;
}

bool GrDynamicRectanizer::internalPlaceRect(int w, int h, SkIPoint16* loc) {
    if (std::max(h, w) > fMaxAtlasSize) {
        return false;
    }
    if (std::min(h, w) <= 0) {
        loc->set(0, 0);
        return true;
    }

    if (!fTopNode) {
        if (w > fWidth) {
            fWidth = std::min(SkNextPow2(w), fMaxAtlasSize);
        }
        if (h > fHeight) {
            fHeight = std::min(SkNextPow2(h), fMaxAtlasSize);
        }
        fTopNode = this->makeNode(nullptr, 0, 0, fWidth, fHeight);
    }

    for (Node* node = fTopNode; node; node = node->previous()) {
        if (node->addRect(w, h, loc)) {
            return true;
        }
    }

    // The rect didn't fit. Grow the atlas and try again.
    do {
        if (fWidth >= fMaxAtlasSize && fHeight >= fMaxAtlasSize) {
            return false;
        }
        if (fHeight <= fWidth) {
            int top = fHeight;
            fHeight = std::min(fHeight * 2, fMaxAtlasSize);
            fTopNode = this->makeNode(fTopNode, 0, top, fWidth, fHeight);
        } else {
            int left = fWidth;
            fWidth = std::min(fWidth * 2, fMaxAtlasSize);
            fTopNode = this->makeNode(fTopNode, left, 0, fWidth, fHeight);
        }
    } while (!fTopNode->addRect(w, h, loc));

    return true;
}
