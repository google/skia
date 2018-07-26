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

package com.google.ar.core.examples.java.helloskar;

import android.animation.PropertyValuesHolder;
import android.animation.ValueAnimator;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.PointF;
import android.graphics.PorterDuff;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.opengl.Matrix;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.design.internal.BottomNavigationMenuView;
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
import com.google.ar.core.examples.java.common.helpers.CameraPermissionHelper;
import com.google.ar.core.examples.java.common.helpers.DisplayRotationHelper;
import com.google.ar.core.examples.java.common.helpers.FullScreenHelper;
import com.google.ar.core.examples.java.common.helpers.SnackbarHelper;
import com.google.ar.core.examples.java.common.helpers.TapHelper;
import com.google.ar.core.examples.java.common.rendering.BackgroundRenderer;
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

public class HelloSkARActivity extends AppCompatActivity implements GLSurfaceView.Renderer {
    public enum DrawingType {
        circle, rect, text, animation
    }

    private static final String TAG = HelloSkARActivity.class.getSimpleName();

    //Simple SurfaceView used to draw 2D objects on top of the GLSurfaceView
    private ARSurfaceView arSurfaceView;
    private Canvas canvas;
    private SurfaceHolder holder;

    //GLSurfaceView used to draw 3D objects & camera input
    private GLSurfaceView glSurfaceView;

    //ARSession
    private Session session;

    private boolean installRequested;
    private final SnackbarHelper messageSnackbarHelper = new SnackbarHelper();
    private DisplayRotationHelper displayRotationHelper;
    private TapHelper tapHelper;

    // OpenGL background renderer
    private final BackgroundRenderer backgroundRenderer = new BackgroundRenderer();

    // 2D Renderer
    private DrawManager drawManager = new DrawManager();
    private DrawingType currentDrawabletype = DrawingType.circle;
    private boolean drawSmoothPainting = true;

    // Temporary matrix allocated here to reduce number of allocations for each frame.
    private final float[] anchorMatrix = new float[16];

    PointF previousEvent;

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

        Toolbar myToolbar = (Toolbar) findViewById(R.id.my_toolbar);
        setSupportActionBar(myToolbar);


        //hide notifications bar
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);

        arSurfaceView = findViewById(R.id.arsurfaceview);
        glSurfaceView = findViewById(R.id.glsurfaceview);
        arSurfaceView.bringToFront();
        arSurfaceView.setLayerType(View.LAYER_TYPE_HARDWARE, null);
        displayRotationHelper = new DisplayRotationHelper(/*context=*/ this);

        // Set up tap listener.
        tapHelper = new TapHelper(/*context=*/ this);
        glSurfaceView.setOnTouchListener(tapHelper);

        // Set up renderer.
        glSurfaceView.setPreserveEGLContextOnPause(true);
        glSurfaceView.setEGLContextClientVersion(2);
        glSurfaceView.setEGLConfigChooser(8, 8, 8, 8, 16, 0); // Alpha used for plane blending.
        glSurfaceView.setRenderer(this);
        glSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);

        installRequested = false;

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

        // Animator set up
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
        canvas = null;
        holder = null;

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
                        if (anchors.size() >= 20) {
                            anchors.get(0).detach();
                            anchors.remove(0);
                        }
                        anchors.add(hit.createAnchor());
                        break;
                    }
                }
            }

            // Draw background with OpenGL.
            // TODO: possibly find a way to extract texture and draw on Canvas
            backgroundRenderer.draw(frame);

            // If not tracking, don't draw objects
            if (camera.getTrackingState() == TrackingState.PAUSED) {
                return;
            }

            // Get projection matrix.
            float[] projmtx = new float[16];
            camera.getProjectionMatrix(projmtx, 0, 0.1f, 100.0f);
            drawManager.updateProjectionMatrix(projmtx);

            // Get camera matrix and draw.
            float[] viewmtx = new float[16];
            camera.getViewMatrix(viewmtx, 0);
            drawManager.updateViewMatrix(viewmtx);

            final float[] colorCorrectionRgba = new float[4];
            frame.getLightEstimate().getColorCorrection(colorCorrectionRgba, 0);
            drawManager.updateLightColorFilter(colorCorrectionRgba);

            // Building finger painting
            TapHelper.ScrollEvent holdTap = tapHelper.holdPoll();
            if (holdTap != null && camera.getTrackingState() == TrackingState.TRACKING) {
                for (HitResult hit : frame.hitTest(holdTap.e)) {
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

                        // Get hit point transform, apply it to the origin
                        float[] gm = new float[16];
                        hit.getHitPose().toMatrix(gm, 0);
                        float[] point = {0, 0, 0, 1};
                        Matrix.multiplyMV(point, 0, gm, 0, point, 0);

                        if (drawManager.fingerPainting.isEmpty()) {
                            drawManager.fingerPainting.addPoint(new PointF(0, 0), true);

                            // Get model matrix of first point
                            float[] m = new float[16];
                            hit.getHitPose().toMatrix(m, 0);
                            drawManager.fingerPainting.setModelMatrix(m);
                        } else {
                            float localDistanceScale = 1000;
                            PointF distance = new PointF(point[0] - previousEvent.x,
                                                         point[2] - previousEvent.y);

                            if (distance.length() < 0.05f) {
                                continue;
                            }

                            // New point is distance + old point
                            PointF p = new PointF(distance.x * localDistanceScale
                                                   + drawManager.fingerPainting.previousPoint.x,
                                                  distance.y * localDistanceScale
                                                   + drawManager.fingerPainting.previousPoint.y);

                            drawManager.fingerPainting.addPoint(p, holdTap.isStartOfScroll);
                        }

                        previousEvent = new PointF(point[0], point[2]);
                        break;
                    }
                }
            }

            // Drawing on Canvas (SurfaceView)
            if (arSurfaceView.isRunning()) {
                // Lock canvas
                SurfaceHolder holder = arSurfaceView.getHolder();
                Canvas canvas = holder.lockHardwareCanvas();
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
            if (holder != null && canvas != null) {
                holder.unlockCanvasAndPost(canvas);
            }
            Log.e(TAG, "Exception on the OpenGL thread", t);
        }
    }

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
            // Get the current pose of an Anchor in world space. The Anchor pose is updated
            // during calls to session.update() as ARCore refines its estimate of the world.
            anchor.getPose().toMatrix(anchorMatrix, 0);
            drawManager.modelMatrices.add(0, anchorMatrix);

            switch (currentDrawabletype) {
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
        drawManager.fingerPainting.setSmoothness(drawSmoothPainting);
        drawManager.drawFingerPainting(canvas);
    }

    // Menu functions
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.main_menu, menu);
        return true;
    }

    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.reset_paint:
                drawManager.fingerPainting.reset();
                return true;
            case R.id.smooth_paint:
                drawSmoothPainting = true;
                return true;
            case R.id.rough_paint:
                drawSmoothPainting = false;
                return true;
            case R.id.draw_circle:
                currentDrawabletype = DrawingType.circle;
                return true;
            case R.id.draw_rect:
                currentDrawabletype = DrawingType.rect;
                return true;
            case R.id.draw_animation:
                currentDrawabletype = DrawingType.animation;
                return true;
            case R.id.draw_text:
                currentDrawabletype = DrawingType.text;
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }
}
