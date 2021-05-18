/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkitdemo1;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import java.io.InputStream;
import org.skia.androidkit.*;
import org.skia.androidkit.util.*;

class SkottieAnimationRenderer extends SurfaceRenderer {
    private SkottieAnimation mAnimation;
    private Matrix           mAnimationMatrix;

    SkottieAnimationRenderer(SkottieAnimation animation) {
        mAnimation = animation;
    }

    @Override
    protected void onSurfaceInitialized(Surface surface) {
        // Scale to fit/center.
        float sx = surface.getWidth()  / mAnimation.getWidth();
        float sy = surface.getHeight() / mAnimation.getHeight();
        float s = Math.min(sx, sy);
        mAnimationMatrix = new Matrix()
            .translate((surface.getWidth()  - s * mAnimation.getWidth())  / 2,
                       (surface.getHeight() - s * mAnimation.getHeight()) / 2)
            .scale(s, s);
    }

    @Override
    protected void onRenderFrame(Canvas canvas, long ms) {
        double t = (double)ms / 1000 % mAnimation.getDuration();
        mAnimation.seekTime(t);

        canvas.save();
        canvas.concat(mAnimationMatrix);

        canvas.drawColor(1, 1, 1, 1);
        mAnimation.render(canvas);

        canvas.restore();
    }
}

public class SkottieAnimationActivity extends Activity {
    static {
        System.loadLibrary("androidkit");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_animation);

        SurfaceView sv = findViewById(R.id.surfaceView);

        try {
            InputStream is = getResources().openRawResource(R.raw.im_thirsty);
            byte[] data = new byte[is.available()];
            is.read(data);

            SkottieAnimation animation = new SkottieAnimation(new String(data));
            sv.getHolder().addCallback(new SkottieAnimationRenderer(animation));
        } catch (Exception e) {
            Log.e("AndroidKit", "Could not load animation resource: " + R.raw.im_thirsty);
        }
    }
}
