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
import org.skia.androidkit.SkottieAnimation;

import org.skia.androidkitdemo1.samples.Sample;

public class SkottieSample implements Sample {
    private Paint            mClearPaint = new Paint().setColor(0.8f, 0.8f, 0.8f, 1);
    private SkottieAnimation mAnimation;

    public SkottieSample(Resources res, int resId) {
        String json = "";

        try {
            InputStream is = res.openRawResource(resId);
            byte[] data = new byte[is.available()];
            is.read(data);
            json = new String(data);
        } catch (Exception e) {}

        mAnimation = new SkottieAnimation(json);
    }

    public void render(Canvas canvas, long ms, float left, float top, float right, float bottom) {
        canvas.drawRect(left, top, right, bottom, mClearPaint);

        double t = (double)ms / 1000 % mAnimation.getDuration();
        mAnimation.seekTime(t);

        float w = right - left,
              h = bottom - top,
              s = Math.min(w / mAnimation.getWidth(),
                           h / mAnimation.getHeight());

        canvas.save();
        canvas.concat(new Matrix().translate(left + (w - s*mAnimation.getWidth() )/2,
                                             top  + (h - s*mAnimation.getHeight())/2)
                                  .scale(s, s));

        mAnimation.render(canvas);
        canvas.restore();
    }
}
