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
import org.skia.androidkit.*;

// TODO: refactor to share w/ other activities
class RuntimeShaderRenderThread extends Thread {
    private android.view.Surface mAndroidSurface;
    private Surface              mSurface;
    private RuntimeShaderBuilder mBuilder;
    private boolean              mRunning;

    private static final String TAG = "*** AK RenderThread";
    private static final String SkSLShader =
        "uniform half u_time;                                  " +
        "uniform half u_w;                                     " +
        "uniform half u_h;                                     " +

        "float f(vec3 p) {                                     " +
        "   p.z -= u_time * 10.;                               " +
        "   float a = p.z * .1;                                " +
        "   p.xy *= mat2(cos(a), sin(a), -sin(a), cos(a));     " +
        "   return .1 - length(cos(p.xy) + sin(p.yz));         " +
        "}                                                     " +

        "half4 main(vec2 fragcoord) {                          " +
        "   vec3 d = .5 - fragcoord.xy1 / u_h;                 " +
        "   vec3 p=vec3(0);                                    " +
        "   for (int i = 0; i < 32; i++) p += f(p) * d;        " +
        "   return ((sin(p) + vec3(2, 5, 9)) / length(p)).xyz1;" +
        "}";

    private static final String SkSLShader2 =
        "uniform half u_time;                             " +
        "uniform half u_w;                                " +
        "uniform half u_h;                                " +

        "half4 main(vec2 fragcoord) {                     " +
        "   vec3 c;" +
	    "   float l;" +
        "   float z=u_time;" +
	    "   for(int i=0;i<3;i++) {" +
		"       vec2 p=fragcoord.xy/vec2(u_w,u_h);" +
		"       vec2 uv=p;" +
		"       p-=.5;" +
		"       p.x*=u_w/u_h;" +
		"       z+=.07;" +
		"       l=length(p);" +
		"       uv+=p/l*(sin(z)+1.)*abs(sin(l*9.-z*2.));" +
		"       c[i]=.01/length(abs(mod(uv,1.)-.5));" +
	    "   }" +
	    "   return half4(c/l,u_time);" +
        "}";

    public RuntimeShaderRenderThread(android.view.Surface surface) {
        mAndroidSurface = surface;
        // TODO: this is too slow without GPU acceleration
        // mBuilder = new RuntimeShaderBuilder(SkSLShader);
        mBuilder = new RuntimeShaderBuilder(SkSLShader2);
    }

    public void finish() {
        mRunning = false;
    }

    @Override
    public void run() {
        mRunning = true;

        Log.d(TAG, "start");

        long time_base = java.lang.System.currentTimeMillis();

        // TODO: convert to native AK surface.
        while (mRunning) {
            android.graphics.Canvas android_canvas = mAndroidSurface.lockHardwareCanvas();

            int w = android_canvas.getWidth(),
                h = android_canvas.getHeight();

            android.graphics.Bitmap bm =
                    android.graphics.Bitmap.createBitmap(w, h,
                                                         android.graphics.Bitmap.Config.ARGB_8888,
                                                         true);
            Surface surface = new Surface(bm);
            renderFrame(surface.getCanvas(),
                        (double)(java.lang.System.currentTimeMillis() - time_base) / 1000,
                        w, h);
            surface.release();

            android_canvas.drawBitmap(bm, 0, 0, new android.graphics.Paint());

            mAndroidSurface.unlockCanvasAndPost(android_canvas);
        }

        Log.d(TAG, "finish");
    }

    private void renderFrame(Canvas canvas, double t, int canvas_width, int canvas_height) {
        final float kWidth  = 1000,
                    kHeight = 1000,
                    kSpeed  = 40;

        Paint p = new Paint();
        p.setShader(mBuilder.setUniform("u_time", (float)t)
                            .setUniform("u_w", canvas_width)
                            .setUniform("u_h", canvas_height)
                            .makeShader());

        canvas.drawRect(0, 0, canvas_width, canvas_height, p);
    }
}

public class RuntimeShaderActivity extends Activity implements SurfaceHolder.Callback {
    static {
        System.loadLibrary("androidkit");
    }

    private RuntimeShaderRenderThread mRenderThread;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_animation);

        SurfaceView sv = findViewById(R.id.surfaceView);
        sv.getHolder().addCallback(this);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        if (mRenderThread != null) {
            mRenderThread.finish();
            try {
                mRenderThread.join();
            } catch (InterruptedException e) {}
        }

        mRenderThread = new RuntimeShaderRenderThread(holder.getSurface());;
        mRenderThread.start();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        if (mRenderThread != null) {
            mRenderThread.finish();
            try {
                mRenderThread.join();
            } catch (InterruptedException e) {}
        }
    }
}
