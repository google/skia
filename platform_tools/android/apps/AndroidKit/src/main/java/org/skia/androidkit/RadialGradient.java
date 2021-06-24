/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit;

import android.support.annotation.Nullable;
import java.lang.IllegalArgumentException;

public class RadialGradient extends Gradient {
    public RadialGradient(float x, float y, float r, int[] colors, float[] pos, TileMode tm,
                          @Nullable Matrix localMatrix) throws IllegalArgumentException {
        super(colors, pos, tm, localMatrix,
                (c, p, t, m) -> nMakeRadial(x, y, r, c, p, t, m));
    }

    public RadialGradient(float x, float y, float r, int[] colors,
                          float[] pos, TileMode tm) throws IllegalArgumentException {
        this(x, y, r, colors, pos, tm, null);
    }

    public RadialGradient(float x, float y, float r, float[] colors, float[] pos, TileMode tm,
                          @Nullable Matrix localMatrix) throws IllegalArgumentException {
        super(colors, pos, tm, localMatrix,
                (c, p, t, m) -> nMakeRadial(x, y, r, c, p, t, m));
    }

    public RadialGradient(float x, float y, float r, float[] colors,
                          float[] pos, TileMode tm) throws IllegalArgumentException {
        this(x, y, r, colors, pos, tm, null);
    }

    private static native long nMakeRadial(float x, float y, float r, float[] colors, float[] pos,
                                           int tilemode, long nativeLocalMatrix);
}
