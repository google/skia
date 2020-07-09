/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.skottie;

import android.content.Context;
import android.content.res.TypedArray;
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
    private View backingView;

    // SkottieView constructor when initialized in XML layout
    public SkottieView(Context context, AttributeSet attrs) {
        super(context, attrs);
        TypedArray a = context.getTheme()
            .obtainStyledAttributes(attrs, R.styleable.SkottieView, 0, 0);
        try {
            switch (a.getInteger(R.styleable.SkottieView_backing_view, -1)) {
                case 0:
                    backingView = new TextureView(context);
                    ((TextureView)backingView).setOpaque(false);
                    break;
                case 1:
                    backingView = new SurfaceView(context);
                    int bg = a.getColor(R.styleable.SkottieView_background_color, -1);
                    if (bg == -1) {
                        throw new RuntimeException("background_color attribute needed for SurfaceView");
                    }
                    break;
                default:
                    throw new RuntimeException("backing_view attribute needed to specify between texture_view or surface_view");
            }
        } catch(RuntimeException err) {
            Log.e("Missing Attribute", err.getMessage());
        } finally {
            a.recycle();
        }
        initBackingArray();
    }
    
    // SkottieView builder constructor
    private SkottieView(Context context, SkottieViewBuilder builder) {
        super(context, builder.attrs, builder.defStyleAttr);
        // create the backing view
        if (builder.advancedFeatures) {
            // backing view must be SurfaceView
            backingView = new SurfaceView(context);
        } else {
            backingView = new TextureView(context);
            ((TextureView)backingView).setOpaque(false);
        }
        initBackingArray();
    }

    private void initBackingArray() {
        backingView.setLayoutParams(new ViewGroup.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT,
            ViewGroup.LayoutParams.MATCH_PARENT));
        addView(backingView);
    }

    //TODO handle SurfaceView
    public void setSource(InputStream inputStream) {
        mAnimation = SkottieRunner.getInstance().createAnimation(((TextureView)backingView), inputStream);
    }

    public void setSource(int resId) {
        InputStream inputStream = backingView.getResources().openRawResource(resId);
        mAnimation = SkottieRunner.getInstance().createAnimation(((TextureView)backingView), inputStream);
    }

    public void setSource(Context context, Uri uri) throws FileNotFoundException {
        InputStream inputStream = context.getContentResolver().openInputStream(uri);
        mAnimation = SkottieRunner.getInstance().createAnimation(((TextureView)backingView), inputStream);
    }

    public SkottieAnimation getSkottieAnimation() {
        return mAnimation;
    }

    // Builder accessed by user to generate SkottieViews
    public static class SkottieViewBuilder {
        protected AttributeSet attrs;
        protected int defStyleAttr;

        // if true, backing view will be surface view
        protected boolean advancedFeatures;
        // TODO private variable backgroundColor

        public void setAttrs(AttributeSet attrs) {
          this.attrs = attrs;
        }

        public void setDefStyleAttr(int defStyleAttr) {
          this.defStyleAttr = defStyleAttr;
        }

        public void setAdvancedFeatures(boolean advancedFeatures) {
          this.advancedFeatures = advancedFeatures;
        }

        public SkottieView build(Context context) {
          return new SkottieView(context, this);
        }
    }
}
