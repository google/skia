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
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import org.skia.androidkit.Canvas;
import org.skia.androidkit.Color;
import org.skia.androidkit.Image;
import org.skia.androidkit.Matrix;
import org.skia.androidkit.Paint;
import org.skia.androidkit.RuntimeShaderBuilder;
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

class CubeRenderer extends SurfaceRenderer {
    private static final String lightingShader =
    "uniform shader color_map;                                              " +
    "uniform shader normal_map;                                             " +

    "uniform float4x4 localToWorld;                                         " +
    "uniform float4x4 localToWorldAdjInv;                                   " +
    "uniform float3   lightPos;                                             " +

    "float3 convert_normal_sample(half4 c) {                                " +
    "    float3 n = 2 * c.rgb - 1;                                          " +
    "    n.y = -n.y;                                                        " +
    "    return n;                                                          " +
    "}                                                                      " +

    "half4 main(float2 p) {                                                 " +
    "    float3 norm = convert_normal_sample(sample(normal_map, p));        " +
    "    float3 plane_norm = normalize(localToWorldAdjInv * norm.xyz0).xyz; " +

    "    float3 plane_pos = (localToWorld * p.xy01).xyz;                    " +
    "    float3 light_dir = normalize(lightPos - plane_pos);                " +

    "    float ambient = 0.2;                                               " +
    "    float dp = dot(plane_norm, light_dir);                             " +
    "    float scale = min(ambient + max(dp, 0), 1);                        " +

    "    return sample(color_map, p) * scale.xxx1;                          " +
    "}";
    // TODO: make these relative to surface size
    private float mCubeSideLength = 500;
    private int DX = 200;
    private int DY = 300;

    private float fAngle = (float)Math.PI / 12;
    private float eyeZ = (float)(1.0f/tan(fAngle/2) - 1);

    private Matrix cam = Matrix.makeLookAt(0, 0, eyeZ, 0, 0, 0, 0, 1, 0);
    private Matrix perspective = Matrix.makePerspective(0.05f, 4, fAngle);
    private Matrix viewport;

    private final float rot = (float) Math.PI;
    private Face[] faces;

    public CubeRenderer(Resources res) {
//        Paint brickPaint = new Paint();
//
//        try {
//            Image image = Image.fromStream(res.openRawResource(R.raw.brickwork_texture));
//            Shader shader =
//                image.makeShader(TileMode.REPEAT, TileMode.REPEAT,
//                                 new SamplingOptions(SamplingOptions.FilterMode.LINEAR));
//            brickPaint.setShader(shader);
//        } catch (Exception e) {}

        faces = new Face[] {
            new Face(0, -rot/2, new Paint().setColor(new Color(1, 0, 1, 1))),
            new Face(0, 0     , new Paint().setColor(new Color(1, 0, 0, 1))),
            new Face(0, rot   , new Paint().setColor(new Color(0, 1, 0, 1))),
            new Face(rot/2, 0 , new Paint().setColor(new Color(0, 0, 1, 1))),
            new Face(-rot/2, 0, new Paint().setColor(new Color(1, 1, 0, 1))),
            new Face(0, rot/2 , new Paint().setColor(new Color(0, 1, 1, 1))),
        };
    }

    @Override
    protected void onSurfaceInitialized(Surface surface) {
        viewport = new Matrix().translate(mCubeSideLength/2, mCubeSideLength/2, 0)
                               .scale(mCubeSideLength/2, mCubeSideLength/2, surface.getWidth());
    }

    @Override
    protected void onRenderFrame(Canvas canvas, long ms) {
        float speed = 0.5f;
        float rads = ms / 1000.f * speed % (float)(2 * Math.PI);

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
            Matrix m = new Matrix().rotateY(rads)
                                   .rotateZ(rads)
                                   .preConcat(f.asMatrix(mCubeSideLength/2));

            canvas.concat(trans);
            Matrix localToWorld = m.preConcat(Matrix.makeInverse(trans));
            canvas.concat(localToWorld);

            if (front(canvas.getLocalToDevice())) {
                RuntimeShaderBuilder builder = new RuntimeShaderBuilder(lightingShader);
                builder.setUniform("lightPos", 0, 0, 0);
                builder.setUniform("localToWorld", localToWorld);
                float[] a = localToWorld.getRowMajor();
                Matrix inverse = Matrix.makeInverse(new Matrix(a[0],  a[1],  a[2],  0,
                                                               a[4],  a[5],  a[6],  0,
                                                               a[8],  a[9],  a[10], 0,
                                                               0,     0,     0,     1));
                Matrix normals = Matrix.makeTranspose(inverse);
                builder.setUniform("localToWorldAdjInv", normals);
                Paint p = new Paint();
                p.setShader(builder.makeShader());
                p.setColor(new Color(1, 0, 0, 1));
                canvas.drawRect(0, 0, mCubeSideLength, mCubeSideLength, p);
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

public class CubeActivity extends Activity {
    static {
        System.loadLibrary("androidkit");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_cube);

        SurfaceView sv = findViewById(R.id.surfaceView);
        sv.getHolder().addCallback(new CubeRenderer(getResources()));
    }
}
