/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.skottie;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Color;
import android.graphics.Point;
import android.net.Uri;
import android.os.Bundle;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.GridLayout;

import java.io.FileNotFoundException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import static java.lang.Math.ceil;
import static java.lang.Math.sqrt;

public class SkottieActivity extends Activity implements View.OnClickListener {

    private final static long TIME_OUT_MS = 10000;

    private SkottieApplication mApplication;

    private CountDownLatch mEnterAnimationFence = new CountDownLatch(1);

    private GridLayout mGrid;
    private int mRowCount = 0;
    private int mColumnCount = 0;
    private int mCellWidth = 0;
    private int mCellHeight = 0;

    private List<SkottieView>  mAnimations;
    static private List<Uri> mAnimationFiles = new ArrayList<Uri>();

    private void populateGrid() {
        mRowCount = 0;
        mColumnCount = 0;
        mAnimations = new ArrayList<SkottieView>();
        mCellWidth = 0;
        mCellHeight = 0;

        int rawAssets[] = {
                R.raw.star, R.raw.movie_loading, R.raw.uk,  R.raw.white_material_wave_loading
        };

        for (int resId : rawAssets) {
            SkottieView view = new SkottieView(this);
            view.setSource(getResources().openRawResource(resId));
            mAnimations.add(view);
        }

        for (Uri uri : mAnimationFiles) {
            try {
                InputStream inputStream = getContentResolver().openInputStream(uri);
                SkottieView view = new SkottieView(this);
                view.setSource(inputStream);
                mAnimations.add(view);
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            }
        }

        Point size = new Point();
        getWindowManager().getDefaultDisplay().getSize(size);
        int screenWidth = size.x;
        int screenHeight = (int)(size.y / 1.3f);

        double unit = sqrt(mAnimations.size() / 6.0f);
        mRowCount = (int)ceil(3 * unit);
        mColumnCount = (int)ceil(2 * unit);
        mGrid.setColumnCount(mColumnCount);
        mGrid.setRowCount(mRowCount);
        mCellWidth = screenWidth / mColumnCount;
        mCellHeight = screenHeight / mRowCount;

        refreshGrid();

        startAnimation();

        for (SkottieView view : mAnimations) {
            view.setOnClickListener(new View.OnClickListener(){
                public void onClick(View view){
                    inflateView((SkottieView)view);
                }
            });
        }

        if (mInflatedIndex >= 0) {
            SkottieView view = mAnimations.get(mInflatedIndex);
            mInflatedIndex = -1;
            inflateView(view);
        }
    }

    static int mInflatedIndex = -1;

    private void inflateView(SkottieView view) {
        if (mInflatedIndex >= 0) {
            //deflate active view
            SkottieView oldView = mAnimations.get(mInflatedIndex);
            if (oldView != null) {
                int row = mInflatedIndex / mColumnCount, column = mInflatedIndex % mColumnCount;
                addView(oldView, row, column, false);
            }
            mInflatedIndex = -1;
            //start and show animations that were in the background
            for (SkottieView anyView : mAnimations) {
                if (anyView != oldView) {
                    anyView.getSkottieAnimation().start();
                    anyView.setVisibility(View.VISIBLE);
                }
            }
            return;
        }

        //stop and hide animations in the background
        for (SkottieView anyView : mAnimations) {
            if (anyView != view) {
                anyView.getSkottieAnimation().stop();
                anyView.setVisibility(View.INVISIBLE);
            }
        }

        mInflatedIndex = mAnimations.indexOf(view);

        GridLayout.Spec rowSpec = GridLayout.spec(0, mRowCount, GridLayout.CENTER);
        GridLayout.Spec colSpec = GridLayout.spec(0, mColumnCount, GridLayout.CENTER);
        GridLayout.LayoutParams params = new GridLayout.LayoutParams(rowSpec, colSpec);
        params.width = ViewGroup.LayoutParams.MATCH_PARENT;
        params.height =  ViewGroup.LayoutParams.MATCH_PARENT;

        mGrid.updateViewLayout(view, params);
    }

    private void refreshGrid() {
        mGrid.removeAllViews();
        int currentRaw = 0;
        int row = 0, column = 0;
        for (SkottieView view : mAnimations) {
            addView(view, row, column, true);
            column++;
            if (column >= mColumnCount) {
                column = 0;
                row++;
            }
        }
    }

    private void addView(SkottieView view,  int row , int column, boolean addView) {
        GridLayout.Spec rowSpec = GridLayout.spec(row, 1, GridLayout.CENTER);
        GridLayout.Spec colSpec = GridLayout.spec(column, 1, GridLayout.CENTER);
        GridLayout.LayoutParams params = new GridLayout.LayoutParams(rowSpec, colSpec);
        params.width = mCellWidth;
        params.height = mCellHeight;
        if (addView) {
            mGrid.addView(view, params);
        } else {
            mGrid.updateViewLayout(view, params);
        }
    }

    private void startAnimation() {
        for (SkottieView view : mAnimations) {
            view.getSkottieAnimation().start();
        }
    }

    private void stopAnimation() {
        for (SkottieView view : mAnimations) {
            view.getSkottieAnimation().stop();
        }
    }

    private void addLottie(Uri uri) throws FileNotFoundException {
        InputStream inputStream = getContentResolver().openInputStream(uri);
        int animations = mAnimations.size();
        if (animations < mRowCount * mColumnCount) {
            SkottieView view = new SkottieView(this);
            view.setSource(inputStream);
            int row = animations / mColumnCount, column = animations % mColumnCount;
            mAnimations.add(view);
            mAnimationFiles.add(uri);
            view.setOnClickListener(new View.OnClickListener(){
                public void onClick(View view){
                    inflateView((SkottieView)view);
                }
            });
            addView(view, row, column, true);
            view.getSkottieAnimation().start();
        } else {
            stopAnimation();
            mAnimationFiles.add(uri);
            populateGrid();
            startAnimation();
        }
    }


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

    private void createLayout() {
        setContentView(R.layout.main_layout);
        Button button1 = (Button)findViewById(R.id.open_lottie);
        button1.setOnClickListener(this);
        mGrid = (GridLayout)findViewById(R.id.grid_lotties);
        mGrid.setBackgroundColor(Color.LTGRAY);

        populateGrid();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        createLayout();
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
                    addLottie(uri);
                } catch (FileNotFoundException e) {
                    e.printStackTrace();
                }
            }
        }
    }
}
