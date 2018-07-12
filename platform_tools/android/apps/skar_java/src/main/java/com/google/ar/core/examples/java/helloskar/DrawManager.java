package com.google.ar.core.examples.java.helloskar;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.BitmapShader;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.ColorFilter;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.PointF;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffColorFilter;
import android.graphics.RectF;
import android.graphics.Shader;
import android.opengl.Matrix;
import com.google.ar.core.Plane;
import com.google.ar.core.PointCloud;
import com.google.ar.core.Pose;
import com.google.ar.core.TrackingState;
import com.google.skar.SkARMatrix;
import com.google.skar.SkARUtil;
import java.io.IOException;
import java.nio.FloatBuffer;
import java.util.ArrayList;
import java.util.Collection;

/**
 * Sample class that handles drawing different types of geometry using the matrices provided
 * by ARCore. The matrices are handled by SkARMatrix in order to be passed to the drawing
 * Canvas.
 */

public class DrawManager {
    private float[] projectionMatrix = new float[16];
    private float[] viewMatrix = new float[16];
    private float viewportWidth;
    private float viewportHeight;
    private ColorFilter lightFilter;
    private BitmapShader planeShader;
    private Bitmap planeTexture;
    public ArrayList<float[]> modelMatrices = new ArrayList<>();

    public void updateViewport(float width, float height) {
        viewportWidth = width;
        viewportHeight = height;
    }

    public void updateProjectionMatrix(float[] projectionMatrix) {
        this.projectionMatrix = projectionMatrix;
    }

    public void updateViewMatrix(float[] viewMatrix) {
        this.viewMatrix = viewMatrix;
    }

    public void updateLightColorFilter(float[] colorCorr) {
        lightFilter = SkARUtil.createLightCorrectionColorFilter(colorCorr);
    }

    // Sample function for drawing a circle
    public void drawCircle(Canvas canvas) {
        if (modelMatrices.isEmpty()) {
            return;
        }
        Paint p = new Paint();
        p.setColorFilter(lightFilter);
        p.setARGB(180, 100, 0, 0);

        canvas.save();
        canvas.setMatrix(SkARMatrix.createPerspectiveMatrix(modelMatrices.get(0),
                viewMatrix, projectionMatrix, viewportWidth, viewportHeight));
        canvas.drawCircle(0, 0, 0.1f, p);
        canvas.restore();
    }

    // Sample function for drawing an animated round rect
    public void drawAnimatedRoundRect(Canvas canvas, float radius) {
        if (modelMatrices.isEmpty()) {
            return;
        }
        Paint p = new Paint();
        p.setColorFilter(lightFilter);
        p.setARGB(180, 100, 0, 100);

        canvas.save();
        canvas.setMatrix(SkARMatrix.createPerspectiveMatrix(modelMatrices.get(0),
                viewMatrix, projectionMatrix, viewportWidth, viewportHeight));
        canvas.drawRoundRect(0,0, 0.5f, 0.5f, radius, radius, p);
        canvas.restore();
    }

    // Sample function for drawing a rect
    public void drawRect(Canvas canvas) {
        if (modelMatrices.isEmpty()) {
            return;
        }
        Paint p = new Paint();
        p.setColorFilter(lightFilter);
        p.setARGB(180, 0, 0, 255);
        canvas.save();
        canvas.setMatrix(SkARMatrix.createPerspectiveMatrix(modelMatrices.get(0),
                viewMatrix, projectionMatrix, viewportWidth, viewportHeight));
        RectF rect = new RectF(0, 0, 0.2f, 0.2f);
        canvas.drawRect(rect, p);
        canvas.restore();
    }

