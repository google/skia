/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkitdemo1;

import android.app.Activity;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import org.skia.androidkit.*;
import org.skia.androidkit.util.*;

class AnimationRenderer extends SurfaceRenderer {
    private Shader mLinearGradient,
                   mRadialGradient,
                   mConicalGradient,
                   mSweepGradient;
    private ColorFilter mColorFilter = new MatrixColorFilter(new float[]{
        0.75f, 0, 0, 0,    0,
            0, 1, 0, 0, 0.5f,
            0, 0, 1, 0,    0,
            0, 0, 0, 1,    0,
    });

    @Override
    protected void onSurfaceInitialized(Surface surface) {
        float sw = surface.getWidth(),
              sh = surface.getHeight();

        float[] colors1 = {
                            1,0,0,1,
                            0,1,0,1,
                            0,0,1,1
                          };
        int[]   colors2 = {
                            0xffffff00,
                            0xff00ffff,
                            0xffff00ff
                          };

        float[] pos = {0, 0.5f, 1};

        mLinearGradient = new LinearGradient(0, 0, sw/4, 0,
                                             colors1, pos, TileMode.REPEAT);
        mRadialGradient = new RadialGradient(sw/2, sh/4, Math.min(sw, sh)/2,
                                             colors2, pos, TileMode.REPEAT);
        mConicalGradient = new TwoPointConicalGradient(sw/4, sh/2, sw/4,
                                                       sw/2, sh/2, sw/2,
                                                       colors1, pos, TileMode.MIRROR);
        mSweepGradient = new SweepGradient(sw/2, sh/4, 0, 90, colors2, pos, TileMode.REPEAT);
    }

    @Override
    protected void onRenderFrame(Canvas canvas, long ms) {
        final float kWidth  = 400,
                    kHeight = 200,
                    kSpeed  = 4;

        canvas.drawColor(0xffffffe0);

        float cw = canvas.getWidth(),
              ch = canvas.getHeight(),
            osc1 = (float)(java.lang.Math.cos(ms * kSpeed / 1000)),
            osc2 = (float)(java.lang.Math.sin(ms * kSpeed / 1000));

        drawRect(canvas, (1 + osc1)*cw/2, ch/2, mLinearGradient);
        drawRect(canvas, (1 - osc1)*cw/2, ch/2, mConicalGradient);
        drawRect(canvas, cw/2, (1 + osc2)*ch/2, mRadialGradient);
        drawRect(canvas, cw/2, (1 - osc2)*ch/2, mSweepGradient);
    }

    private void drawRect(Canvas canvas, float cx, float cy, Shader shader) {
        final float kWidth  = 400,
                    kHeight = 200;

        canvas.drawRect(cx - kWidth/2, cy - kHeight/2, cx + kWidth/2, cy + kHeight/2,
                        new Paint().setShader(shader).setColorFilter(mColorFilter));
    }
}

public class AnimationActivity extends Activity {
    static {
        System.loadLibrary("androidkit");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_animation);

        SurfaceView sv = findViewById(R.id.surfaceView);
        sv.getHolder().addCallback(new AnimationRenderer());
    }
}
