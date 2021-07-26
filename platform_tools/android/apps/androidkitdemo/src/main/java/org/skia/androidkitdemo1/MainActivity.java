/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkitdemo1;

import android.app.Activity;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;

import org.skia.androidkit.*;
import org.skia.androidkit.util.SkottieView;
import org.skia.androidkit.util.SurfaceRenderer;

public class MainActivity extends Activity {
    private ImageView bitmapImage;
    private Surface threadedSurface;

    static {
        System.loadLibrary("androidkit");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Paint p = new Paint();
        p.setColor(new Color(0, 1, 0, 1));

        /*
         * Draw into a Java Bitmap through software using Skia's native API.
         * Load the Bitmap into an ImageView.
         * Applies Matrix transformations to canvas
         */
        Bitmap.Config conf = Bitmap.Config.ARGB_8888;
        Bitmap bmp = Bitmap.createBitmap(400, 400, conf);
        Surface bitmapSurface = new Surface(bmp);
        Canvas canvas = bitmapSurface.getCanvas();

        canvas.drawRect(0, 0, 100, 100, p);

        float[] m = {1, 0, 0, 100,
                0, 1, 0, 100,
                0, 0, 1, 0,
                0, 0, 0, 1};
        p.setColor(new Color(0, 0, 1, 1));
        canvas.save();
        canvas.concat(m);
        canvas.drawRect(0, 0, 100, 100, p);
        canvas.restore();

        Image snapshot = bitmapSurface.makeImageSnapshot();
        canvas.drawImage(snapshot, 0, 200);

        try {
            Image image = Image.fromStream(getResources().openRawResource(R.raw.brickwork_texture));
            // TODO: Canvas.scale
            canvas.concat(new Matrix().scale(10, 10));
            canvas.drawImage(image, 20, 0, SamplingOptions.CATMULLROM());
        } catch (Exception e) {
            Log.e("AndroidKit Demo", "Could not load Image resource: " + R.raw.brickwork_texture);
        }
        bitmapImage = findViewById(R.id.bitmapImage);
        bitmapImage.setImageBitmap(bmp);

        /*
         * Draw into a SurfaceView's surface with GL
         * The ThreadedSurface is handled by AndroidKit through native code
         */
        SurfaceView surfaceView = findViewById(R.id.threadedSurface);
        surfaceView.getHolder().addCallback(new ThreadedSurfaceHandler());

        /*
         * Draw into a SurfaceView's surface with GL
         * The thread is handled using a util RenderThread provided by AndroidKit
         */
        SurfaceView runtimeEffectView = findViewById(R.id.runtimeEffect);
        runtimeEffectView.getHolder().addCallback(new DemoRuntimeShaderRenderer());

        /*
         * SkottieView added programmatically to view hierarchy
         */
        SkottieView skottieView = new SkottieView(this, R.raw.im_thirsty, new Color(1, 1, 1, 1));
        skottieView.setLayoutParams(new ViewGroup.LayoutParams(400, 400));
        skottieView.setOnClickListener((View v) -> {
            SkottieView s = (SkottieView)v;
            s.pause();
        });
        LinearLayout skottieContainer = findViewById(R.id.skottie_container);
        skottieContainer.addView(skottieView);
    }

    private class ThreadedSurfaceHandler implements SurfaceHolder.Callback {
        @Override
        public void surfaceCreated(@NonNull SurfaceHolder holder) {}

        @Override
        public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
            threadedSurface = Surface.createThreadedSurface(holder.getSurface());
            Paint p = new Paint();
            p.setColor(new Color(1, 1, 0, 1));
            p.setStrokeWidth(15);
            p.setStrokeCap(Paint.Cap.ROUND);
            p.setStrokeJoin(Paint.Join.ROUND);
            p.setStrokeMiter(4);
//            ImageFilter filter = ImageFilter.distantLitDiffuse(.5f, .5f, .5f, new Color(1, 0, 0, 1), 1, 1, null);
//            ImageFilter filter = ImageFilter.blur(10, 10, TileMode.DECAL, null);
            ImageFilter filter = ImageFilter.dropShadow(10, 10, 10, 10, new Color(1, 0, 0, 1), null);
//            ImageFilter filter2 = ImageFilter.blend(BlendMode.DIFFERENCE, null, filter);
            p.setImageFilter(filter);
            PathBuilder pathBuilder = new PathBuilder();
            pathBuilder.moveTo(20, 20);
            pathBuilder.quadTo(180, 60, 180, 180);
            pathBuilder.close();
            pathBuilder.moveTo(180, 60);
            pathBuilder.quadTo(180, 180, 60, 180);
            Path path = pathBuilder.makePath();
            threadedSurface.getCanvas().drawPath(path, p);
            threadedSurface.flushAndSubmit();
        }

        @Override
        public void surfaceDestroyed(@NonNull SurfaceHolder holder) {}
    }

    class DemoRuntimeShaderRenderer extends SurfaceRenderer {
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
}
