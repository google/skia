/*
 * Copyright 2018 Google LLC All Rights Reserved.
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

package com.google.skar.examples.helloskar.app;

import android.animation.PropertyValuesHolder;
import android.animation.ValueAnimator;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.PorterDuff;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.opengl.Matrix;
import android.os.Bundle;
import android.support.annotation.NonNull;

import android.support.design.widget.BottomNavigationView;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.View;
import android.view.WindowManager;
import android.widget.Toast;

import com.google.ar.core.Anchor;
import com.google.ar.core.ArCoreApk;
import com.google.ar.core.Camera;
import com.google.ar.core.Frame;
import com.google.ar.core.HitResult;
import com.google.ar.core.Plane;
import com.google.ar.core.Point;
import com.google.ar.core.Point.OrientationMode;
import com.google.ar.core.PointCloud;
import com.google.ar.core.Session;
import com.google.ar.core.Trackable;
import com.google.ar.core.TrackingState;

import com.google.ar.core.examples.java.helloskar.R;
import com.google.skar.examples.helloskar.helpers.CameraPermissionHelper;
import com.google.skar.examples.helloskar.helpers.DisplayRotationHelper;
import com.google.skar.examples.helloskar.helpers.FullScreenHelper;
import com.google.skar.examples.helloskar.helpers.GestureHelper;
import com.google.skar.examples.helloskar.helpers.SnackbarHelper;

import com.google.skar.examples.helloskar.rendering.BackgroundRenderer;
import com.google.skar.examples.helloskar.rendering.DrawManager;

import com.google.ar.core.exceptions.CameraNotAvailableException;
import com.google.ar.core.exceptions.UnavailableApkTooOldException;
import com.google.ar.core.exceptions.UnavailableArcoreNotInstalledException;
import com.google.ar.core.exceptions.UnavailableDeviceNotCompatibleException;
import com.google.ar.core.exceptions.UnavailableSdkTooOldException;
import com.google.ar.core.exceptions.UnavailableUserDeclinedInstallationException;

import java.io.IOException;
import java.util.ArrayList;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * This is a simple example that shows how to create an augmented reality (AR) application using the
 * ARCore API. The application will display any detected planes and will allow the user to tap on a
 * plane to place 2D objects
 */

public class HelloCanvasAR extends AppCompatActivity implements GLSurfaceView.Renderer {
    private static final String TAG = HelloCanvasAR.class.getSimpleName();
    private final int MAX_NUMBER_DRAWABLES = 50; // Arbitrary limit to the # of anchors to store

    // Simple SurfaceView used to draw 2D objects on top of the GLSurfaceView
    private CanvasARSurfaceView arSurfaceView;
    private SurfaceHolder holder;

    // GLSurfaceView used to draw 3D objects & camera input
    private GLSurfaceView glSurfaceView;

    // ARSession
    private Session session;

    // OpenGL background renderer
    private final BackgroundRenderer backgroundRenderer = new BackgroundRenderer();

    // 2D Renderer
    private DrawManager drawManager = new DrawManager();

    private boolean installRequested;
    private final SnackbarHelper messageSnackbarHelper = new SnackbarHelper();
    private DisplayRotationHelper displayRotationHelper;
    private GestureHelper tapHelper;

    // Temporary matrix allocated here to reduce number of allocations for each frame.
    private final float[] anchorMatrix = new float[16];

    // Anchors created from taps used for object placing.
    private final ArrayList<Anchor> anchors = new ArrayList<>();

    // Animation fields
    float radius;
    String PROPERTY_RADIUS = "radius";
    ValueAnimator animator;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Menu tool bar set up
        Toolbar toolbar = findViewById(R.id.main_toolbar);
        setSupportActionBar(toolbar);

