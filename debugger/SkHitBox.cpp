
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkHitBox.h"

SkHitBox::SkHitBox() {
    fHitBox = NULL;
    fX = -1;
    fY = -1;
    fLayer = -1;
}

SkHitBox::~SkHitBox() {}

void SkHitBox::alloc(int width, int height) {
    free(fHitBox);
    int length = width * height;
    fHitBox = (int*) malloc(length * sizeof(int));
    for (int i = 0; i < length; i++) {
        fHitBox[i] = 0;
    }
}

void SkHitBox::updateHitBox(SkBitmap* newBitmap, int layer) {
        int length = fPrev.width() * fPrev.height();
        int* prevBase = (int*)fPrev.getPixels();
        int* currBase = (int*)newBitmap->getPixels();

        for (int i = 0; i < length; i++) {
            if (SkUnPreMultiply::PMColorToColor(prevBase[i]) !=
                    SkUnPreMultiply::PMColorToColor(currBase[i])) {
                fHitBox[i] = layer;
            }
        }
        if (fPrev.empty()) {
            alloc(newBitmap->width(), newBitmap->height());
            fPrev.setConfig(SkBitmap::kARGB_8888_Config, newBitmap->width(), newBitmap->height());
            fPrev.allocPixels();
        }
        newBitmap->deepCopyTo(&fPrev, SkBitmap::kARGB_8888_Config);
}

void SkHitBox::updateHitPoint(SkBitmap* newBitmap, int layer) {
    int* prevBase = (int*)fPrev.getPixels();
    int* currBase = (int*)newBitmap->getPixels();
    int pixel = fY * fPrev.width() + fX;

    if (pointIsSet() && !fPrev.empty()) {
        if (SkUnPreMultiply::PMColorToColor(prevBase[pixel]) !=
                SkUnPreMultiply::PMColorToColor(currBase[pixel])) {
            fLayer = layer;
        }
    }
    if (fPrev.empty()) {
        alloc(newBitmap->width(), newBitmap->height());
        fPrev.setConfig(SkBitmap::kARGB_8888_Config, newBitmap->width(), newBitmap->height());
        fPrev.allocPixels();
    }
    newBitmap->deepCopyTo(&fPrev, SkBitmap::kARGB_8888_Config);
}