    // Sample function for drawing text on a canvas
    public void drawText(Canvas canvas, String text) {
        if (modelMatrices.isEmpty()) {
            return;
        }
        Paint p = new Paint();
        float textSize = 100;
        p.setColorFilter(lightFilter);
        p.setARGB(255, 0, 255, 0);
        p.setTextSize(textSize);

        float[] scaleMatrix = getTextScaleMatrix(textSize);
        float[] rotateMatrix = SkARMatrix.createXYtoXZRotationMatrix();
        float[][] matrices = { scaleMatrix, rotateMatrix, modelMatrices.get(0), viewMatrix,
                                projectionMatrix,
                                SkARMatrix.createViewportMatrix(viewportWidth, viewportHeight)};

        canvas.save();
        canvas.setMatrix(SkARMatrix.createMatrixFrom4x4(SkARMatrix.multiplyMatrices4x4(matrices)));
        canvas.drawText(text, 0, 0, p);
        canvas.restore();
    }

    // Sample function for drawing the AR point cloud
    public void drawPointCloud(Canvas canvas, PointCloud cloud) {
        FloatBuffer points = cloud.getPoints();
        int numberOfPoints = points.remaining() / 4;

        float[][] matrices = {viewMatrix, projectionMatrix, SkARMatrix.createViewportMatrix(viewportWidth, viewportHeight)};
        float[] vpv = SkARMatrix.multiplyMatrices4x4(matrices);

        float[] pointsToDraw = new float[numberOfPoints * 2];
        for (int i = 0; i < numberOfPoints; i++) {
            float[] point = {points.get(i * 4), points.get(i * 4 + 1), points.get(i * 4 + 2), 1};
            PointF p = SkARMatrix.multiplyMatrixVector(vpv, point, true);
            pointsToDraw[i * 2] = p.x;
            pointsToDraw[i * 2 + 1] = p.y;
        }

        Paint p = new Paint();
        p.setARGB(220, 20, 232, 255);
        p.setStrokeCap(Paint.Cap.SQUARE);
        p.setStrokeWidth(6.0f);

        canvas.save();
        float[] id = new float[16];
        Matrix.setIdentityM(id, 0);
        android.graphics.Matrix identity = SkARMatrix.createMatrixFrom4x4(id);
        canvas.setMatrix(identity);
        canvas.drawPoints(pointsToDraw, p);
        canvas.restore();
    }


    // Sample function for drawing AR planes
    public void drawPlanes(Canvas canvas, Pose cameraPose, Collection<Plane> allPlanes) {
        if (allPlanes.size() <= 0) {
            return;
        }

        for (Plane plane : allPlanes) {
            Plane subsumePlane = plane.getSubsumedBy();
            if (plane.getTrackingState() != TrackingState.TRACKING || subsumePlane != null) {
                continue;
            }

            float distance = calculateDistanceToPlane(plane.getCenterPose(), cameraPose);
            if (distance < 0) { // Plane is back-facing.
                continue;
            }


            // Get plane model matrix
            float[] model = new float[16];
            plane.getCenterPose().toMatrix(model, 0);

            // Initial rotation
            float[] initRot = SkARMatrix.createXYtoXZRotationMatrix();

            float[] initScale = new float[16];
            Matrix.setIdentityM(initScale, 0);
            Matrix.scaleM(initScale, 0, 1f, 1f, 1f);
            android.graphics.Matrix scale = SkARMatrix.createMatrixFrom4x4(SkARMatrix.multiplyMatrices4x4(new float[][] {initScale}));

            // Matrix = mvpv
            float[][] matrices = {initRot, model, viewMatrix, projectionMatrix, SkARMatrix.createViewportMatrix(viewportWidth, viewportHeight)};
            android.graphics.Matrix mvpv = SkARMatrix.createMatrixFrom4x4(SkARMatrix.multiplyMatrices4x4(matrices));

            canvas.save();

            canvas.setMatrix(mvpv);

            drawPlaneAsPath(canvas, plane);
            canvas.restore();
        }
    }

