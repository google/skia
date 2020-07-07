/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.skottie;

import android.content.Context;
import android.net.Uri;
import android.util.AttributeSet;
import android.view.TextureView;

import java.io.FileNotFoundException;
import java.io.InputStream;

public class SkottieView extends TextureView {

    private SkottieAnimation mAnimation;

    public SkottieView(Context context) {
        super(context);
        init();
    }

    public SkottieView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public SkottieView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }

    public SkottieView(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
        init();
    }

    private void init() {
        setOpaque(false);
    }

    public void setSource(InputStream inputStream) {
        mAnimation = SkottieRunner.getInstance().createAnimation(this, inputStream);
    }

    public void setSkottieResource(int resId) {
        InputStream inputStream = getResources().openRawResource(resId);
        mAnimation = SkottieRunner.getInstance().createAnimation(this, inputStream);
    }

    public void setSkottieURI(Context context, Uri uri) throws FileNotFoundException {
        InputStream inputStream = context.getContentResolver().openInputStream(uri);
        mAnimation = SkottieRunner.getInstance().createAnimation(this, inputStream);
    }

    public SkottieAnimation getSkottieAnimation() {
        return mAnimation;
    }
}
