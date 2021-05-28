/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkitdemo1;

import android.app.Activity;
import android.content.res.Resources;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import org.skia.androidkit.Canvas;
import org.skia.androidkit.Color;
import org.skia.androidkit.Image;
import org.skia.androidkit.Matrix;
import org.skia.androidkit.Paint;
import org.skia.androidkit.SamplingOptions;
import org.skia.androidkit.Shader;
import org.skia.androidkit.Surface;
import org.skia.androidkit.TileMode;
import org.skia.androidkit.util.SurfaceRenderer;

import org.skia.androidkitdemo1.samples.ImageShaderSample;
import org.skia.androidkitdemo1.samples.Sample;

import static java.lang.Math.tan;

// TODO: remove after all sides are migrated to something more interesting.
class SolidColorSample implements Sample {
    private Paint mPaint = new Paint();

    public SolidColorSample(float r, float g, float b, float a) {
        mPaint.setColor(r, g, b, a);
    }

    public void render(Canvas canvas, long t, float left, float top, float right, float bottom) {
        canvas.drawRect(left, top, right, bottom, mPaint);
    }
}

class Face {
    private float rotX;
    private float rotY;
    public Sample sample;

    Face(float rotX, float rotY, Sample sample) {
        this.rotX = rotX;
        this.rotY = rotY;
        this.sample = sample;
    }

    Matrix asMatrix(float scale) {
        return new Matrix().rotateY(rotY).rotateX(rotX).translate(0, 0, scale);
    }
}

// TODO: make this public?
class Vec3 {
    public float x, y, z;

    public Vec3(float x, float y, float z) { this.x = x; this.y = y; this.z = z; }

    public float length() { return (float)Math.sqrt(x*x + y*y + z*z); }

    public Vec3 normalize() {
        mul(1/length());
        return this;
    }

    public Vec3 add(float v) {
        x += v; y += v; z += v;
        return this;
    }

    public Vec3 mul(float v) {
        x *= v; y *= v; z *= v;
        return this;
    }

    public float dot(Vec3 v) {
        return x*v.x + y*v.y + z*v.z;
    }

    public Vec3 cross(Vec3 v) {
        float xx = y*v.z - z*v.y,
              yy = z*v.x - x*v.z,
              zz = x*v.y - y*v.x;
        x = xx; y = yy; z = zz;
        return this;
    }
};

class VSphereAnimator {
    private Matrix mRotMatrix = new Matrix();
    private Vec3   mRotAxis   = new Vec3(0, 1, 0);
    private float  mRotSpeed  = (float)Math.PI,
                   mCenterX,
                   mCenterY,
                   mRadius;

    public VSphereAnimator(float x, float y, float r) {
        mCenterX = x;
        mCenterY = y;
        mRadius  = r;
    }

    public void animate(float dt) {
        final float kDecay = 0.99f;

        mRotMatrix = new Matrix().rotate(mRotAxis.x, mRotAxis.y, mRotAxis.z, mRotSpeed * dt)
                                 .preConcat(mRotMatrix);
        mRotSpeed *= kDecay;
    }

    public Matrix getMatrix() {
        return mRotMatrix;
    }

    public void fling(float dx, float dy) {
        Vec3 u = normalVec(mCenterX, mCenterY),
             v = normalVec(mCenterX + dx, mCenterY + dy);
        mRotAxis = u.cross(v).normalize();

        double flingSpeed = Math.sqrt(dx*dx + dy*dy)/mRadius;
        mRotSpeed = (float)(flingSpeed*Math.PI);
    }

    private Vec3 normalVec(float x, float y) {
        x = (x - mCenterX)/mRadius;
        y = (y - mCenterY)/mRadius;
        float len2 = x*x + y*y;

        if (len2 > 1) {
            // normalize
            float len = (float)Math.sqrt(len2);
            x /= len;
            y /= len;
            len2 = 1;
        }

        return new Vec3(x, y, (float)Math.sqrt(1 - len2));
    }
};

