/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.jetskidemo;

import android.app.Activity;
import android.content.res.Resources;
import android.os.Bundle;
import android.view.SurfaceView;

import org.skia.jetski.*;
import org.skia.jetski.util.*;
import org.skia.jetskidemo.samples.SkottieSample;

class SkottieAnimationRenderer extends SurfaceRenderer {
    private SkottieSample mSample;
    private float         mSurfaceWidth,
                          mSurfaceHeight;

    SkottieAnimationRenderer(Resources res, int resID) {
        mSample = new SkottieSample(res, resID);
    }

    @Override
    protected void onSurfaceInitialized(Surface surface) {
        mSurfaceWidth  = surface.getWidth();
        mSurfaceHeight = surface.getHeight();
    }

    @Override
    protected void onRenderFrame(Canvas canvas, long ms) {
        canvas.drawColor(1, 1, 1, 1);
        mSample.render(canvas, ms, 0, 0, mSurfaceWidth, mSurfaceHeight);
    }
}

public class SkottieAnimationActivity extends Activity {
    static {
        System.loadLibrary("jetski");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_animation);

        SurfaceView sv = findViewById(R.id.surfaceView);
        sv.getHolder().addCallback(new SkottieAnimationRenderer(getResources(), R.raw.im_thirsty));
    }
}
