/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit;

import android.support.annotation.Nullable;
import java.lang.IllegalArgumentException;

public class TwoPointConicalGradient extends Gradient {
    public TwoPointConicalGradient(float x0, float y0, float r0, float x1, float y1, float r1,
                                   int[] colors, float[] pos, TileMode tm,
                                   @Nullable Matrix localMatrix) throws IllegalArgumentException {
        super(colors, pos, tm, localMatrix,
                (c, p, t, m) -> nMakeTwoPointConical(x0, y0, r0, x1, y1, r1, c, p, t, m));
    }

    public TwoPointConicalGradient(float x0, float y0, float r0, float x1, float y1, float r1,
                                   int[] colors, float[] pos,
                                   TileMode tm) throws IllegalArgumentException {
        this(x0, y0, r0, x1, y1, r1, colors, pos, tm, null);
    }

    public TwoPointConicalGradient(float x0, float y0, float r0, float x1, float y1, float r1,
                                   float[] colors, float[] pos, TileMode tm,
                                   @Nullable Matrix localMatrix) throws IllegalArgumentException {
        super(colors, pos, tm, localMatrix,
                (c, p, t, m) -> nMakeTwoPointConical(x0, y0, r0, x1, y1, r1, c, p, t, m));
    }

    public TwoPointConicalGradient(float x0, float y0, float r0, float x1, float y1, float r1,
                                   float[] colors, float[] pos,
                                   TileMode tm) throws IllegalArgumentException {
        this(x0, y0, r0, x1, y1, r1, colors, pos, tm, null);
    }

    private static native long nMakeTwoPointConical(float x0, float y0, float r0,
                                                    float x1, float y1, float r1,
                                                    float[] colors, float[] pos, int tilemode,
                                                    long nativeLocalMatrix);
}
