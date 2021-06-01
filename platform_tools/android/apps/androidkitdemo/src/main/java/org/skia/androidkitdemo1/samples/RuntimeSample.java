/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkitdemo1.samples;

import android.content.res.Resources;

import java.io.InputStream;

import org.skia.androidkit.Canvas;
import org.skia.androidkit.Matrix;
import org.skia.androidkit.Paint;
import org.skia.androidkit.RuntimeShaderBuilder;

import org.skia.androidkitdemo1.samples.Sample;

public class RuntimeSample implements Sample {
    private RuntimeShaderBuilder mShaderBuilder;

    public RuntimeSample(Resources res, int resId) {
        String sksl = "";
        try {
            InputStream is = res.openRawResource(resId);
            byte[] data = new byte[is.available()];
            is.read(data);

            sksl = new String(data);
        } catch (Exception e) {}

        mShaderBuilder = new RuntimeShaderBuilder(sksl);
    }

    public void render(Canvas canvas, long ms, float left, float top, float right, float bottom) {
        mShaderBuilder.setUniform("u_time", ms / 1000.0f)
                      .setUniform("u_w", right - left)
                      .setUniform("u_h", bottom - top);

        Paint paint = new Paint().setShader(mShaderBuilder.makeShader());

        canvas.save();
        canvas.concat(new Matrix().translate(left, top));
        canvas.drawRect(0, 0, right - left, bottom - top, paint);
        canvas.restore();
    }
}
