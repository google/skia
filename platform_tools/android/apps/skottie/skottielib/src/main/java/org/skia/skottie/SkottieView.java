/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.skottie;

import android.content.Context;
import android.net.Uri;
import android.view.SurfaceView;
import android.view.TextureView;

import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.widget.FrameLayout;
import java.io.FileNotFoundException;
import java.io.InputStream;

public class SkottieView extends FrameLayout {

    private SkottieAnimation mAnimation;
    private View backingView;

    protected SkottieView(Context context, SkottieViewBuilder builder) {
      super(context);
      // create the backing view
      if (builder.advancedFeatures) {
            // backing view must be SurfaceView
          backingView = new SurfaceView(context, builder.attrs, builder.defStyleAttr);
        } else {
          backingView = new TextureView(context, builder.attrs, builder.defStyleAttr);
          ((TextureView)backingView).setOpaque(false);
        }
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
}
