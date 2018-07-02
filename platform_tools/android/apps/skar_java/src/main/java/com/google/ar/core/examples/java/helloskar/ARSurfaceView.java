package com.google.ar.core.examples.java.helloskar;

import android.content.Context;
import android.graphics.Color;
import android.graphics.PixelFormat;
import android.util.AttributeSet;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

/**
 * SurfaceView that is overlayed on top of a GLSurfaceView. All 2D drawings can be done on this
 * surface.
 */
public class ARSurfaceView extends SurfaceView implements SurfaceHolder.Callback {

    boolean running;

    public ARSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);

        SurfaceHolder holder = getHolder();
        this.setBackgroundColor(Color.TRANSPARENT);
        this.setZOrderOnTop(true);
        holder.setFormat(PixelFormat.TRANSPARENT);
        holder.addCallback(this);
    }

    public boolean isRunning() {
        return running;
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        running = true;
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        running = false;
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
    }
}
