
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SKHITBOX_H_
#define SKHITBOX_H_

#include "SkBitmap.h"
#include "SkUnPreMultiply.h"

/* NOTE(chudy): It's possible that this class can be entirely static similar to
 * SkObjectParser. We will have to pass in the fHitBox void * every call.
 */
class SkHitBox {
public:
    SkHitBox();
    ~SkHitBox();

    /**
        Allocates enough space in memory for our hitbox pointer to contain
        a layer value for every pixel. Initializes every value to 0.
     */
    void alloc(int width, int height);

    /**
        Compares the new SkBitmap compared to the SkBitmap from the last
        call. Updates our hitbox with the draw command number if different.
     */
    void updateHitBox(SkBitmap* newBitmap, int layer);

    /**
        Compares point x,y in the new bitmap compared to the saved previous
        one. Updates hitpoint with the draw command number if different.
     */
    void updateHitPoint(SkBitmap* newBitmap, int layer);

    /**
        Sets the target hitpoint we are attempting to find the layer of.
     */
    void setHitPoint(int x, int y) {
        fX = x;
        fY = y;
        fLayer = 0;
    }

    /**
        Returns a pointer to the start of the hitbox.
     */
    int* getHitBox() {
        return fHitBox;
    }

    /**
        Returns the layer numbr corresponding to the point (fX, fY) in this class.
     */
    int getPoint() {
        return fLayer;
    }

    /**
        Checks to see if a mouse click has been passed in.
     */
    bool pointIsSet() {
        return !(fX == -1 && fY == -1);
    }

private:
    SkBitmap fPrev;
    int* fHitBox;
    int fX;
    int fY;
    int fLayer;
};


#endif /* SKHITBOX_H_ */
