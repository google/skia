/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.skottie;

import android.animation.Animator;
import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Color;
import android.net.Uri;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceView;
import android.view.TextureView;

import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import java.io.FileNotFoundException;
import java.io.InputStream;

import org.skia.skottielib.R;

public class SkottieView extends FrameLayout {

    private SkottieAnimation mAnimation;
    private View mBackingView;
    private int mBackgroundColor;
    // Repeat follows Animator API, infinite is represented by -1 (see Animator.DURATION_INFINITE)
    private int mRepeatCount;

    private static final int BACKING_VIEW_TEXTURE = 0;
    private static final int BACKING_VIEW_SURFACE = 1;
    private final String LOG_TAG = "SkottieView";

    /*
     * Build function for SkottieViews backed with a texture view
     * Is the same as calling for a default SkottieView
     */
    public SkottieView buildAsTexture(Context context) {
        return new SkottieView(context);
    }

    /*
     * Build function for SkottieViews backed with a surface view
     * Backs the animation surface with a SurfaceView instead, requires background color
     */
    public SkottieView buildAsSurface(Context context, int backgroundColor) {
        SkottieView s = new SkottieView(context);
        s.mBackingView = new SurfaceView(context);
        s.mBackgroundColor = backgroundColor;
        return s;
    }

    // Basic SkottieView, backing view defaults to TextureView
    public SkottieView(Context context) {
        super(context);
        mBackingView = new TextureView(context);
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
            // set mRepeatCount
            mRepeatCount = a.getInteger(R.styleable.SkottieView_android_repeatCount, 0);
            // set backing view and background color
            switch (a.getInteger(R.styleable.SkottieView_backing_view, -1)) {
                case BACKING_VIEW_TEXTURE:
                    mBackingView = new TextureView(context);
                    ((TextureView) mBackingView).setOpaque(false);
                    mBackgroundColor = a.getColor(R.styleable.SkottieView_background_color, 0);
                    break;
                case BACKING_VIEW_SURFACE:
                    mBackingView = new SurfaceView(context);
                    mBackgroundColor = a.getColor(R.styleable.SkottieView_background_color, -1);
                    if (mBackgroundColor == -1) {
                        throw new RuntimeException("background_color attribute "
                            + "needed for SurfaceView");
                    }
                    if (Color.alpha(mBackgroundColor) != 255) {
                        throw new RuntimeException("background_color cannot be transparent");
                    }
                    break;
                default:
                    throw new RuntimeException("backing_view attribute needed to "
                        + "specify between texture_view or surface_view");
            }
            // set source
            int src = a.getResourceId(R.styleable.SkottieView_src, -1);
            if (src != -1) {
                setSource(src);
            }
        } finally {
            a.recycle();
        }
        initBackingView();
    }

    private void initBackingView() {
        mBackingView.setLayoutParams(new ViewGroup.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT,
            ViewGroup.LayoutParams.MATCH_PARENT));
        addView(mBackingView);
    }

    public void setSource(InputStream inputStream) {
        mAnimation = setSourceHelper(inputStream);
    }

    public void setSource(int resId) {
        InputStream inputStream = mBackingView.getResources().openRawResource(resId);
        mAnimation = setSourceHelper(inputStream);
    }

    public void setSource(Context context, Uri uri) throws FileNotFoundException {
        InputStream inputStream = context.getContentResolver().openInputStream(uri);
        mAnimation = setSourceHelper(inputStream);
    }

    private SkottieAnimation setSourceHelper(InputStream inputStream) {
        SkottieAnimation.Config config = null;
        SkottieAnimation animation;
        // if there is already an animation, save config and finalize it so as to not confuse GL
        if (mAnimation != null) {
            config = mAnimation.getBackingViewConfig();
            try {
                mAnimation.finalize();
            } catch (Throwable t) {
                Log.e(LOG_TAG, "existing animation couldn't finalize before setting new src");
            }
        }
        if (mBackingView instanceof TextureView) {
            animation = SkottieRunner.getInstance()
                .createAnimation(((TextureView) mBackingView), inputStream, mBackgroundColor, mRepeatCount);
        } else {
            animation = SkottieRunner.getInstance()
                .createAnimation(((SurfaceView) mBackingView), inputStream, mBackgroundColor, mRepeatCount);
        }
        // restore config settings from previous animation if needed
        if (config != null) {
            animation.setBackingViewConfig(config);
            animation.setProgress(0f);
        }
        return animation;
    }

    protected SkottieAnimation getSkottieAnimation() {
        return mAnimation;
    }

    public void removeListener(Animator.AnimatorListener listener) {
        mAnimation.removeListener(listener);
    }

    public void addListener(Animator.AnimatorListener listener) {
        mAnimation.addListener(listener);
    }

    // progress: a float from 0 to 1 representing the percent into the animation
    public void seek(float progress) {
        if(mAnimation != null) {
            mAnimation.setProgress(progress);
        }
    }

    public void play() {
        if(mAnimation != null) {
            mAnimation.resume();
        }
    }

    public void pause() {
        if(mAnimation != null) {
            mAnimation.pause();
        }
    }

    public void start() {
        if(mAnimation != null) {
            mAnimation.start();
        }
    }

    public void stop() {
        if(mAnimation != null) {
            mAnimation.end();
        }
    }

    public float getProgress() {
        if(mAnimation != null) {
            return mAnimation.getProgress();
        }
        return -1;
    }

    public void setRepeatCount(int repeatCount) {
        mRepeatCount = repeatCount;
    }

    public void setBackgroundColor(int colorRGB) {
        mBackgroundColor = colorRGB;
    }
}
