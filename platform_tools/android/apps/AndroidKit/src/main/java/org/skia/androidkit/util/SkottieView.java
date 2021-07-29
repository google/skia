/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit.util;

import android.content.Context;

import android.content.res.TypedArray;
import android.util.AttributeSet;
import android.view.SurfaceView;

import android.view.ViewGroup;
import android.widget.FrameLayout;
import java.io.InputStream;

import org.skia.androidkit.Canvas;
import org.skia.androidkit.Color;
import org.skia.androidkit.Matrix;
import org.skia.androidkit.SkottieAnimation;
import org.skia.androidkit.Surface;

import org.skia.androidkit.R;

class SkottieRenderer extends SurfaceRenderer {
    private float mSurfaceWidth,
                  mSurfaceHeight;
    private Color mBackground;
    private SkottieAnimation mAnimation;
    private boolean mPlaying;

    SkottieRenderer(SkottieAnimation mAnimation, Color mBackground) {
        this.mAnimation = mAnimation;
        this.mBackground = mBackground;
    }
    @Override
    protected void onSurfaceInitialized(Surface surface) {
        mSurfaceWidth  = surface.getWidth();
        mSurfaceHeight = surface.getHeight();
        mPlaying = true;
    }

    @Override
    protected void onRenderFrame(Canvas canvas, long ms) {
        if(mPlaying) {
            canvas.drawColor(mBackground);
            double t = (double)ms / 1000 % mAnimation.getDuration();
            mAnimation.seekTime(t);

            float s = Math.min(mSurfaceWidth / mAnimation.getWidth(),
                    mSurfaceHeight / mAnimation.getHeight());
            canvas.save();
            canvas.concat(new Matrix().translate((mSurfaceWidth  - s*mAnimation.getWidth())/2,
                    (mSurfaceHeight - s*mAnimation.getHeight())/2)
                    .scale(s, s));

            mAnimation.render(canvas);
            canvas.restore();
        }
    }

    void play() {
        if (!mPlaying) {
            mPlaying = true;
        }
    }

    void pause() {
        if (mPlaying) {
            mPlaying = false;
        }
    }

    SkottieAnimation getAnimation() {
        return mAnimation;
    }

    @Override
    public void release() {
        mAnimation.release();
        super.release();
    }

    @Override
    protected void finalize() throws Throwable {
        this.release();
    }
}

public class SkottieView extends FrameLayout {
    private SurfaceView mBackingView;
    private SkottieRenderer mRenderer;

    private final String LOG_TAG = "SkottieView";

    public SkottieView(Context context, int resID, Color background) {
        super(context);
        mBackingView = new SurfaceView(context);
        initBackingView();
        InputStream inputStream = context.getResources().openRawResource(resID);
        mRenderer = new SkottieRenderer(makeAnimation(inputStream), background);
        mBackingView.getHolder().addCallback(mRenderer);
    }

    public SkottieView(Context context) {
        super(context);
        mBackingView = new SurfaceView(context);
        initBackingView();
    }

    public SkottieView(Context context, AttributeSet attrs) {
        this(context, attrs, 0, 0);
    }

    public SkottieView(Context context, AttributeSet attrs, int defStyleAttr) {
        this(context, attrs, defStyleAttr, 0);
    }

    // SkottieView constructor when initialized in XML layout
    public SkottieView(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs);
        TypedArray a = context.getTheme()
                .obtainStyledAttributes(attrs, R.styleable.SkottieView, defStyleAttr, defStyleRes);
        try {
            // set backing view and background color
            mBackingView = new SurfaceView(context);
            initBackingView();
            int backgroundColor = a.getColor(R.styleable.SkottieView_background_color, -1);
            if (backgroundColor == -1) {
                throw new RuntimeException("background_color attribute "
                        + "needed for SurfaceView");
            }
            if (android.graphics.Color.alpha(backgroundColor) != 255) {
                throw new RuntimeException("background_color cannot be transparent");
            }
            // set source
            int src = a.getResourceId(R.styleable.SkottieView_src, -1);
            Color c = new Color(backgroundColor);
            setSource(src, context, new Color(backgroundColor));
        } finally {
            a.recycle();
        }
    }

    private void initBackingView() {
        mBackingView.setLayoutParams(new ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT));
        addView(mBackingView);
    }

    static private SkottieAnimation makeAnimation(InputStream is) {
        String json = "";
        try {
            byte[] data = new byte[is.available()];
            is.read(data);
            json = new String(data);
        } catch (Exception e) {}
        return new SkottieAnimation(json);
    }

    public void setSource(int resID, Context context, Color background) {
        InputStream inputStream = context.getResources().openRawResource(resID);
        mRenderer = new SkottieRenderer(makeAnimation(inputStream), background);
        mBackingView.getHolder().addCallback(mRenderer);
    }

    public void play() {
        mRenderer.play();
    }

    public void pause() {
        mRenderer.pause();
    }

    public void seekTime(double t) {
        mRenderer.setBaseTime(java.lang.System.currentTimeMillis() - ((long)t * 1000));
    }

    public void seekFrame(double f) {
        double totalFrames = mRenderer.getAnimation().getFrameCount();
        double totalTime = mRenderer.getAnimation().getDuration();
        double targetTime = (f % totalFrames) / totalFrames * totalTime;
        seekTime(targetTime);
    }
}
