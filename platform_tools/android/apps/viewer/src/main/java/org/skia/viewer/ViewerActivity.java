/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.viewer;

import android.app.Activity;
import android.content.res.Configuration;
import android.os.Bundle;
import android.support.v7.app.ActionBarDrawerToggle;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.ListView;
import androidx.core.widget.DrawerLayout;

public class ViewerActivity
        extends Activity implements SurfaceHolder.Callback, View.OnTouchListener {
    private static final float FLING_VELOCITY_THRESHOLD = 1000;

    private DrawerLayout mDrawerLayout;
    private ActionBarDrawerToggle mDrawerToggle;
    private ListView mDrawerList;
    private StateAdapter mStateAdapter;

    private ViewerApplication mApplication;

    private native void onSurfaceCreated(long handle, Surface surface);
    private native void onSurfaceChanged(long handle, Surface surface);
    private native void onSurfaceDestroyed(long handle);
    private native void onKeyPressed(long handle, int keycode);
    private native void onTouched(long handle, int owner, int state, float x, float y);
    private native void onUIStateChanged(long handle, String stateName, String stateValue);

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.title, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Pass the event to ActionBarDrawerToggle, if it returns
        // true, then it has handled the app icon touch event
        if (mDrawerToggle != null && mDrawerToggle.onOptionsItemSelected(item)) {
            return true;
        }

        switch (item.getItemId()) {
            case R.id.action_left:
                onKeyPressed(mApplication.getNativeHandle(), KeyEvent.KEYCODE_SOFT_LEFT);
                return true;
            case R.id.action_right:
                onKeyPressed(mApplication.getNativeHandle(), KeyEvent.KEYCODE_SOFT_RIGHT);
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        SurfaceView surfaceView = (SurfaceView) findViewById(R.id.surfaceView);
        surfaceView.getHolder().addCallback(this);
        surfaceView.setOnTouchListener(this);

        mDrawerLayout = (DrawerLayout) findViewById(R.id.drawerLayout);
        if (mDrawerLayout != null) { // xlarge-land has no drawer layout (drawer is always open)
            mDrawerToggle = new ActionBarDrawerToggle(this, mDrawerLayout,
                    R.string.drawer_open, R.string.drawer_close);
            mDrawerLayout.addDrawerListener(mDrawerToggle);
            getActionBar().setDisplayHomeAsUpEnabled(true);
            getActionBar().setHomeButtonEnabled(true);
        }

        mDrawerList = (ListView) findViewById(R.id.leftDrawer);
        mStateAdapter = new StateAdapter(this);
        mDrawerList.setAdapter(mStateAdapter);

        mApplication = (ViewerApplication) getApplication();
        mApplication.setViewerActivity(this);
    }

    @Override
    protected void onPostCreate(Bundle savedInstanceState) {
        super.onPostCreate(savedInstanceState);
        if (mDrawerToggle != null) {
            mDrawerToggle.syncState();
        }
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        if (mDrawerToggle != null) {
            mDrawerToggle.onConfigurationChanged(newConfig);
        }
    }

    @Override
    protected void onDestroy() {
        mApplication.setViewerActivity(null);
        super.onDestroy();
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        if (mApplication.getNativeHandle() != 0) {
            onSurfaceCreated(mApplication.getNativeHandle(), holder.getSurface());
        }
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        if (mApplication.getNativeHandle() != 0) {
            onSurfaceChanged(mApplication.getNativeHandle(), holder.getSurface());
        }
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        if (mApplication.getNativeHandle() != 0) {
            onSurfaceDestroyed(mApplication.getNativeHandle());
        }
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        int count = event.getPointerCount();
        for (int i = 0; i < count; i++) {
            final float x = event.getX(i);
            final float y = event.getY(i);
            final int owner = event.getPointerId(i);
            int action = event.getAction() & MotionEvent.ACTION_MASK;
            onTouched(mApplication.getNativeHandle(), owner, action, x, y);
        }
        return true;
    }

    public void setState(String stateJson) {
        mStateAdapter.setState(stateJson);
    }

    public void onStateChanged(String stateName, String stateValue) {
        onUIStateChanged(mApplication.getNativeHandle(), stateName, stateValue);
    }
}