    // Helper function that draws an AR plane using a path
    private void drawPlaneAsPath(Canvas canvas, Plane plane) {
        int vertsSize = plane.getPolygon().limit() / 2;
        FloatBuffer polygon = plane.getPolygon();
        polygon.rewind();

        Path path = new Path();
        path.moveTo(polygon.get(0), polygon.get(1));
        for (int i = 1; i < vertsSize; i++) {
            path.lineTo(polygon.get(i * 2), polygon.get(i * 2 + 1));
        }
        path.close();

        Paint p = new Paint();
        p.setShader(planeShader);
        p.setColorFilter(new PorterDuffColorFilter(Color.argb(0.4f, 1, 0, 0), PorterDuff.Mode.SRC_ATOP));
        p.setAlpha(120);

        //canvas.drawPath(path, p); TODO: enable this when path is drawn on GPU

        RectF r = new RectF();
        path.computeBounds(r, true);
        canvas.drawRect(r, p);
    }

    public void initializePlaneShader(Context context, String gridDistanceTextureName) throws IOException {
        // Read the texture.
        planeTexture =
                BitmapFactory.decodeStream(context.getAssets().open(gridDistanceTextureName));
        // Set up the shader
        planeShader = new BitmapShader(planeTexture, Shader.TileMode.REPEAT, Shader.TileMode.REPEAT);
        android.graphics.Matrix m = new android.graphics.Matrix();
        m.setScale(0.0005f, 0.0005f);
        planeShader.setLocalMatrix(m);
    }

    private float[] getTextScaleMatrix(float size) {
        float scaleFactor = 1 / (size * 10);
        float[] initScale = new float[16];
        android.opengl.Matrix.setIdentityM(initScale, 0);
        android.opengl.Matrix.scaleM(initScale, 0, scaleFactor, scaleFactor, scaleFactor);
        return initScale;
    }

    public static float calculateDistanceToPlane(Pose planePose, Pose cameraPose) {
        float[] normal = new float[3];
        float cameraX = cameraPose.tx();
        float cameraY = cameraPose.ty();
        float cameraZ = cameraPose.tz();
        // Get transformed Y axis of plane's coordinate system.
        planePose.getTransformedAxis(1, 1.0f, normal, 0);
        // Compute dot product of plane's normal with vector from camera to plane center.
        return (cameraX - planePose.tx()) * normal[0]
                + (cameraY - planePose.ty()) * normal[1]
                + (cameraZ - planePose.tz()) * normal[2];
    }

    // Drawing plane with drawVertices
    // TODO: Wait until latest Android release for this to work..
    private void drawPlane(Canvas canvas, android.graphics.Matrix mvpv, Plane plane) {
        int vertsSize = plane.getPolygon().limit() / 2;
        FloatBuffer polygon = plane.getPolygon();
        float[] polyVerts = new float[vertsSize * 2];
        int[] polyColors = new int[vertsSize];

        for (int i = 0; i < vertsSize; i++) {
            polyVerts[i * 2] = polygon.get(i * 2);
            polyVerts[i * 2 + 1] = polygon.get(i * 2 + 1);

            polyColors[i] = Color.RED;
        }

        // Construct indices through a list
        ArrayList<Short> indices = new ArrayList<>();
        for (int i = 1; i < vertsSize - 1; ++i) {
            indices.add((short) 0);
            indices.add((short) i);
            indices.add((short) (i + 1));
        }

        // Copy indices into an array
        short[] indicesArray = new short[indices.size()];
        for (int i = 0; i < indices.size(); i++) {
            indicesArray[i] = indices.get(i);
        }

        Paint p = new Paint();
        p.setShader(planeShader);
        p.setColor(Color.RED);
        p.setAlpha(100);

        canvas.save();
        canvas.setMatrix(mvpv);

        canvas.drawVertices(Canvas.VertexMode.TRIANGLE_FAN, vertsSize, polyVerts, 0,
                null, 0, polyColors, 0, indicesArray, 0,
                indicesArray.length, p);
        canvas.restore();
    }
}
