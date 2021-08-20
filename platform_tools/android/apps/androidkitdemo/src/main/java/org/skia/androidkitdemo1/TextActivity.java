package org.skia.androidkitdemo1;

import android.app.Activity;
import android.content.res.Resources;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import org.skia.androidkit.Canvas;
import org.skia.androidkit.Surface;
import org.skia.androidkit.Text;
import org.skia.androidkit.util.SurfaceRenderer;
import org.skia.androidkitdemo1.samples.RuntimeSample;

public class TextActivity extends Activity implements SurfaceHolder.Callback {
    static {
        System.loadLibrary("androidkit");
    }

    Surface mSurface;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_animation);

        SurfaceView sv = findViewById(R.id.surfaceView);
        sv.getHolder().addCallback(this);
    }

    @Override
    public void surfaceCreated(@NonNull SurfaceHolder holder) {}

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
        mSurface = Surface.CreateGL(holder.getSurface());
        Text text = new Text("Hello World!");
        text.renderText(mSurface.getCanvas(), 0, 200);
        mSurface.flushAndSubmit();
    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {

    }
}