class CubeRenderer extends SurfaceRenderer implements GestureDetector.OnGestureListener {
    private VSphereAnimator mVSphere;
    private Matrix          mViewMatrix;
    private float           mCubeSideLength;
    private long            mPrevMS;

    private Face[] faces;

    public CubeRenderer(Resources res) {
        final float rot = (float) Math.PI;
        faces = new Face[] {
            new Face(0, -rot/2, new ImageShaderSample(res, R.raw.brickwork_texture)),
            new Face(0, 0     , new SolidColorSample(1, 0, 0, 1)),
            new Face(0, rot   , new SolidColorSample(0, 1, 0, 1)),
            new Face(rot/2, 0 , new SolidColorSample(0, 0, 1, 1)),
            new Face(-rot/2, 0, new SolidColorSample(1, 1, 0, 1)),
            new Face(0, rot/2 , new SolidColorSample(0, 1, 1, 1)),
        };
    }

    @Override
    protected void onSurfaceInitialized(Surface surface) {
        float cx = surface.getWidth()  / 2,
              cy = surface.getHeight() / 2,
              r  = Math.min(cx, cy);

        mVSphere = new VSphereAnimator(cx, cy, r);

        // square viewport size fitting the given surface
        float vsz = r * 2;

        mCubeSideLength = vsz * 0.5f;

        float viewAngle = (float)Math.PI / 4f,
              viewDistance = (float)(r / tan(viewAngle/2));

        mViewMatrix = new Matrix()
                        // centered viewport
                        .translate(cx, cy)
                        // perspective
                        .scale(vsz/2, vsz/2, 1)
                        .preConcat(Matrix.makePerspective(0.05f, viewDistance, viewAngle))
                        // camera
                        .preConcat(Matrix.makeLookAt(0, 0, -viewDistance, 0, 0, 0, 0, 1, 0));
    }

    @Override
    protected void onRenderFrame(Canvas canvas, long ms) {
        if (mPrevMS == 0) {
            mPrevMS = ms;
        }

        mVSphere.animate((ms - mPrevMS) / 1000.f);
        mPrevMS = ms;


        // clear canvas
        canvas.drawColor(0xffffffff);

        canvas.save();
        canvas.concat(mViewMatrix);
        canvas.concat(mVSphere.getMatrix());

        for (Face f : faces) {
            //TODO: auto restore
            canvas.save();
            canvas.concat(f.asMatrix(mCubeSideLength/2));

            if (front(canvas.getLocalToDevice())) {
                f.sample.render(canvas, ms,
                                -mCubeSideLength/2,
                                -mCubeSideLength/2,
                                 mCubeSideLength/2,
                                 mCubeSideLength/2);
            }
            canvas.restore();
        }
        canvas.restore();
    }

    @Override
    public boolean onFling(MotionEvent e1, MotionEvent e2, float dx, float dy) {
        mVSphere.fling(dx, -dy);
        return true;
    }

    // GestureDetector stubs
    @Override
    public boolean onDown(MotionEvent e) { return true; }

    @Override
    public boolean onScroll(MotionEvent e1, MotionEvent e2, float dx, float dy) { return false; }

    @Override
    public boolean onSingleTapUp(MotionEvent e) { return false; }

    @Override
    public void onLongPress(MotionEvent e) {}

    @Override
    public void onShowPress(MotionEvent e) {}

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

public class CubeActivity extends Activity {
    static {
        System.loadLibrary("androidkit");
    }

    private GestureDetector mDetector;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_cube);

        SurfaceView sv = findViewById(R.id.surfaceView);

        CubeRenderer renderer = new CubeRenderer(getResources());
        sv.getHolder().addCallback(renderer);

        mDetector = new GestureDetector(this, renderer);
    }

    @Override
    public boolean onTouchEvent(MotionEvent e) {
          return mDetector.onTouchEvent(e);
    }
}
