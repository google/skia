/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.jetskidemo.samples;

import android.content.res.Resources;

import org.skia.jetski.Canvas;
import org.skia.jetski.Image;
import org.skia.jetski.Paint;
import org.skia.jetski.SamplingOptions;
import org.skia.jetski.Shader;
import org.skia.jetski.TileMode;

public class ImageShaderSample implements Sample {
    private Paint mPaint = new Paint();

    public ImageShaderSample(Resources res, int resId) {
        try {
            Image image = Image.fromStream(res.openRawResource(resId));
            Shader shader =
                image.makeShader(TileMode.REPEAT, TileMode.REPEAT,
                                 new SamplingOptions(SamplingOptions.FilterMode.LINEAR));
            mPaint.setShader(shader);
        } catch (Exception e) {}
    }

    public void render(Canvas canvas, long t, float left, float top, float right, float bottom) {
        canvas.drawRect(left, top, right, bottom, mPaint);
    }
}
