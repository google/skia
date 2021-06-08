/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit;

import android.support.annotation.Nullable;
import java.lang.IllegalArgumentException;
import org.skia.androidkit.Color;
import org.skia.androidkit.Shader;
import org.skia.androidkit.TileMode;

public class Gradient {
    public static Shader makeLinear(float x0, float y0, float x1, float y1, Color[] colors,
                                    float[] pos, TileMode tm,
                                    @Nullable Matrix localMatrix) throws IllegalArgumentException {
        if (colors.length != pos.length) {
            throw new IllegalArgumentException("Expecting equal-length colors and positions.");
        }

        float[] fcolors = new float[colors.length * 4];
        for (int i = 0; i < colors.length; ++i) {
            fcolors[4*i + 0] = colors[i].r();
            fcolors[4*i + 1] = colors[i].g();
            fcolors[4*i + 2] = colors[i].b();
            fcolors[4*i + 3] = colors[i].a();
        }

        long nativeLocalMatrix = localMatrix != null ? localMatrix.getNativeInstance() : 0;

        return new Shader(
            nMakeLinear(x0, y0, x1, y1, fcolors, pos, tm.ordinal(), nativeLocalMatrix));
    }

    public static Shader makeLinear(float x0, float y0, float x1, float y1, Color[] colors,
                                    float[] pos, TileMode tm) throws IllegalArgumentException {
        return makeLinear(x0, y0, x1, y1, colors, pos, tm, null);
    }

    private static native long nMakeLinear(float x0, float y0, float x1, float y1,
                                           float[] colors, float[] pos, int tilemode,
                                           long nativeLocalMatrix);
}
