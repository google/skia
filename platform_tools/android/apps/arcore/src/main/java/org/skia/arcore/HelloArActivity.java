/*
 * Copyright 2017 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.skia.arcore;

import android.app.Activity;
import android.content.Context;
import android.hardware.display.DisplayManager;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Handler;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.ActionMode;
import android.view.ContextMenu;
import android.view.GestureDetector;
import android.view.Gravity;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.EditText;
import android.widget.PopupMenu;
import android.widget.Toast;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * This is a simple example that shows how to create an augmented reality (AR) application using the
 * ARCore C API.
 */
public class HelloArActivity extends AppCompatActivity
        implements GLSurfaceView.Renderer, DisplayManager.DisplayListener {
    private static final String TAG = HelloArActivity.class.getSimpleName();
    private static final int SNACKBAR_UPDATE_INTERVAL_MILLIS = 1000; // In milliseconds.

    private GLSurfaceView mSurfaceView;
    private Activity activity = null;
    private boolean mViewportChanged = false;
    private int mViewportWidth;
    private int mViewportHeight;
    private View contextView = null;
    private int mCurrentObjectRotation = 0;
    private float mCurrentValue = 0;
    private float X = 0;
    private float Y = 0;

    private boolean toEdit = false;

    // Opaque native pointer to the native application instance.
    private long mNativeApplication;
    private GestureDetector mGestureDetector;

    private Snackbar mLoadingMessageSnackbar;
    private Handler mPlaneStatusCheckingHandler;
    private final Runnable mPlaneStatusCheckingRunnable =
            new Runnable() {
                @Override
                public void run() {
                    // The runnable is executed on main UI thread.
                    try {
                        if (JniInterface.hasDetectedPlanes(mNativeApplication)) {
                            if (mLoadingMessageSnackbar != null) {
                                mLoadingMessageSnackbar.dismiss();
                            }
                            mLoadingMessageSnackbar = null;
                        } else {
                            mPlaneStatusCheckingHandler.postDelayed(
                                    mPlaneStatusCheckingRunnable, SNACKBAR_UPDATE_INTERVAL_MILLIS);
                        }
                    } catch (Exception e) {
                        Log.e(TAG, e.getMessage());
                    }
                }
            };
    private int mDrawMode = -1;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Toolbar myToolbar = (Toolbar) findViewById(R.id.my_toolbar);
        setSupportActionBar(myToolbar);

        activity = this;

        //hide notifications bar
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);

        mSurfaceView = (GLSurfaceView) findViewById(R.id.surfaceview);

        mGestureDetector =
                new GestureDetector(
                        this,
                        new GestureDetector.SimpleOnGestureListener() {
                            @Override
                            public boolean onSingleTapUp(final MotionEvent e) {
                                toEdit = JniInterface.onTouchedFirst(mNativeApplication, e.getX(), e.getY(), mDrawMode);

                                Log.i(TAG, "toEdit: " + toEdit);
                                X = e.getX();
                                Y = e.getY();
                                contextView.showContextMenu(e.getX(), e.getY());
                                return true;
                            }

                            @Override
                            public boolean onScroll (MotionEvent e1, MotionEvent e2, float distanceX, float distanceY) {
                                Log.i(TAG, "Scrolling!");
                                JniInterface.onTouchTranslate(mNativeApplication, e2.getX(), e2.getY());
                                return true;
                            }

                            @Override
                            public boolean onDown(MotionEvent e) {
                                return true;
                            }
                        });

        mSurfaceView.setOnTouchListener(
                new View.OnTouchListener() {
                    @Override
                    public boolean onTouch(View v, MotionEvent event) {
                        return mGestureDetector.onTouchEvent(event);
                    }
                });

        // Set up renderer.
        mSurfaceView.setPreserveEGLContextOnPause(true);
        mSurfaceView.setEGLContextClientVersion(2);
        mSurfaceView.setEGLConfigChooser(8, 8, 8, 8, 16, 0); // Alpha used for plane blending.
        mSurfaceView.setRenderer(this);
        mSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);

        JniInterface.assetManager = getAssets();
        mNativeApplication = JniInterface.createNativeApplication(getAssets());

        mPlaneStatusCheckingHandler = new Handler();

        //Floating context menu
        contextView = findViewById(R.id.menuView);
        this.registerForContextMenu(contextView);
        View.OnLongClickListener listener = new View.OnLongClickListener() {
            @Override
            public boolean onLongClick(View v) {
                activity.closeContextMenu();
                return false;
            }
        };
        contextView.setOnLongClickListener(listener);
    }

    @Override
    public void onCreateContextMenu(ContextMenu menu, View v, ContextMenu.ContextMenuInfo menuInfo)
    {
        super.onCreateContextMenu(menu, v, menuInfo);

        if (!toEdit) {
            MenuInflater inflater = getMenuInflater();
            inflater.inflate(R.menu.draw_menu, menu);
            menu.setHeaderTitle("Draw Options");
        }

        v.setClickable(false);
        v.setFocusable(false);
    }

    @Override
    public boolean onContextItemSelected(MenuItem item) {
        AdapterView.AdapterContextMenuInfo info = (AdapterView.AdapterContextMenuInfo) item.getMenuInfo();
        switch (item.getItemId()) {
            case R.id.draw_text:
                JniInterface.onTouchedFinal(mNativeApplication, 0);
                return true;
            case R.id.draw_circle:
                JniInterface.onTouchedFinal(mNativeApplication, 1);
                return true;
            case R.id.draw_rect:
                JniInterface.onTouchedFinal(mNativeApplication, 2);
                return true;
            case R.id.edit_size:
                return true;
            case R.id.edit_text:
                return true;
            default:
                return super.onContextItemSelected(item);
        }
    }

    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.rotation_mode, menu);
        return true;
    }

    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.rotation_axis_aligned:
                mCurrentObjectRotation = 0;
                break;
            case R.id.rotation_camera_aligned:
                mCurrentObjectRotation = 1;
                break;
            case R.id.rotation_snap_aligned:
                mCurrentObjectRotation = 2;
                break;
            case R.id.action:
                mCurrentValue = 180;
                JniInterface.onAction(mNativeApplication, mCurrentValue);
                return true;
            default:
                return true;
        }
        JniInterface.onObjectRotationChanged(mNativeApplication, mCurrentObjectRotation);
        return true;
    }

    @Override
    protected void onResume() {
        super.onResume();
        // ARCore requires camera permissions to operate. If we did not yet obtain runtime
        // permission on Android M and above, now is a good time to ask the user for it.
        if (!CameraPermissionHelper.hasCameraPermission(this)) {
            CameraPermissionHelper.requestCameraPermission(this);
            return;
        }

        JniInterface.onResume(mNativeApplication, getApplicationContext(), this);
        mSurfaceView.onResume();

        mLoadingMessageSnackbar =
                Snackbar.make(
                        HelloArActivity.this.findViewById(android.R.id.content),
                        "Searching for surfaces...",
                        Snackbar.LENGTH_INDEFINITE);
        // Set the snackbar background to light transparent black color.
        mLoadingMessageSnackbar.getView().setBackgroundColor(0xbf323232);
        mLoadingMessageSnackbar.show();
        mPlaneStatusCheckingHandler.postDelayed(
                mPlaneStatusCheckingRunnable, SNACKBAR_UPDATE_INTERVAL_MILLIS);

        // Listen to display changed events to detect 180° rotation, which does not cause a config
        // change or view resize.
        getSystemService(DisplayManager.class).registerDisplayListener(this, null);
    }

    @Override
    public void onPause() {
        super.onPause();
        mSurfaceView.onPause();
        JniInterface.onPause(mNativeApplication);

        mPlaneStatusCheckingHandler.removeCallbacks(mPlaneStatusCheckingRunnable);

        getSystemService(DisplayManager.class).unregisterDisplayListener(this);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        // Synchronized to avoid racing onDrawFrame.
        synchronized (this) {
            JniInterface.destroyNativeApplication(mNativeApplication);
            mNativeApplication = 0;
        }
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            // Standard Android full-screen functionality.
            getWindow()
                    .getDecorView()
                    .setSystemUiVisibility(
                            View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                                    | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                                    | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                                    | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                                    | View.SYSTEM_UI_FLAG_FULLSCREEN
                                    | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
            getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        }
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        GLES20.glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        JniInterface.onGlSurfaceCreated(mNativeApplication);
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        mViewportWidth = width;
        mViewportHeight = height;
        mViewportChanged = true;
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        // Synchronized to avoid racing onDestroy.
        synchronized (this) {
            if (mNativeApplication == 0) {
                return;
            }
            if (mViewportChanged) {
                int displayRotation = getWindowManager().getDefaultDisplay().getRotation();
                JniInterface.onDisplayGeometryChanged(
                        mNativeApplication, displayRotation, mViewportWidth, mViewportHeight);
                mViewportChanged = false;
            }
            JniInterface.onGlSurfaceDrawFrame(mNativeApplication);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] results) {
        if (!CameraPermissionHelper.hasCameraPermission(this)) {
            Toast.makeText(this, "Camera permission is needed to run this application", Toast.LENGTH_LONG)
                    .show();
            if (!CameraPermissionHelper.shouldShowRequestPermissionRationale(this)) {
                // Permission denied with checking "Do not ask again".
                CameraPermissionHelper.launchPermissionSettings(this);
            }
            finish();
        }
    }

    // DisplayListener methods
    @Override
    public void onDisplayAdded(int displayId) {
    }

    @Override
    public void onDisplayRemoved(int displayId) {
    }

    @Override
    public void onDisplayChanged(int displayId) {
        mViewportChanged = true;
    }
}
