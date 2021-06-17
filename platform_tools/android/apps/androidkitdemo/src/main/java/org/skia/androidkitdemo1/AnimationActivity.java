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
    @Override
    protected void onSurfaceInitialized(Surface surface) {}

    @Override
    protected void onRenderFrame(Canvas canvas, long ms) {
        final float kWidth  = 400,
                    kHeight = 200,
                    kSpeed  = 4;

        canvas.drawColor(0xffffffe0);

        Color[] colors = { new Color(1,0,0,1),
                           new Color(0,1,0,1),
                           new Color(0,0,1,1)};
        float[] pos = {0, 0.5f, 1};
        Shader gradient = new LinearGradient(0, 0, canvas.getWidth(), 0,
                                             colors, pos, TileMode.CLAMP);

        Paint p = new Paint().setShader(gradient);

        float x = (float)(java.lang.Math.cos(ms * kSpeed / 1000) + 1) * canvas.getWidth()/2;
        canvas.drawRect(x - kWidth/2, (canvas.getHeight() - kHeight)/2,
                        x + kWidth/2, (canvas.getHeight() + kHeight)/2, p);
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
