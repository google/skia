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

class RuntimeShaderRenderer extends SurfaceRenderer {
    private RuntimeShaderBuilder mBuilder = new RuntimeShaderBuilder(SkSLShader);

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

    @Override
    protected void onSurfaceInitialized(Surface surface) {}

    @Override
    protected void onRenderFrame(Canvas canvas, long ms) {
        final int w = canvas.getWidth();
        final int h = canvas.getHeight();

        Paint p = new Paint();
        p.setShader(mBuilder.setUniform("u_time", ms/1000.0f)
                            .setUniform("u_w", w)
                            .setUniform("u_h", h)
                            .makeShader());

        canvas.drawRect(0, 0, w, h, p);
    }
}

public class RuntimeShaderActivity extends Activity {
    static {
        System.loadLibrary("androidkit");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_animation);

        SurfaceView sv = findViewById(R.id.surfaceView);
        sv.getHolder().addCallback(new RuntimeShaderRenderer());
    }
}
