/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit;

import android.support.annotation.Nullable;
import java.lang.IllegalArgumentException;

public class Gradient extends Shader {
    protected interface GradientFactory {
        long make(float[] colors, float[] pos, int tileMode, long nativeLocalMatrix);
    }

    protected Gradient(int[] colors, float[] pos, TileMode tm, Matrix lm,
                       GradientFactory gf) throws IllegalArgumentException {
        super(makeGradient(colors, pos, tm, lm, gf));
    }

    protected Gradient(float[] colors, float[] pos, TileMode tm, Matrix lm,
                       GradientFactory gf) throws IllegalArgumentException {
        super(makeGradient(colors, pos, tm, lm, gf));
    }

    private static long makeGradient(int[] colors, float[] pos, TileMode tm, Matrix lm,
                                     GradientFactory gf) throws IllegalArgumentException {
        if (colors.length != pos.length) {
            throw new IllegalArgumentException("Expecting equal-length colors and positions.");
        }

        float[] fcolors = new float[colors.length * 4];

        for (int i = 0; i < colors.length; ++i) {
            fcolors[4*i + 0] = ((colors[i] >> 16) & 0xff) / 255.0f;
            fcolors[4*i + 1] = ((colors[i] >>  8) & 0xff) / 255.0f;
            fcolors[4*i + 2] = ((colors[i] >>  0) & 0xff) / 255.0f;
            fcolors[4*i + 3] = ((colors[i] >> 24) & 0xff) / 255.0f;
        }

        return gf.make(fcolors, pos, tm.ordinal(), lm != null ? lm.getNativeInstance() : 0);
    }

    private static long makeGradient(float[] colors, float[] pos, TileMode tm, Matrix lm,
                                     GradientFactory gf) throws IllegalArgumentException {
        if (colors.length % 4 != 0) {
            throw new IllegalArgumentException("Colors length must be a multiple of 4.");
        }

        if (colors.length / 4 != pos.length) {
            throw new IllegalArgumentException("Colors must be 4x positions length.");
        }

        return gf.make(colors, pos, tm.ordinal(), lm != null ? lm.getNativeInstance() : 0);
    }
}
