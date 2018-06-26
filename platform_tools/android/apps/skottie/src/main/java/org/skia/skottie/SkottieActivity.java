/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.skottie;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.view.Gravity;
import android.view.View;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.GridLayout;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

public class SkottieActivity extends Activity implements View.OnClickListener {

    private final static long TIME_OUT_MS = 10000;

    private SkottieApplication mApplication;

    private CountDownLatch mEnterAnimationFence = new CountDownLatch(1);

    private GridLayout mGrid;

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

    private void addLottie(InputStream inputStream) {
        SkottieView view = new SkottieView(this, inputStream);
        mGrid.addView(view, 1, new FrameLayout.LayoutParams(FrameLayout.LayoutParams.MATCH_PARENT, 2000));
        view.start();
    }

    private void createLayout1() {
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

    private void createLayout2() {
        setContentView(R.layout.main_layout);
        Button button1 = (Button)findViewById(R.id.open_lottie);
        button1.setOnClickListener(this);
        mGrid = (GridLayout)findViewById(R.id.grid_lotties);

        addLottie(getResources().openRawResource(R.raw.star));
        addLottie(getResources().openRawResource(R.raw.movie_loading));
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        createLayout2();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    static final int PICK_FILE_REQUEST = 2;

    @Override
    public void onClick(View view) {
        Intent intent = new Intent();
        intent.setType("application/json");
        Intent i = Intent.createChooser(intent, "View Default File Manager");
        startActivityForResult(i, PICK_FILE_REQUEST);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (resultCode == Activity.RESULT_OK) {
            if (requestCode == PICK_FILE_REQUEST) if (data != null) {
                //no data present
                Uri uri = data.getData();

                try {
                    InputStream inputStream = getContentResolver().openInputStream(uri);
                    addLottie(inputStream);
                } catch (FileNotFoundException e) {
                    e.printStackTrace();
                }
            }
        }
    }
}