        // Hide notifications bar
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);

        // Canvas Surface View set up
        arSurfaceView = findViewById(R.id.canvas_surfaceview);
        glSurfaceView = findViewById(R.id.gl_surfaceview);
        arSurfaceView.bringToFront();
        arSurfaceView.setLayerType(View.LAYER_TYPE_HARDWARE, null);
        holder = arSurfaceView.getHolder();

        // Set up tap listener.
        tapHelper = new GestureHelper(this);

        glSurfaceView.setOnTouchListener(tapHelper);

        // Set up renderer.
        glSurfaceView.setPreserveEGLContextOnPause(true);
        glSurfaceView.setEGLContextClientVersion(2);
        glSurfaceView.setEGLConfigChooser(8, 8, 8, 8, 16, 0); // Alpha used for plane blending.
        glSurfaceView.setRenderer(this);
        glSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        displayRotationHelper = new DisplayRotationHelper(this);
        installRequested = false;

        // Set up finger painting palette bar
        BottomNavigationView bottomNav = findViewById(R.id.palette);
        bottomNav.setOnNavigationItemSelectedListener(new BottomNavigationView.OnNavigationItemSelectedListener() {
            @Override
            public boolean onNavigationItemSelected(@NonNull MenuItem item) {
                switch (item.getItemId()) {
                    case R.id.palette_green:
                        drawManager.fingerPainting.setColor(Color.GREEN);
                        return true;
                    case R.id.palette_red:
                        drawManager.fingerPainting.setColor(Color.RED);
                        return true;
                    case R.id.palette_reset:
                        drawManager.fingerPainting.reset();
                        return true;
                    default:
                        return true;
                }
            }
        });

        // Value Animator set up
        PropertyValuesHolder propertyRadius = PropertyValuesHolder.ofFloat(PROPERTY_RADIUS, 0, 0.5f);
        animator = new ValueAnimator();
        animator.setValues(propertyRadius);
        animator.setDuration(1000);
        animator.setRepeatCount(ValueAnimator.INFINITE);
        animator.setRepeatMode(ValueAnimator.REVERSE);
        animator.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator animation) {
                radius = (float) animation.getAnimatedValue(PROPERTY_RADIUS);
            }
        });
        animator.start();
    }

    @Override
    protected void onResume() {
        super.onResume();

        if (session == null) {
            Exception exception = null;
            String message = null;
            try {
                switch (ArCoreApk.getInstance().requestInstall(this, !installRequested)) {
                    case INSTALL_REQUESTED:
                        installRequested = true;
                        return;
                    case INSTALLED:
                        break;
                }

                // ARCore requires camera permissions to operate. If we did not yet obtain runtime
                // permission on Android M and above, now is a good time to ask the user for it.
                if (!CameraPermissionHelper.hasCameraPermission(this)) {
                    CameraPermissionHelper.requestCameraPermission(this);
                    return;
                }

                // Create the session.
                session = new Session(/* context= */ this);

            } catch (UnavailableArcoreNotInstalledException
                    | UnavailableUserDeclinedInstallationException e) {
                message = "Please install ARCore";
                exception = e;
            } catch (UnavailableApkTooOldException e) {
                message = "Please update ARCore";
                exception = e;
            } catch (UnavailableSdkTooOldException e) {
                message = "Please update this app";
                exception = e;
            } catch (UnavailableDeviceNotCompatibleException e) {
                message = "This device does not support AR";
                exception = e;
            } catch (Exception e) {
                message = "Failed to create AR session";
                exception = e;
            }

            if (message != null) {
                messageSnackbarHelper.showError(this, message);
                Log.e(TAG, "Exception creating session", exception);
                return;
            }
        }

        // Note that order matters - see the note in onPause(), the reverse applies here.
        try {
            session.resume();
        } catch (CameraNotAvailableException e) {
            messageSnackbarHelper.showError(this, "Camera not available. Please restart the app.");
            session = null;
            return;
        }

        glSurfaceView.onResume();
        displayRotationHelper.onResume();
        messageSnackbarHelper.showMessage(this, "Searching for surfaces...");
    }

    @Override
    public void onPause() {
        super.onPause();
        if (session != null) {
            displayRotationHelper.onPause();
            glSurfaceView.onPause();
            session.pause();
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

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        FullScreenHelper.setFullScreenOnWindowFocusChanged(this, hasFocus);
    }

    /************** GLSurfaceView Methods ****************************/
    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        GLES20.glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        // Prepare the rendering objects. This involves reading shaders, so may throw an IOException.
        try {
            // Create the texture and pass it to ARCore session to be filled during update().
            backgroundRenderer.createOnGlThread( this);
            drawManager.initializePlaneShader(this, "models/trigrid.png");
        } catch (IOException e) {
            Log.e(TAG, "Failed to read an asset file", e);
        }
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        displayRotationHelper.onSurfaceChanged(width, height);
        GLES20.glViewport(0, 0, width, height);

        // Send viewport information to 2D AR drawing manager
        drawManager.updateViewport(width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        Canvas canvas = null;

        // Clear screen to notify driver it should not load any pixels from previous frame.
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);

        if (session == null) {
            return;
        }

        // Notify ARCore session that the view size changed so that the perspective matrix and
        // the video background can be properly adjusted.
        displayRotationHelper.updateSessionIfNeeded(session);

        try {
            session.setCameraTextureName(backgroundRenderer.getTextureId());
            Frame frame = session.update();
            Camera camera = frame.getCamera();

            // Query information from single tap gestures to get anchors
            handleSingleTaps(frame, camera);

            // Draw background with OpenGL.
            // TODO: possibly find a way to extract texture and draw on Canvas
            backgroundRenderer.draw(frame);

            // If not tracking, don't draw objects
            if (camera.getTrackingState() == TrackingState.PAUSED) {
                return;
            }

            // Get projection matrix.
            float[] projMatrix = new float[16];
            camera.getProjectionMatrix(projMatrix, 0, 0.1f, 100.0f);
            drawManager.updateProjectionMatrix(projMatrix);

            // Get camera matrix and draw.
            float[] viewMatrix = new float[16];
            camera.getViewMatrix(viewMatrix, 0);
            drawManager.updateViewMatrix(viewMatrix);

            final float[] colorCorrectionRgba = new float[4];
            frame.getLightEstimate().getColorCorrection(colorCorrectionRgba, 0);
            drawManager.updateLightColorFilter(colorCorrectionRgba);

            // Query information from scrolling gestures to build finger paintings
            handleHoldTaps(frame, camera);

            // Drawing on Canvas (SurfaceView)
            if (arSurfaceView.isRunning()) {
                // Lock canvas
                canvas = holder.lockHardwareCanvas();
                canvas.drawColor(Color.TRANSPARENT, PorterDuff.Mode.CLEAR);

                // Draw point cloud
                PointCloud pointCloud = frame.acquirePointCloud();
                drawPointCloud(canvas, pointCloud);
                pointCloud.release();

                // Draw planes
                // Check if we detected at least one plane. If so, hide the loading message.
                if (messageSnackbarHelper.isShowing()) {
                    for (Plane plane : session.getAllTrackables(Plane.class)) {
                        if (plane.getType() == com.google.ar.core.Plane.Type.HORIZONTAL_UPWARD_FACING
                                && plane.getTrackingState() == TrackingState.TRACKING) {
                            messageSnackbarHelper.hide(this);
                            break;
                        }
                    }
                }

                // Draw planes
                drawPlanes(canvas, camera);

                // Draw models
                drawModels(canvas);

                // Draw finger painting
                drawFingerPainting(canvas);

                // Unlock canvas
                holder.unlockCanvasAndPost(canvas);
            }
        } catch (Throwable t) {
            // Avoid crashing the application due to unhandled exceptions.
            if (canvas != null) {
                holder.unlockCanvasAndPost(canvas);
            }
            Log.e(TAG, "Exception on the OpenGL thread", t);
        }
    }

    /**************************** Gesture helpers ******************************/
    /**
     * Given a Frame and a Camera, perform hit tests on stored UI touch events. If a hit test is
     * successful, construct an Anchor at the hit position and add it to the set of anchors.
     * @param frame     Frame of this update() call
     * @param camera    Camera of this update() call
     */
    private void handleSingleTaps(Frame frame, Camera camera) {
        MotionEvent tap = tapHelper.poll();
        if (tap != null && camera.getTrackingState() == TrackingState.TRACKING) {
            for (HitResult hit : frame.hitTest(tap)) {
                // Check if any plane was hit, and if it was hit inside the plane polygon
                Trackable trackable = hit.getTrackable();
                // Creates an anchor if a plane or an oriented point was hit.
                if ((trackable instanceof Plane
                        && ((Plane) trackable).isPoseInPolygon(hit.getHitPose())
                        && (DrawManager.calculateDistanceToPlane(hit.getHitPose(), camera.getPose())
                        > 0))
                        || (trackable instanceof Point
                        && ((Point) trackable).getOrientationMode()
                        == OrientationMode.ESTIMATED_SURFACE_NORMAL)) {
                    if (anchors.size() >= MAX_NUMBER_DRAWABLES) {
                        anchors.get(0).detach();
                        anchors.remove(0);
                    }
                    anchors.add(hit.createAnchor());
                    break;
                }
            }
        }
    }

    /**
     * Given a Frame and a Camera, perform hit tests on stored UI touch events. If a hit test is
     * successful, construct an Anchor at the hit position and add it to the set of anchors.
     * @param frame     Frame of this update() call
     * @param camera    Camera of this update() call
     */
    private void handleHoldTaps(Frame frame, Camera camera) {
        // Building finger painting
        GestureHelper.ScrollEvent holdTap = tapHelper.holdPoll();
        if (holdTap != null && camera.getTrackingState() == TrackingState.TRACKING) {
            for (HitResult hit : frame.hitTest(holdTap.event)) {
                // Check if any plane was hit, and if it was hit inside the plane polygon
                Trackable trackable = hit.getTrackable();
                // Creates an anchor if a plane or an oriented point was hit.
                if ((trackable instanceof Plane
                        && ((Plane) trackable).isPoseInPolygon(hit.getHitPose())
                        && (DrawManager.calculateDistanceToPlane(hit.getHitPose(), camera.getPose())
                        > 0))
                        || (trackable instanceof Point
                        && ((Point) trackable).getOrientationMode()
                        == OrientationMode.ESTIMATED_SURFACE_NORMAL)) {

                    // Get hit point transform, apply it to the origin --> point is not in hit
                    // location on the plane
                    float[] modelMatrix = new float[16];
                    hit.getHitPose().toMatrix(modelMatrix, 0);
                    float[] hitLocation = {0, 0, 0, 1};
                    Matrix.multiplyMV(hitLocation, 0, modelMatrix, 0,
                                      hitLocation, 0);

                    if (! drawManager.fingerPainting.computeNextPoint(hitLocation, modelMatrix, holdTap)) {
                        // Try to add the next point to the finger painting. If return value
                        // is false, then keep looping
                        continue;
                    }

                    break;
                }
            }
        }
    }

    /**************************** Drawing helpers ******************************/
    // Helper drawing functions that invoke drawManager
    private void drawPlanes(Canvas canvas, Camera camera) {
        drawManager.drawPlanes(canvas, camera.getPose(), session.getAllTrackables(Plane.class));
    }

    private void drawPointCloud(Canvas canvas, PointCloud cloud) {
        drawManager.drawPointCloud(canvas, cloud);
    }

    private void drawModels(Canvas canvas) {
        for (Anchor anchor : anchors) {
            if (anchor.getTrackingState() != TrackingState.TRACKING) {
                continue;
            }
            // Get the current pose of an Anchor in world space
            anchor.getPose().toMatrix(anchorMatrix, 0);
            drawManager.modelMatrices.add(0, anchorMatrix);

            switch (drawManager.currentDrawabletype) {
                case circle:
                    drawManager.drawCircle(canvas);
                    break;
                case rect:
                    drawManager.drawRect(canvas);
                    break;
                case animation:
                    drawManager.drawAnimatedRoundRect(canvas, radius);
                    break;
                case text:
                    drawManager.drawText(canvas, "Android");
                    break;
                default:
                    drawManager.drawCircle(canvas);
                    break;
            }
        }
    }

    private void drawFingerPainting(Canvas canvas) {
        drawManager.fingerPainting.setSmoothness(drawManager.drawSmoothPainting);
        drawManager.fingerPainting.buildPath();
        drawManager.drawFingerPainting(canvas);
    }

    /**************************** UI helpers ******************************/
    // Tool bar functions
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.main_menu, menu);

        menu.setGroupCheckable(R.id.menu_drawables, true, true);
        return true;
    }

    public boolean onOptionsItemSelected(MenuItem item) {
        item.setChecked(!item.isChecked());
        switch (item.getItemId()) {
            case R.id.smooth_paint:
                drawManager.drawSmoothPainting = item.isChecked();
                return true;
            case R.id.draw_circle:
                drawManager.currentDrawabletype = DrawManager.DrawingType.circle;
                return true;
            case R.id.draw_rect:
                drawManager.currentDrawabletype = DrawManager.DrawingType.rect;
                return true;
            case R.id.draw_text:
                drawManager.currentDrawabletype = DrawManager.DrawingType.text;
                return true;
            case R.id.draw_animation:
                drawManager.currentDrawabletype = DrawManager.DrawingType.animation;
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }
}
