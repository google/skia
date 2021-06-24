/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit;

import android.support.annotation.Nullable;
import java.lang.IllegalArgumentException;

public class LinearGradient extends Gradient {
    public LinearGradient(float x0, float y0, float x1, float y1, int[] colors,
                          float[] pos, TileMode tm,
                          @Nullable Matrix localMatrix) throws IllegalArgumentException {
        super(colors, pos, tm, localMatrix,
            (c, p, t, m) -> nMakeLinear(x0, y0, x1, y1, c, p, t, m));
    }

    public LinearGradient(float x0, float y0, float x1, float y1, int[] colors,
                          float[] pos, TileMode tm) throws IllegalArgumentException {
        this(x0, y0, x1, y1, colors, pos, tm, null);
    }

    public LinearGradient(float x0, float y0, float x1, float y1, float[] colors,
                          float[] pos, TileMode tm,
                          @Nullable Matrix localMatrix) throws IllegalArgumentException {
        super(colors, pos, tm, localMatrix,
            (c, p, t, m) -> nMakeLinear(x0, y0, x1, y1, c, p, t, m));
    }

    public LinearGradient(float x0, float y0, float x1, float y1, float[] colors,
                          float[] pos, TileMode tm) throws IllegalArgumentException {
        this(x0, y0, x1, y1, colors, pos, tm, null);
    }

    private static native long nMakeLinear(float x0, float y0, float x1, float y1,
                                           float[] colors, float[] pos, int tilemode,
                                           long nativeLocalMatrix);
}