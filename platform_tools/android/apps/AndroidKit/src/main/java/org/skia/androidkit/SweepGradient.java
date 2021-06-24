/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit;

import android.support.annotation.Nullable;
import java.lang.IllegalArgumentException;

public class SweepGradient extends Gradient {
    public SweepGradient(float x, float y, float startAngle, float endAngle,
                         int[] colors, float[] pos, TileMode tm,
                         @Nullable Matrix localMatrix) throws IllegalArgumentException {
        super(colors, pos, tm, localMatrix,
                (c, p, t, m) -> nMakeSweep(x, y, startAngle, endAngle, c, p, t, m));
    }

    public SweepGradient(float x, float y, float startAngle, float endAngle,
                         int[] colors, float[] pos, TileMode tm) throws IllegalArgumentException {
        this(x, y, startAngle, endAngle, colors, pos, tm, null);
    }

    public SweepGradient(float x, float y, int[] colors,
                         float[] pos) throws IllegalArgumentException {
        this(x, y, 0, 360, colors, pos, TileMode.CLAMP, null);
    }

    public SweepGradient(float x, float y, float startAngle, float endAngle,
                         float[] colors, float[] pos, TileMode tm,
                         @Nullable Matrix localMatrix) throws IllegalArgumentException {
        super(colors, pos, tm, localMatrix,
                (c, p, t, m) -> nMakeSweep(x, y, startAngle, endAngle, c, p, t, m));
    }

    public SweepGradient(float x, float y, float startAngle, float endAngle,
                         float[] colors, float[] pos, TileMode tm) throws IllegalArgumentException {
        this(x, y, startAngle, endAngle, colors, pos, tm, null);
    }

    public SweepGradient(float x, float y, float[] colors,
                         float[] pos) throws IllegalArgumentException {
        this(x, y, 0, 360, colors, pos, TileMode.CLAMP, null);
    }

    private static native long nMakeSweep(float x, float y, float sa, float ea,
                                          float[] colors, float[] pos,
                                          int tilemode, long nativeLocalMatrix);
}
