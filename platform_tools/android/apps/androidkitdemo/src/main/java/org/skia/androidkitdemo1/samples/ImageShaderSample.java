/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkitdemo1.samples;

import android.content.res.Resources;

import org.skia.androidkit.Canvas;
import org.skia.androidkit.Image;
import org.skia.androidkit.Paint;
import org.skia.androidkit.SamplingOptions;
import org.skia.androidkit.Shader;
import org.skia.androidkit.TileMode;

import org.skia.androidkitdemo1.samples.Sample;

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
