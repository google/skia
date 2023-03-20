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
import org.skia.jetskidemo.samples.RuntimeSample;

class RuntimeShaderRenderer extends SurfaceRenderer {
    private RuntimeSample mSample;
    private float         mSurfaceWidth,
                          mSurfaceHeight;

    public RuntimeShaderRenderer(Resources res, int resID) {
        mSample = new RuntimeSample(res, resID);
    }

    @Override
    protected void onSurfaceInitialized(Surface surface) {
        mSurfaceWidth  = surface.getWidth();
        mSurfaceHeight = surface.getHeight();
    }

    @Override
    protected void onRenderFrame(Canvas canvas, long ms) {
        mSample.render(canvas, ms, 0, 0, mSurfaceWidth, mSurfaceHeight);
    }
}

public class RuntimeShaderActivity extends Activity {
    static {
        System.loadLibrary("jetski");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_animation);

        SurfaceView sv = findViewById(R.id.surfaceView);
        sv.getHolder().addCallback(new RuntimeShaderRenderer(getResources(),
                                                             R.raw.runtime_shader1));
    }
}
