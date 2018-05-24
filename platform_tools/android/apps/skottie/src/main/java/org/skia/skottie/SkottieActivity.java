/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.skottie;

import android.app.Activity;
import android.os.Bundle;
import android.view.Gravity;
import android.widget.FrameLayout;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

public class SkottieActivity extends Activity {

    private final static long TIME_OUT_MS = 10000;

    private SkottieApplication mApplication;

    private CountDownLatch mEnterAnimationFence = new CountDownLatch(1);

    @Override
    public void onEnterAnimationComplete() {
        super.onEnterAnimationComplete();
        mEnterAnimationFence.countDown();
    }

    public void waitForEnterAnimationComplete() throws TimeoutException, InterruptedException {
        if (!mEnterAnimationFence.await(TIME_OUT_MS, TimeUnit.MILLISECONDS)) {
            throw new TimeoutException();
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        SkottieView skottie1 = new SkottieView(this, getResources().openRawResource(R.raw.star));
        SkottieView skottie2 = new SkottieView(this, getResources().openRawResource(R.raw.movie_loading));
        FrameLayout frameView = new FrameLayout(this);
        frameView.addView(skottie1, new FrameLayout.LayoutParams(1000, 1000,
                Gravity.TOP | Gravity.LEFT));
        frameView.addView(skottie2, new FrameLayout.LayoutParams(1500, 1500,
                Gravity.BOTTOM | Gravity.LEFT));
        setContentView(frameView);

        new Thread() {
            public void run() {
                try {
                    waitForEnterAnimationComplete();
                    skottie1.start();
                    skottie2.start();
                }
                catch (Throwable t) {
                    throw new RuntimeException(t);
                }
            }
        }.start();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }
}
