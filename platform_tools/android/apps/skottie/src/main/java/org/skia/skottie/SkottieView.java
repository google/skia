/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.skottie;

import android.content.Context;
import android.graphics.drawable.Animatable;
import android.view.TextureView;
import android.view.ViewGroup;

import java.io.InputStream;

public class SkottieView extends ViewGroup implements Animatable {

    private TextureView mTextureView;
    private Animatable mAnimation;

    public SkottieView(Context context, InputStream is) {
        super(context);

        mTextureView = new TextureView(context);
        mTextureView.setOpaque(false);
        mAnimation = SkottieRunner.getInstance().createAnimation(mTextureView, is);
        addView(mTextureView, LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);
    }

    /**
     * Starts the animation.
     */
    @Override
    public void start()  {
        mAnimation.start();
    }

    /**
     * Stops the animation.
     */
    @Override
    public void stop() {
        mAnimation.stop();
    }

    @Override
    public boolean isRunning() {
        return mAnimation.isRunning();
    }

    /**
     * Ask all children to measure themselves and compute the measurement of this
     * layout based on the children.
     */
    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        mTextureView.measure(widthMeasureSpec, heightMeasureSpec);
        int width = mTextureView.getMeasuredWidth();
        int height = mTextureView.getMeasuredHeight();
        setMeasuredDimension(width, height);
    }

    @Override
    protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
        if (changed) { // This is a new size or position for this view
            mTextureView.layout(0, 0, right - left, bottom - top);
        }
    }
}
