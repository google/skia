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

import static java.lang.Math.tan;

class Face {
    private float rotX;
    private float rotY;
    public Paint paint;

    Face(float rotX, float rotY, Paint paint) {
        this.rotX = rotX;
        this.rotY = rotY;
        this.paint = paint;
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

    // TODO: make these relative to surface size
    private float mCubeSideLength = 500;
    private int DX = 200;
    private int DY = 300;

    private long mPrevMS;

    private float fAngle = (float)Math.PI / 12;
    private float eyeZ = (float)(1.0f/tan(fAngle/2) - 1);

    private Matrix cam = Matrix.makeLookAt(0, 0, eyeZ, 0, 0, 0, 0, 1, 0);
    private Matrix perspective = Matrix.makePerspective(0.05f, 4, fAngle);
    private Matrix viewport;

    private final float rot = (float) Math.PI;
    private Face[] faces;

    public CubeRenderer(Resources res) {
        Paint brickPaint = new Paint();

        try {
            Image image = Image.fromStream(res.openRawResource(R.raw.brickwork_texture));
            Shader shader =
                image.makeShader(TileMode.REPEAT, TileMode.REPEAT,
                                 new SamplingOptions(SamplingOptions.FilterMode.LINEAR));
            brickPaint.setShader(shader);
        } catch (Exception e) {}

        faces = new Face[] {
            new Face(0, -rot/2, brickPaint),
            new Face(0, 0     , new Paint().setColor(new Color(1, 0, 0, 1))),
            new Face(0, rot   , new Paint().setColor(new Color(0, 1, 0, 1))),
            new Face(rot/2, 0 , new Paint().setColor(new Color(0, 0, 1, 1))),
            new Face(-rot/2, 0, new Paint().setColor(new Color(1, 1, 0, 1))),
            new Face(0, rot/2 , new Paint().setColor(new Color(0, 1, 1, 1))),
        };
    }

    @Override
    protected void onSurfaceInitialized(Surface surface) {
        float hw = surface.getWidth() / 2,
              hh = surface.getHeight() / 2;
        mVSphere = new VSphereAnimator(hw, hh, Math.min(hw, hh));

        viewport = new Matrix().translate(mCubeSideLength/2, mCubeSideLength/2, 0)
                               .scale(mCubeSideLength/2, mCubeSideLength/2, surface.getWidth());
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
        canvas.concat(new Matrix().translate(DX, DY, 0));
        canvas.concat(viewport.preConcat(perspective)
                              .preConcat(cam)
                              .preConcat(Matrix.makeInverse(viewport)));

        for (Face f : faces) {
            //TODO: auto restore
            canvas.save();
            Matrix trans = new Matrix().translate(mCubeSideLength/2, mCubeSideLength/2, 0);
            Matrix m = new Matrix().preConcat(mVSphere.getMatrix())
                                   .preConcat(f.asMatrix(mCubeSideLength/2));

            canvas.concat(trans);
            Matrix localToWorld = m.preConcat(Matrix.makeInverse(trans));
            canvas.concat(localToWorld);

            if (front(canvas.getLocalToDevice())) {
                canvas.drawRect(0, 0, mCubeSideLength, mCubeSideLength, f.paint);
            }
            canvas.restore();
        }
        canvas.restore();
    }

    @Override
    public boolean onFling(MotionEvent e1, MotionEvent e2, float dx, float dy) {
        mVSphere.fling(dx, dy);
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
