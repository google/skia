/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkitdemo1;

import android.app.Activity;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import org.skia.androidkit.Canvas;
import org.skia.androidkit.Color;
import org.skia.androidkit.Matrix;
import org.skia.androidkit.Paint;
import org.skia.androidkit.Surface;

import static java.lang.Math.tan;

class Face {
    private float rotX;
    private float rotY;
    public Color color;

    Face(float rotX, float rotY, Color color) {
        this.rotX = rotX;
        this.rotY = rotY;
        this.color = color;
    }

    Matrix asMatrix(float scale) {
        return new Matrix().rotateY(rotY).rotateX(rotX).translate(0, 0, scale);
    }
}

class CubeRenderThread extends Thread {
    private android.view.Surface mASurface;
    private boolean mRunning;
    // TODO: make these relative to surface size
    private float mCubeSideLength = 500;
    private int DX = 200;
    private int DY = 300;

    private float fAngle = (float)Math.PI / 12;
    private float eyeZ = (float)(1.0f/tan(fAngle/2) - 1);

    private Matrix cam = Matrix.makeLookAt(0, 0, eyeZ, 0, 0, 0, 0, 1, 0);
    private Matrix perspective = Matrix.makePerspective(0.05f, 4, fAngle);
    private Matrix viewport;

    private Paint mPaint;

    private final float rot = (float) Math.PI;
    private Face[] faces = {new Face(0, 0, new Color(1, 0, 0, 1)),
                            new Face(0, rot, new Color(0, 1, 0, 1)),
                            new Face(rot/2, 0, new Color(0, 0, 1, 1)),
                            new Face(-rot/2, 0, new Color(1, 1, 0, 1)),
                            new Face(0, rot/2, new Color(0, 1, 1, 1)),
                            new Face(0, -rot/2, new Color(0, 0, 0, 1))};

    private static final String TAG = "*** AK CubeRenderThread";

    public CubeRenderThread(android.view.Surface surface) {
        mASurface = surface;
        mPaint = new Paint();
        mPaint.setColor(new Color(0, 1, 1, 1));
        mPaint.setStroke(false);
        mPaint.setStrokeWidth(10);

    }

    public void finish(){
        mRunning = false;
    }

    @Override
    public void run() {
        mRunning = true;
        long time_base = java.lang.System.currentTimeMillis();

        Surface surface = Surface.CreateGL(mASurface);
        viewport = new Matrix().translate(mCubeSideLength/2, mCubeSideLength/2, 0)
                               .scale(mCubeSideLength/2, mCubeSideLength/2, surface.getWidth());
        while (mRunning) {
            float t = (float)(java.lang.System.currentTimeMillis() - time_base) / 1000;
            float speed = 0.5f;
            float rads = t * speed % (float)(2 * Math.PI);
            renderFrame(surface, rads);
            surface.flushAndSubmit();
        }

        surface.release();
    }

    private void renderFrame(Surface surface, float rads) {
        Canvas canvas = surface.getCanvas();
        // clear canvas
        canvas.drawColor(0xffffffff);

        canvas.save();
        canvas.concat(new Matrix().translate(DX, DY, 0));
        canvas.concat(viewport.preConcat(perspective).preConcat(cam).preConcat(Matrix.makeInverse(viewport)));

        for (Face f : faces) {
            //TODO: auto restore
            canvas.save();
            Matrix trans = new Matrix().translate(mCubeSideLength/2, mCubeSideLength/2, 0);
            Matrix m = new Matrix().rotateY(rads).rotateZ(rads).preConcat(f.asMatrix(mCubeSideLength/2));

            canvas.concat(trans);
            Matrix localToWorld = m.preConcat(Matrix.makeInverse(trans));
            canvas.concat(localToWorld);

            if (front(canvas.getLocalToDevice())) {
                mPaint.setColor(f.color);
                canvas.drawRect(0, 0, mCubeSideLength, mCubeSideLength, mPaint);
            }
            canvas.restore();
        }
        canvas.restore();
    }

    private boolean front(Matrix m) {
        Matrix m2;
        try {
            m2 = Matrix.makeInverse(m);
        } catch (RuntimeException e) {
            m2 = new Matrix();
        }
        return m2.getAtRowCol(2, 2) > 0;
    }
}
public class CubeActivity extends Activity implements SurfaceHolder.Callback {
    private CubeRenderThread mRenderThread;
    static {
        System.loadLibrary("androidkit");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_cube);

        SurfaceView sv = findViewById(R.id.surfaceView);
        sv.getHolder().addCallback(this);
    }

    @Override
    public void surfaceCreated(@NonNull SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
        if (mRenderThread != null) {
            mRenderThread.finish();
            try {
                mRenderThread.join();
            } catch (InterruptedException e) {}
        }

        mRenderThread = new CubeRenderThread(holder.getSurface());;
        mRenderThread.start();
    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {
        if (mRenderThread != null) {
            mRenderThread.finish();
            try {
                mRenderThread.join();
            } catch (InterruptedException e) {}
        }
    }
}
